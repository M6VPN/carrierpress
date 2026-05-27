/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_agc.c */

#include <sys/types.h>

#include <math.h>
#include <stdint.h>

#include "cp_agc.h"

#define CP_AGC_DB_FLOOR		(-120.0f)
#define CP_AGC_DENORMAL_FLOOR	(0.00000000000000000001f)
#define CP_AGC_FAST_DELTA_DB	(3.0f)
#define CP_AGC_MAX_ABS_DB	(96.0f)
#define CP_AGC_LEGACY_STEP_DB	(96.0f)

static cp_sample_t	cp_agc_block_coeff(cp_sample_t, size_t,
			    cp_sample_t);
static cp_sample_t	cp_agc_clamp(cp_sample_t, cp_sample_t,
			    cp_sample_t);
static cp_sample_t	cp_agc_db_to_linear(cp_sample_t);
static cp_sample_t	cp_agc_gain_step(cp_sample_t, cp_sample_t,
			    cp_sample_t, size_t, cp_sample_t, cp_sample_t);
static size_t		cp_agc_hold_frames(const struct cp_agc *);
static cp_sample_t	cp_agc_legacy_ms(cp_sample_t, cp_sample_t);
static cp_sample_t	cp_agc_linear_to_db(cp_sample_t);
static cp_sample_t	cp_agc_measure_rms(const cp_sample_t *, size_t);
static int		cp_agc_valid_config(const struct cp_agc_config *);
static cp_sample_t	cp_agc_safe_sample(cp_sample_t);

void
cp_agc_default_config(struct cp_agc_config *config)
{
	if (config == NULL)
		return;

	config->target_rms           = CP_DEFAULT_TARGET_RMS;
	config->min_gain             = CP_AGC_DEFAULT_MIN_GAIN;
	config->max_gain             = CP_DEFAULT_MAX_GAIN;
	config->attack_ms            = CP_AGC_DEFAULT_ATTACK_MS;
	config->release_ms           = CP_AGC_DEFAULT_RELEASE_MS;
	config->fast_attack_ms       = CP_AGC_DEFAULT_FAST_ATTACK_MS;
	config->hold_ms              = CP_AGC_DEFAULT_HOLD_MS;
	config->gate_threshold_db    = CP_AGC_DEFAULT_GATE_DB;
	config->silence_threshold_db = CP_AGC_DEFAULT_SILENCE_DB;
	config->max_gain_step_db     = CP_AGC_DEFAULT_MAX_STEP_DB;
	config->sample_rate          = CP_AGC_DEFAULT_SAMPLE_RATE;
}

int
cp_agc_init_config(struct cp_agc *state, size_t channels,
	const struct cp_agc_config *config)
{
	if (state == NULL || config == NULL)
		return CP_ERR_NULL;
	if (channels != CP_CHANNELS_MONO && channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (!cp_agc_valid_config(config))
		return CP_ERR_RANGE;

	state->channels             = channels;
	state->config               = *config;
	state->target_rms           = config->target_rms;
	state->min_gain             = config->min_gain;
	state->max_gain             = config->max_gain;
	state->attack_coeff         = 0.0f;
	state->release_coeff        = 0.0f;
	state->smooth_coeff         = 0.0f;
	state->attack_ms            = config->attack_ms;
	state->release_ms           = config->release_ms;
	state->fast_attack_ms       = config->fast_attack_ms;
	state->hold_ms              = config->hold_ms;
	state->gate_threshold_db    = config->gate_threshold_db;
	state->silence_threshold_db = config->silence_threshold_db;
	state->max_gain_step_db     = config->max_gain_step_db;
	state->sample_rate          = config->sample_rate;

	return cp_agc_reset(state);
}

int
cp_agc_init(struct cp_agc *state, size_t channels, cp_sample_t target_rms,
	cp_sample_t max_gain, cp_sample_t attack_coeff, cp_sample_t release_coeff,
	cp_sample_t smooth_coeff)
{
	struct cp_agc_config config;
	int status;

	cp_agc_default_config(&config);
	config.target_rms       = target_rms;
	config.max_gain         = max_gain;
	config.attack_ms        = cp_agc_legacy_ms(attack_coeff,
	    CP_AGC_DEFAULT_ATTACK_MS);
	config.release_ms       = cp_agc_legacy_ms(release_coeff,
	    CP_AGC_DEFAULT_RELEASE_MS);
	config.fast_attack_ms   = config.attack_ms;
	config.max_gain_step_db = (smooth_coeff >= 1.0f) ?
	    CP_AGC_LEGACY_STEP_DB :
	    cp_agc_clamp(smooth_coeff * CP_AGC_LEGACY_STEP_DB, 0.1f,
	    CP_AGC_LEGACY_STEP_DB);

	status = cp_agc_init_config(state, channels, &config);
	if (status != CP_OK)
		return status;

	state->attack_coeff  = attack_coeff;
	state->release_coeff = release_coeff;
	state->smooth_coeff  = smooth_coeff;

	return CP_OK;
}

int
cp_agc_process(struct cp_agc *state, const cp_sample_t *input,
	cp_sample_t *output, size_t frames)
{
	cp_sample_t block_db;
	cp_sample_t desired_db;
	cp_sample_t desired_gain;
	cp_sample_t move_ms;
	cp_sample_t sample;
	size_t index;
	size_t samples;

	if (state == NULL || input == NULL || output == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (frames == 0)
		return CP_ERR_RANGE;
	if (frames > (SIZE_MAX / state->channels))
		return CP_ERR_RANGE;

	samples         = frames * state->channels;
	state->last_rms = cp_agc_measure_rms(input, samples);
	block_db        = cp_agc_linear_to_db(state->last_rms);
	desired_db      = state->gain_db;
	move_ms         = state->release_ms;

	if (block_db <= state->silence_threshold_db) {
		state->gate_state = CP_AGC_STATE_SILENT;
	} else if (block_db <= state->gate_threshold_db) {
		state->gate_state = CP_AGC_STATE_HELD;
	} else {
		desired_gain = state->target_rms /
		    cp_agc_clamp(state->last_rms, CP_AGC_DENORMAL_FLOOR,
		    CP_SAMPLE_MAX);
		desired_gain = cp_agc_clamp(desired_gain, state->min_gain,
		    state->max_gain);
		desired_db = cp_agc_linear_to_db(desired_gain);

		if (desired_db < state->gain_db) {
			state->gate_state           = CP_AGC_STATE_OPEN;
			state->hold_remaining_frames = cp_agc_hold_frames(state);
			move_ms = ((state->gain_db - desired_db) >=
			    CP_AGC_FAST_DELTA_DB) ? state->fast_attack_ms :
			    state->attack_ms;
		} else if (desired_db > state->gain_db &&
		    state->hold_remaining_frames > 0) {
			state->gate_state = CP_AGC_STATE_HELD;
			if (state->hold_remaining_frames > frames)
				state->hold_remaining_frames -= frames;
			else
				state->hold_remaining_frames = 0;
			desired_db = state->gain_db;
		} else {
			state->gate_state = CP_AGC_STATE_OPEN;
			move_ms = state->release_ms;
		}
	}

	state->gain_db = cp_agc_gain_step(state->gain_db, desired_db, move_ms,
	    frames, state->sample_rate, state->max_gain_step_db);
	state->gain_db = cp_agc_clamp(state->gain_db,
	    cp_agc_linear_to_db(state->min_gain),
	    cp_agc_linear_to_db(state->max_gain));
	state->gain = cp_agc_db_to_linear(state->gain_db);
	state->gain = cp_agc_clamp(state->gain, state->min_gain,
	    state->max_gain);
	state->envelope_rms = state->last_rms;

	for (index = 0; index < samples; index++) {
		sample = cp_agc_safe_sample(input[index]) * state->gain;
		output[index] = cp_agc_safe_sample(sample);
	}

	return CP_OK;
}

int
cp_agc_reset(struct cp_agc *state)
{
	if (state == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	state->envelope_rms          = state->target_rms;
	state->last_rms              = 0.0f;
	state->gain                  = 1.0f;
	state->gain_db               = 0.0f;
	state->gate_state            = CP_AGC_STATE_OPEN;
	state->hold_remaining_frames = 0;

	return CP_OK;
}

const char *
cp_agc_state_string(enum cp_agc_gate_state state)
{
	switch (state) {
	case CP_AGC_STATE_OPEN:
		return "open";
	case CP_AGC_STATE_HELD:
		return "held";
	case CP_AGC_STATE_SILENT:
		return "silent";
	default:
		return "unknown";
	}
}

static cp_sample_t
cp_agc_block_coeff(cp_sample_t time_ms, size_t frames, cp_sample_t sample_rate)
{
	cp_sample_t seconds;
	cp_sample_t time_seconds;

	if (time_ms <= 0.0f)
		return 1.0f;

	seconds      = (cp_sample_t)frames / sample_rate;
	time_seconds = time_ms / 1000.0f;

	return 1.0f - expf(-seconds / time_seconds);
}

static cp_sample_t
cp_agc_clamp(cp_sample_t value, cp_sample_t min_value, cp_sample_t max_value)
{
	if (value < min_value)
		return min_value;
	if (value > max_value)
		return max_value;

	return value;
}

static cp_sample_t
cp_agc_db_to_linear(cp_sample_t db)
{
	if (!isfinite(db))
		return 1.0f;

	db = cp_agc_clamp(db, -CP_AGC_MAX_ABS_DB, CP_AGC_MAX_ABS_DB);
	return powf(10.0f, db / 20.0f);
}

static cp_sample_t
cp_agc_gain_step(cp_sample_t current_db, cp_sample_t desired_db,
	cp_sample_t time_ms, size_t frames, cp_sample_t sample_rate,
	cp_sample_t max_step_db)
{
	cp_sample_t coeff;
	cp_sample_t delta;
	cp_sample_t step;

	coeff = cp_agc_block_coeff(time_ms, frames, sample_rate);
	delta = (desired_db - current_db) * coeff;
	step  = cp_agc_clamp(delta, -max_step_db, max_step_db);

	return current_db + step;
}

static size_t
cp_agc_hold_frames(const struct cp_agc *state)
{
	cp_sample_t frames;

	frames = (state->hold_ms / 1000.0f) * state->sample_rate;
	if (!isfinite(frames) || frames <= 0.0f)
		return 0;
	if (frames > (cp_sample_t)SIZE_MAX)
		return SIZE_MAX;

	return (size_t)frames;
}

static cp_sample_t
cp_agc_legacy_ms(cp_sample_t coeff, cp_sample_t fallback_ms)
{
	if (!isfinite(coeff) || coeff < 0.0f)
		return fallback_ms;
	if (coeff >= 1.0f)
		return 0.0f;
	if (coeff <= 0.0f)
		return fallback_ms * 10.0f;

	return fallback_ms * ((1.0f - coeff) / coeff);
}

static cp_sample_t
cp_agc_linear_to_db(cp_sample_t linear)
{
	if (!isfinite(linear) || linear <= CP_AGC_DENORMAL_FLOOR)
		return CP_AGC_DB_FLOOR;

	return 20.0f * log10f(linear);
}

static cp_sample_t
cp_agc_measure_rms(const cp_sample_t *input, size_t samples)
{
	cp_sample_t sample;
	cp_sample_t sum_sq;
	size_t index;

	if (input == NULL || samples == 0)
		return 0.0f;

	sum_sq = 0.0f;
	for (index = 0; index < samples; index++) {
		sample  = cp_agc_safe_sample(input[index]);
		sum_sq += sample * sample;
	}

	if (sum_sq <= CP_AGC_DENORMAL_FLOOR)
		return 0.0f;

	return sqrtf(sum_sq / (cp_sample_t)samples);
}

static int
cp_agc_valid_config(const struct cp_agc_config *config)
{
	if (config == NULL)
		return 0;
	if (!isfinite(config->target_rms) || config->target_rms <= 0.0f ||
	    config->target_rms > CP_SAMPLE_MAX)
		return 0;
	if (!isfinite(config->min_gain) || !isfinite(config->max_gain) ||
	    config->min_gain <= 0.0f || config->max_gain < config->min_gain)
		return 0;
	if (!isfinite(config->attack_ms) || !isfinite(config->release_ms) ||
	    !isfinite(config->fast_attack_ms) || !isfinite(config->hold_ms) ||
	    config->attack_ms < 0.0f || config->release_ms < 0.0f ||
	    config->fast_attack_ms < 0.0f || config->hold_ms < 0.0f)
		return 0;
	if (!isfinite(config->gate_threshold_db) ||
	    !isfinite(config->silence_threshold_db) ||
	    config->gate_threshold_db <= config->silence_threshold_db)
		return 0;
	if (!isfinite(config->max_gain_step_db) ||
	    config->max_gain_step_db <= 0.0f)
		return 0;
	if (!isfinite(config->sample_rate) || config->sample_rate < 8000.0f ||
	    config->sample_rate > 384000.0f)
		return 0;

	return 1;
}

static cp_sample_t
cp_agc_safe_sample(cp_sample_t sample)
{
	if (!isfinite(sample))
		return 0.0f;
	if (fabsf(sample) < CP_AGC_DENORMAL_FLOOR)
		return 0.0f;

	return sample;
}

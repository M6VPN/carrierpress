/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_low_level_boost.c */

#include <sys/types.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "cp_low_level_boost.h"

#define CP_LLB_DB_FLOOR		(-120.0f)
#define CP_LLB_DENORMAL_FLOOR	0.00000000000000000001f
#define CP_LLB_MAX_ABS_DB	96.0f

static cp_sample_t	cp_llb_block_coeff(cp_sample_t, size_t, cp_sample_t);
static cp_sample_t	cp_llb_clamp(cp_sample_t, cp_sample_t, cp_sample_t);
static cp_sample_t	cp_llb_db_to_linear(cp_sample_t);
static cp_sample_t	cp_llb_linear_to_db(cp_sample_t);
static cp_sample_t	cp_llb_measure_rms(const cp_sample_t *, size_t);
static cp_sample_t	cp_llb_safe_sample(cp_sample_t);
static int		cp_llb_validate_config(
			    const struct cp_low_level_boost_config *);

void
cp_low_level_boost_default_config(struct cp_low_level_boost_config *config)
{
	if (config == NULL)
		return;

	config->enabled = CP_LLB_DEFAULT_ENABLED;
	config->sample_rate = CP_LLB_DEFAULT_SAMPLE_RATE;
	config->channel_count = CP_LLB_DEFAULT_CHANNELS;
	config->target_rms = CP_LLB_DEFAULT_TARGET_RMS;
	config->max_boost_db = CP_LLB_DEFAULT_MAX_BOOST_DB;
	config->attack_ms = CP_LLB_DEFAULT_ATTACK_MS;
	config->release_ms = CP_LLB_DEFAULT_RELEASE_MS;
	config->gate_threshold_db = CP_LLB_DEFAULT_GATE_DB;
	config->silence_threshold_db = CP_LLB_DEFAULT_SILENCE_DB;
}

int
cp_low_level_boost_init(struct cp_low_level_boost *state,
	const struct cp_low_level_boost_config *config)
{
	if (state == NULL || config == NULL)
		return CP_ERR_NULL;
	if (!cp_llb_validate_config(config))
		return CP_ERR_RANGE;

	(void)memset(state, 0, sizeof(*state));
	state->config = *config;

	return cp_low_level_boost_reset(state);
}

int
cp_low_level_boost_process(struct cp_low_level_boost *state,
	const cp_sample_t *input, cp_sample_t *output, size_t frames)
{
	cp_sample_t block_db;
	cp_sample_t coeff;
	cp_sample_t desired_db;
	cp_sample_t desired_linear;
	cp_sample_t sample;
	size_t index;
	size_t samples;

	if (state == NULL || input == NULL || output == NULL)
		return CP_ERR_NULL;
	if (state->config.channel_count != CP_CHANNELS_MONO &&
	    state->config.channel_count != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (frames == 0)
		return CP_ERR_RANGE;
	if (frames > SIZE_MAX / state->config.channel_count)
		return CP_ERR_RANGE;

	samples = frames * state->config.channel_count;
	if (!state->config.enabled) {
		if (input != output)
			(void)memcpy(output, input, samples * sizeof(*output));
		state->last_rms = cp_llb_measure_rms(input, samples);
		state->gain = 1.0f;
		state->gain_db = 0.0f;
		state->gate_state = CP_AGC_STATE_OPEN;
		return CP_OK;
	}

	state->last_rms = cp_llb_measure_rms(input, samples);
	block_db = cp_llb_linear_to_db(state->last_rms);
	desired_db = 0.0f;
	if (block_db <= state->config.silence_threshold_db) {
		state->gate_state = CP_AGC_STATE_SILENT;
	} else if (block_db <= state->config.gate_threshold_db) {
		state->gate_state = CP_AGC_STATE_HELD;
	} else {
		state->gate_state = CP_AGC_STATE_OPEN;
		if (state->last_rms < state->config.target_rms) {
			desired_linear = state->config.target_rms /
			    cp_llb_clamp(state->last_rms,
			    CP_LLB_DENORMAL_FLOOR, CP_SAMPLE_MAX);
			desired_db = cp_llb_linear_to_db(desired_linear);
			desired_db = cp_llb_clamp(desired_db, 0.0f,
			    state->config.max_boost_db);
		}
	}

	coeff = (desired_db > state->gain_db) ?
	    cp_llb_block_coeff(state->config.attack_ms, frames,
	    state->config.sample_rate) :
	    cp_llb_block_coeff(state->config.release_ms, frames,
	    state->config.sample_rate);
	state->gain_db += coeff * (desired_db - state->gain_db);
	state->gain_db = cp_llb_clamp(state->gain_db, 0.0f,
	    state->config.max_boost_db);
	state->gain = cp_llb_db_to_linear(state->gain_db);

	for (index = 0; index < samples; index++) {
		sample = cp_llb_safe_sample(input[index]) * state->gain;
		output[index] = cp_llb_safe_sample(sample);
	}

	return CP_OK;
}

int
cp_low_level_boost_reset(struct cp_low_level_boost *state)
{
	if (state == NULL)
		return CP_ERR_NULL;
	if (state->config.channel_count != CP_CHANNELS_MONO &&
	    state->config.channel_count != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	state->last_rms = 0.0f;
	state->gain = 1.0f;
	state->gain_db = 0.0f;
	state->gate_state = CP_AGC_STATE_OPEN;

	return CP_OK;
}

static cp_sample_t
cp_llb_block_coeff(cp_sample_t time_ms, size_t frames, cp_sample_t sample_rate)
{
	cp_sample_t seconds;

	if (time_ms <= 0.0f)
		return 1.0f;
	seconds = (cp_sample_t)frames / sample_rate;

	return 1.0f - expf(-seconds / (time_ms / 1000.0f));
}

static cp_sample_t
cp_llb_clamp(cp_sample_t value, cp_sample_t low, cp_sample_t high)
{
	if (!isfinite(value))
		return low;
	if (value < low)
		return low;
	if (value > high)
		return high;

	return value;
}

static cp_sample_t
cp_llb_db_to_linear(cp_sample_t db)
{
	if (!isfinite(db))
		return 1.0f;
	db = cp_llb_clamp(db, -CP_LLB_MAX_ABS_DB, CP_LLB_MAX_ABS_DB);

	return powf(10.0f, db / 20.0f);
}

static cp_sample_t
cp_llb_linear_to_db(cp_sample_t linear)
{
	if (!isfinite(linear) || linear <= CP_LLB_DENORMAL_FLOOR)
		return CP_LLB_DB_FLOOR;

	return 20.0f * log10f(linear);
}

static cp_sample_t
cp_llb_measure_rms(const cp_sample_t *input, size_t samples)
{
	cp_sample_t sample;
	cp_sample_t sum;
	size_t index;

	if (input == NULL || samples == 0)
		return 0.0f;
	sum = 0.0f;
	for (index = 0; index < samples; index++) {
		sample = cp_llb_safe_sample(input[index]);
		sum += sample * sample;
	}
	if (sum <= CP_LLB_DENORMAL_FLOOR)
		return 0.0f;

	return sqrtf(sum / (cp_sample_t)samples);
}

static cp_sample_t
cp_llb_safe_sample(cp_sample_t sample)
{
	if (!isfinite(sample))
		return 0.0f;
	if (fabsf(sample) < CP_LLB_DENORMAL_FLOOR)
		return 0.0f;

	return sample;
}

static int
cp_llb_validate_config(const struct cp_low_level_boost_config *config)
{
	if (config == NULL)
		return 0;
	if (config->channel_count != CP_CHANNELS_MONO &&
	    config->channel_count != CP_CHANNELS_STEREO)
		return 0;
	if (!isfinite(config->sample_rate) ||
	    config->sample_rate < CP_LLB_MIN_SAMPLE_RATE ||
	    config->sample_rate > CP_LLB_MAX_SAMPLE_RATE)
		return 0;
	if (!isfinite(config->target_rms) ||
	    config->target_rms < CP_LLB_MIN_TARGET_RMS ||
	    config->target_rms > CP_LLB_MAX_TARGET_RMS)
		return 0;
	if (!isfinite(config->max_boost_db) ||
	    config->max_boost_db < CP_LLB_MIN_BOOST_DB ||
	    config->max_boost_db > CP_LLB_MAX_BOOST_DB)
		return 0;
	if (!isfinite(config->attack_ms) ||
	    !isfinite(config->release_ms) ||
	    config->attack_ms < CP_LLB_MIN_TIME_MS ||
	    config->release_ms < CP_LLB_MIN_TIME_MS ||
	    config->attack_ms > CP_LLB_MAX_TIME_MS ||
	    config->release_ms > CP_LLB_MAX_TIME_MS)
		return 0;
	if (!isfinite(config->gate_threshold_db) ||
	    !isfinite(config->silence_threshold_db) ||
	    config->gate_threshold_db < CP_LLB_MIN_GATE_DB ||
	    config->gate_threshold_db > CP_LLB_MAX_GATE_DB ||
	    config->silence_threshold_db < CP_LLB_MIN_GATE_DB ||
	    config->silence_threshold_db > CP_LLB_MAX_GATE_DB ||
	    config->gate_threshold_db <= config->silence_threshold_db)
		return 0;

	return 1;
}

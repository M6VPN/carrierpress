/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_natural_dynamics.c */

#include <sys/types.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "cp_natural_dynamics.h"

#define CP_ND_DB_FLOOR		(-120.0f)
#define CP_ND_DENORMAL_FLOOR	0.00000000000000000001f
#define CP_ND_MAX_ABS_DB	96.0f

static cp_sample_t	cp_nd_block_coeff(cp_sample_t, size_t, cp_sample_t);
static cp_sample_t	cp_nd_clamp(cp_sample_t, cp_sample_t, cp_sample_t);
static cp_sample_t	cp_nd_db_to_linear(cp_sample_t);
static cp_sample_t	cp_nd_linear_to_db(cp_sample_t);
static cp_sample_t	cp_nd_measure_rms(const cp_sample_t *, size_t);
static cp_sample_t	cp_nd_safe_sample(cp_sample_t);
static int		cp_nd_validate_config(
			    const struct cp_natural_dynamics_config *);

void
cp_natural_dynamics_default_config(
	struct cp_natural_dynamics_config *config)
{
	if (config == NULL)
		return;

	config->enabled = CP_ND_DEFAULT_ENABLED;
	config->sample_rate = CP_ND_DEFAULT_SAMPLE_RATE;
	config->channel_count = CP_ND_DEFAULT_CHANNELS;
	config->threshold_db = CP_ND_DEFAULT_THRESHOLD_DB;
	config->ratio = CP_ND_DEFAULT_RATIO;
	config->attack_ms = CP_ND_DEFAULT_ATTACK_MS;
	config->release_ms = CP_ND_DEFAULT_RELEASE_MS;
	config->max_gain_reduction_db = CP_ND_DEFAULT_MAX_REDUCTION_DB;
}

int
cp_natural_dynamics_init(struct cp_natural_dynamics *state,
	const struct cp_natural_dynamics_config *config)
{
	if (state == NULL || config == NULL)
		return CP_ERR_NULL;
	if (!cp_nd_validate_config(config))
		return CP_ERR_RANGE;

	(void)memset(state, 0, sizeof(*state));
	state->config = *config;

	return cp_natural_dynamics_reset(state);
}

int
cp_natural_dynamics_process(struct cp_natural_dynamics *state,
	const cp_sample_t *input, cp_sample_t *output, size_t frames)
{
	cp_sample_t block_db;
	cp_sample_t coeff;
	cp_sample_t desired_db;
	cp_sample_t over_db;
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
		state->last_rms = cp_nd_measure_rms(input, samples);
		state->gain = 1.0f;
		state->gain_db = 0.0f;
		state->gain_reduction_db = 0.0f;
		return CP_OK;
	}

	state->last_rms = cp_nd_measure_rms(input, samples);
	block_db = cp_nd_linear_to_db(state->last_rms);
	desired_db = 0.0f;
	if (block_db > state->config.threshold_db) {
		over_db = block_db - state->config.threshold_db;
		desired_db = -over_db *
		    (1.0f - (1.0f / state->config.ratio));
		desired_db = cp_nd_clamp(desired_db,
		    -state->config.max_gain_reduction_db, 0.0f);
	}

	coeff = (desired_db < state->gain_db) ?
	    cp_nd_block_coeff(state->config.attack_ms, frames,
	    state->config.sample_rate) :
	    cp_nd_block_coeff(state->config.release_ms, frames,
	    state->config.sample_rate);
	state->gain_db += coeff * (desired_db - state->gain_db);
	state->gain_db = cp_nd_clamp(state->gain_db,
	    -state->config.max_gain_reduction_db, 0.0f);
	state->gain = cp_nd_db_to_linear(state->gain_db);
	state->gain_reduction_db = -state->gain_db;
	if (state->gain_reduction_db < 0.000001f)
		state->gain_reduction_db = 0.0f;

	for (index = 0; index < samples; index++) {
		sample = cp_nd_safe_sample(input[index]) * state->gain;
		output[index] = cp_nd_safe_sample(sample);
	}

	return CP_OK;
}

int
cp_natural_dynamics_reset(struct cp_natural_dynamics *state)
{
	if (state == NULL)
		return CP_ERR_NULL;
	if (state->config.channel_count != CP_CHANNELS_MONO &&
	    state->config.channel_count != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	state->last_rms = 0.0f;
	state->gain = 1.0f;
	state->gain_db = 0.0f;
	state->gain_reduction_db = 0.0f;

	return CP_OK;
}

static cp_sample_t
cp_nd_block_coeff(cp_sample_t time_ms, size_t frames, cp_sample_t sample_rate)
{
	cp_sample_t seconds;

	if (time_ms <= 0.0f)
		return 1.0f;
	seconds = (cp_sample_t)frames / sample_rate;

	return 1.0f - expf(-seconds / (time_ms / 1000.0f));
}

static cp_sample_t
cp_nd_clamp(cp_sample_t value, cp_sample_t low, cp_sample_t high)
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
cp_nd_db_to_linear(cp_sample_t db)
{
	if (!isfinite(db))
		return 1.0f;
	db = cp_nd_clamp(db, -CP_ND_MAX_ABS_DB, CP_ND_MAX_ABS_DB);

	return powf(10.0f, db / 20.0f);
}

static cp_sample_t
cp_nd_linear_to_db(cp_sample_t linear)
{
	if (!isfinite(linear) || linear <= CP_ND_DENORMAL_FLOOR)
		return CP_ND_DB_FLOOR;

	return 20.0f * log10f(linear);
}

static cp_sample_t
cp_nd_measure_rms(const cp_sample_t *input, size_t samples)
{
	cp_sample_t sample;
	cp_sample_t sum;
	size_t index;

	if (input == NULL || samples == 0)
		return 0.0f;
	sum = 0.0f;
	for (index = 0; index < samples; index++) {
		sample = cp_nd_safe_sample(input[index]);
		sum += sample * sample;
	}
	if (sum <= CP_ND_DENORMAL_FLOOR)
		return 0.0f;

	return sqrtf(sum / (cp_sample_t)samples);
}

static cp_sample_t
cp_nd_safe_sample(cp_sample_t sample)
{
	if (!isfinite(sample))
		return 0.0f;
	if (fabsf(sample) < CP_ND_DENORMAL_FLOOR)
		return 0.0f;

	return sample;
}

static int
cp_nd_validate_config(const struct cp_natural_dynamics_config *config)
{
	if (config == NULL)
		return 0;
	if (config->channel_count != CP_CHANNELS_MONO &&
	    config->channel_count != CP_CHANNELS_STEREO)
		return 0;
	if (!isfinite(config->sample_rate) ||
	    config->sample_rate < CP_ND_MIN_SAMPLE_RATE ||
	    config->sample_rate > CP_ND_MAX_SAMPLE_RATE)
		return 0;
	if (!isfinite(config->threshold_db) ||
	    config->threshold_db < CP_ND_MIN_THRESHOLD_DB ||
	    config->threshold_db > CP_ND_MAX_THRESHOLD_DB)
		return 0;
	if (!isfinite(config->ratio) || config->ratio < CP_ND_MIN_RATIO ||
	    config->ratio > CP_ND_MAX_RATIO)
		return 0;
	if (!isfinite(config->attack_ms) ||
	    !isfinite(config->release_ms) ||
	    config->attack_ms < CP_ND_MIN_TIME_MS ||
	    config->release_ms < CP_ND_MIN_TIME_MS ||
	    config->attack_ms > CP_ND_MAX_TIME_MS ||
	    config->release_ms > CP_ND_MAX_TIME_MS)
		return 0;
	if (!isfinite(config->max_gain_reduction_db) ||
	    config->max_gain_reduction_db < CP_ND_MIN_REDUCTION_DB ||
	    config->max_gain_reduction_db > CP_ND_MAX_REDUCTION_DB)
		return 0;

	return 1;
}

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_compressor.c */

#include <sys/types.h>

#include <math.h>
#include <stdint.h>

#include "cp_compressor.h"

#define CP_COMPRESSOR_DB_FLOOR		(-120.0f)
#define CP_COMPRESSOR_DENORMAL_FLOOR	(0.00000000000000000001f)
#define CP_COMPRESSOR_MAX_ABS_DB	(96.0f)
#define CP_COMPRESSOR_MAX_RATIO		(40.0f)

static cp_sample_t	cp_compressor_coeff(cp_sample_t, cp_sample_t);
static cp_sample_t	cp_compressor_db_to_linear(cp_sample_t);
static cp_sample_t	cp_compressor_detector_db(const cp_sample_t *,
			    size_t);
static cp_sample_t	cp_compressor_gain_db(struct cp_compressor *,
			    cp_sample_t);
static cp_sample_t	cp_compressor_linear_to_db(cp_sample_t);
static cp_sample_t	cp_compressor_safe(cp_sample_t);
static int		cp_compressor_valid_config(
			    const struct cp_compressor_config *);

void
cp_compressor_default_config(struct cp_compressor_config *config)
{
	if (config == NULL)
		return;

	config->threshold_db   = CP_COMPRESSOR_DEFAULT_THRESHOLD_DB;
	config->ratio          = CP_COMPRESSOR_DEFAULT_RATIO;
	config->attack_ms      = CP_COMPRESSOR_DEFAULT_ATTACK_MS;
	config->release_ms     = CP_COMPRESSOR_DEFAULT_RELEASE_MS;
	config->makeup_gain_db = CP_COMPRESSOR_DEFAULT_MAKEUP_DB;
	config->knee_db        = CP_COMPRESSOR_DEFAULT_KNEE_DB;
	config->sample_rate    = CP_COMPRESSOR_DEFAULT_RATE;
	config->channels       = CP_CHANNELS_MONO;
	config->enabled        = 1;
}

int
cp_compressor_init(struct cp_compressor *state,
	const struct cp_compressor_config *config)
{
	if (state == NULL || config == NULL)
		return CP_ERR_NULL;
	if (!cp_compressor_valid_config(config))
		return CP_ERR_RANGE;

	state->config      = *config;
	state->channels    = config->channels;
	state->makeup_gain = cp_compressor_db_to_linear(config->makeup_gain_db);

	return cp_compressor_reset(state);
}

int
cp_compressor_process(struct cp_compressor *state, const cp_sample_t *input,
	cp_sample_t *output, size_t frames)
{
	size_t frame;

	if (state == NULL || input == NULL || output == NULL)
		return CP_ERR_NULL;
	if (frames > (SIZE_MAX / state->channels))
		return CP_ERR_RANGE;

	for (frame = 0; frame < frames; frame++) {
		cp_compressor_process_frame(state, &input[frame * state->channels],
		    &output[frame * state->channels]);
	}

	return CP_OK;
}

int
cp_compressor_process_frame(struct cp_compressor *state,
	const cp_sample_t *input, cp_sample_t *output)
{
	cp_sample_t coeff;
	cp_sample_t detector_db;
	cp_sample_t desired_gain_db;
	cp_sample_t gain;
	size_t channel;

	if (state == NULL || input == NULL || output == NULL)
		return CP_ERR_NULL;

	if (!state->config.enabled) {
		for (channel = 0; channel < state->channels; channel++)
			output[channel] = cp_compressor_safe(input[channel]);
		state->gain_db           = 0.0f;
		state->gain_reduction_db = 0.0f;
		return CP_OK;
	}

	detector_db = cp_compressor_detector_db(input, state->channels);
	coeff = (detector_db > state->envelope_db) ?
	    cp_compressor_coeff(state->config.attack_ms,
	    state->config.sample_rate) :
	    cp_compressor_coeff(state->config.release_ms,
	    state->config.sample_rate);
	state->envelope_db += coeff * (detector_db - state->envelope_db);

	desired_gain_db = cp_compressor_gain_db(state, state->envelope_db);

	coeff = (desired_gain_db < state->gain_db) ?
	    cp_compressor_coeff(state->config.attack_ms,
	    state->config.sample_rate) :
	    cp_compressor_coeff(state->config.release_ms,
	    state->config.sample_rate);
	state->gain_db += coeff * (desired_gain_db - state->gain_db);
	state->gain_reduction_db = -state->gain_db;
	if (fabsf(state->gain_reduction_db) < 0.000001f ||
	    state->gain_reduction_db < 0.0f)
		state->gain_reduction_db = 0.0f;

	gain = cp_compressor_db_to_linear(state->gain_db +
	    state->config.makeup_gain_db);
	for (channel = 0; channel < state->channels; channel++)
		output[channel] = cp_compressor_safe(input[channel]) * gain;

	return CP_OK;
}

int
cp_compressor_reset(struct cp_compressor *state)
{
	if (state == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	state->envelope_db       = CP_COMPRESSOR_DB_FLOOR;
	state->gain_db           = 0.0f;
	state->gain_reduction_db = 0.0f;

	return CP_OK;
}

static cp_sample_t
cp_compressor_coeff(cp_sample_t time_ms, cp_sample_t sample_rate)
{
	if (time_ms <= 0.0f)
		return 1.0f;

	return 1.0f - expf(-1.0f / ((time_ms / 1000.0f) * sample_rate));
}

static cp_sample_t
cp_compressor_db_to_linear(cp_sample_t db)
{
	if (!isfinite(db))
		return 1.0f;
	if (db > CP_COMPRESSOR_MAX_ABS_DB)
		db = CP_COMPRESSOR_MAX_ABS_DB;
	if (db < -CP_COMPRESSOR_MAX_ABS_DB)
		db = -CP_COMPRESSOR_MAX_ABS_DB;

	return powf(10.0f, db / 20.0f);
}

static cp_sample_t
cp_compressor_detector_db(const cp_sample_t *input, size_t channels)
{
	cp_sample_t sample;
	cp_sample_t sum;
	size_t channel;

	sum = 0.0f;
	for (channel = 0; channel < channels; channel++) {
		sample = cp_compressor_safe(input[channel]);
		sum += sample * sample;
	}

	return cp_compressor_linear_to_db(sqrtf(sum / (cp_sample_t)channels));
}

static cp_sample_t
cp_compressor_gain_db(struct cp_compressor *state, cp_sample_t level_db)
{
	cp_sample_t above;
	cp_sample_t compressed;
	cp_sample_t knee;
	cp_sample_t threshold;

	threshold = state->config.threshold_db;
	knee      = state->config.knee_db;
	if (knee > 0.0f && level_db > threshold - (knee * 0.5f) &&
	    level_db < threshold + (knee * 0.5f)) {
		above = level_db - threshold + (knee * 0.5f);
		compressed = (above * above) / (2.0f * knee);
		return -compressed * (1.0f - (1.0f / state->config.ratio));
	}

	if (level_db <= threshold)
		return 0.0f;

	above = level_db - threshold;
	return -above * (1.0f - (1.0f / state->config.ratio));
}

static cp_sample_t
cp_compressor_linear_to_db(cp_sample_t linear)
{
	if (!isfinite(linear) || linear <= CP_COMPRESSOR_DENORMAL_FLOOR)
		return CP_COMPRESSOR_DB_FLOOR;

	return 20.0f * log10f(linear);
}

static cp_sample_t
cp_compressor_safe(cp_sample_t sample)
{
	if (!isfinite(sample))
		return 0.0f;
	if (fabsf(sample) < CP_COMPRESSOR_DENORMAL_FLOOR)
		return 0.0f;

	return sample;
}

static int
cp_compressor_valid_config(const struct cp_compressor_config *config)
{
	if (config == NULL)
		return 0;
	if (config->channels != CP_CHANNELS_MONO &&
	    config->channels != CP_CHANNELS_STEREO)
		return 0;
	if (!isfinite(config->sample_rate) || config->sample_rate < 8000.0f ||
	    config->sample_rate > 384000.0f)
		return 0;
	if (!isfinite(config->threshold_db) || config->threshold_db < -90.0f ||
	    config->threshold_db > 12.0f)
		return 0;
	if (!isfinite(config->ratio) || config->ratio < 1.0f ||
	    config->ratio > CP_COMPRESSOR_MAX_RATIO)
		return 0;
	if (!isfinite(config->attack_ms) || !isfinite(config->release_ms) ||
	    config->attack_ms < 0.0f || config->release_ms < 0.0f)
		return 0;
	if (!isfinite(config->makeup_gain_db) ||
	    config->makeup_gain_db < -24.0f ||
	    config->makeup_gain_db > 24.0f)
		return 0;
	if (!isfinite(config->knee_db) || config->knee_db < 0.0f ||
	    config->knee_db > 24.0f)
		return 0;

	return 1;
}

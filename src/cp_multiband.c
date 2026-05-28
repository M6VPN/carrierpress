/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_multiband.c */

#include <sys/types.h>

#include <math.h>
#include <stdint.h>

#include "cp_multiband.h"

#define CP_MULTIBAND_DENORMAL_FLOOR	(0.00000000000000000001f)

static void		cp_multiband_apply_preset(struct cp_compressor_config *,
			    const struct cp_multiband_config *, size_t);
static cp_sample_t	cp_multiband_clean(cp_sample_t);
static int		cp_multiband_valid_config(
			    const struct cp_multiband_config *);

void
cp_multiband_default_config(struct cp_multiband_config *config)
{
	if (config == NULL)
		return;

	config->sample_rate = CP_MULTIBAND_DEFAULT_RATE;
	config->channels    = CP_CHANNELS_MONO;
	config->band_count  = CP_MULTIBAND_DEFAULT_BANDS;
	config->preset      = CP_MULTIBAND_PRESET_SPEECH;
	config->enabled     = CP_MULTIBAND_DEFAULT_ENABLED;
}

int
cp_multiband_init(struct cp_multiband *state,
	const struct cp_multiband_config *config)
{
	struct cp_compressor_config compressor_config;
	struct cp_crossover_config crossover_config;
	size_t band;
	int status;

	if (state == NULL || config == NULL)
		return CP_ERR_NULL;
	if (!cp_multiband_valid_config(config))
		return CP_ERR_RANGE;

	state->config     = *config;
	state->channels   = config->channels;
	state->band_count = config->band_count;
	state->enabled    = config->enabled;

	cp_crossover_default_config(&crossover_config, config->channels,
	    config->band_count);
	crossover_config.sample_rate = config->sample_rate;
	status = cp_crossover_init(&state->crossover, &crossover_config);
	if (status != CP_OK)
		return status;

	for (band = 0; band < CP_MULTIBAND_MAX_BANDS; band++) {
		cp_compressor_default_config(&compressor_config);
		compressor_config.sample_rate = config->sample_rate;
		compressor_config.channels    = config->channels;
		compressor_config.enabled     = config->enabled &&
		    band < config->band_count;
		cp_multiband_apply_preset(&compressor_config, config, band);
		status = cp_compressor_init(&state->compressor[band],
		    &compressor_config);
		if (status != CP_OK)
			return status;
	}

	return cp_multiband_reset(state);
}

int
cp_multiband_process(struct cp_multiband *state, const cp_sample_t *input,
	cp_sample_t *output, size_t frames)
{
	cp_sample_t band_frame[CP_MULTIBAND_MAX_BANDS][CP_MAX_CHANNELS];
	cp_sample_t band_output[CP_MULTIBAND_MAX_BANDS][CP_MAX_CHANNELS];
	cp_sample_t band_sum[CP_MULTIBAND_MAX_BANDS];
	cp_sample_t sample;
	cp_sample_t sum_sq[CP_MULTIBAND_MAX_BANDS];
	size_t band;
	size_t channel;
	size_t frame;
	size_t index;
	int status;

	if (state == NULL || input == NULL || output == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (frames > (SIZE_MAX / state->channels))
		return CP_ERR_RANGE;

	for (band = 0; band < CP_MULTIBAND_MAX_BANDS; band++) {
		state->band_peak[band]              = 0.0f;
		state->band_rms[band]               = 0.0f;
		state->band_gain_reduction_db[band] = 0.0f;
		sum_sq[band]                        = 0.0f;
	}

	if (!state->enabled) {
		for (frame = 0; frame < frames; frame++) {
			for (channel = 0; channel < state->channels; channel++) {
				index = (frame * state->channels) + channel;
				output[index] = cp_multiband_clean(input[index]);
			}
		}
		return CP_OK;
	}

	for (frame = 0; frame < frames; frame++) {
		for (band = 0; band < state->band_count; band++)
			band_sum[band] = 0.0f;

		for (channel = 0; channel < state->channels; channel++) {
			index = (frame * state->channels) + channel;
			status = cp_crossover_process_sample(&state->crossover,
			    input[index], channel, band_sum);
			if (status != CP_OK)
				return status;
			for (band = 0; band < state->band_count; band++)
				band_frame[band][channel] = band_sum[band];
		}

		for (band = 0; band < state->band_count; band++) {
			status = cp_compressor_process_frame(
			    &state->compressor[band], band_frame[band],
			    band_output[band]);
			if (status != CP_OK)
				return status;
			state->band_gain_reduction_db[band] =
			    state->compressor[band].gain_reduction_db;
		}

		for (channel = 0; channel < state->channels; channel++) {
			sample = 0.0f;
			for (band = 0; band < state->band_count; band++) {
				sample += band_output[band][channel];
				sum_sq[band] += band_output[band][channel] *
				    band_output[band][channel];
				if (fabsf(band_output[band][channel]) >
				    state->band_peak[band]) {
					state->band_peak[band] =
					    fabsf(band_output[band][channel]);
				}
			}
			index = (frame * state->channels) + channel;
			output[index] = cp_multiband_clean(sample);
		}
	}

	if (frames == 0)
		return CP_OK;

	for (band = 0; band < state->band_count; band++) {
		state->band_rms[band] = sqrtf(sum_sq[band] /
		    ((cp_sample_t)frames * (cp_sample_t)state->channels));
	}

	return CP_OK;
}

int
cp_multiband_reset(struct cp_multiband *state)
{
	size_t band;
	int status;

	if (state == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	status = cp_crossover_reset(&state->crossover);
	if (status != CP_OK)
		return status;

	for (band = 0; band < CP_MULTIBAND_MAX_BANDS; band++) {
		state->band_peak[band]              = 0.0f;
		state->band_rms[band]               = 0.0f;
		state->band_gain_reduction_db[band] = 0.0f;
		status = cp_compressor_reset(&state->compressor[band]);
		if (status != CP_OK)
			return status;
	}

	return CP_OK;
}

const char *
cp_multiband_preset_string(enum cp_multiband_preset preset)
{
	switch (preset) {
	case CP_MULTIBAND_PRESET_SPEECH:
		return "speech";
	case CP_MULTIBAND_PRESET_MUSIC:
		return "music";
	default:
		return "unknown";
	}
}

static void
cp_multiband_apply_preset(struct cp_compressor_config *compressor_config,
	const struct cp_multiband_config *config, size_t band)
{
	static const cp_sample_t music_ratio[CP_MULTIBAND_M5_MAX_BANDS] = {
		1.6f, 1.5f, 1.4f, 1.3f
	};
	static const cp_sample_t music_threshold[CP_MULTIBAND_M5_MAX_BANDS] = {
		-18.0f, -16.0f, -14.0f, -14.0f
	};
	static const cp_sample_t speech_ratio[CP_MULTIBAND_M5_MAX_BANDS] = {
		2.2f, 2.0f, 1.7f, 1.4f
	};
	static const cp_sample_t speech_threshold[CP_MULTIBAND_M5_MAX_BANDS] = {
		-24.0f, -20.0f, -18.0f, -18.0f
	};
	size_t index;

	index = band;
	if (index >= CP_MULTIBAND_M5_MAX_BANDS)
		index = CP_MULTIBAND_M5_MAX_BANDS - 1;

	if (config->preset == CP_MULTIBAND_PRESET_MUSIC) {
		compressor_config->threshold_db = music_threshold[index];
		compressor_config->ratio        = music_ratio[index];
		compressor_config->attack_ms    = 30.0f;
		compressor_config->release_ms   = 300.0f;
		compressor_config->knee_db      = 8.0f;
		return;
	}

	compressor_config->threshold_db = speech_threshold[index];
	compressor_config->ratio        = speech_ratio[index];
	compressor_config->attack_ms    = 15.0f;
	compressor_config->release_ms   = 180.0f;
	compressor_config->knee_db      = 6.0f;
}

static cp_sample_t
cp_multiband_clean(cp_sample_t sample)
{
	if (!isfinite(sample))
		return 0.0f;
	if (fabsf(sample) < CP_MULTIBAND_DENORMAL_FLOOR)
		return 0.0f;

	return sample;
}

static int
cp_multiband_valid_config(const struct cp_multiband_config *config)
{
	if (config == NULL)
		return 0;
	if (config->channels != CP_CHANNELS_MONO &&
	    config->channels != CP_CHANNELS_STEREO)
		return 0;
	if (config->band_count < CP_MULTIBAND_MIN_BANDS ||
	    config->band_count > CP_MULTIBAND_M5_MAX_BANDS)
		return 0;
	if (!isfinite(config->sample_rate) || config->sample_rate < 8000.0f ||
	    config->sample_rate > 384000.0f)
		return 0;
	if (config->preset != CP_MULTIBAND_PRESET_SPEECH &&
	    config->preset != CP_MULTIBAND_PRESET_MUSIC)
		return 0;

	return 1;
}

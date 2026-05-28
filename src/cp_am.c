/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_am.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cp_am.h"

#define CP_AM_FILTER_Q			(0.70710678f)
#define CP_AM_DENORMAL_FLOOR		(0.00000000000000000001f)

static cp_sample_t	cp_am_clean(cp_sample_t);
static cp_sample_t	cp_am_limit_sample(const struct cp_am *, cp_sample_t);
static cp_sample_t	cp_am_phase_frequency(size_t);
static void		cp_am_set_name(struct cp_am_config *, const char *);
static int		cp_am_valid_config(const struct cp_am_config *);

int
cp_am_apply_preset(struct cp_am_config *config, const char *preset_name)
{
	if (config == NULL || preset_name == NULL)
		return CP_ERR_NULL;

	if (strcmp(preset_name, "am-safe") == 0) {
		config->highpass_hz           = 60.0f;
		config->lowpass_hz            = 5000.0f;
		config->phase_rotator_enabled = 1;
		config->phase_rotator_stages  = 2;
		config->positive_peak_limit   = 0.95f;
		config->negative_peak_limit   = 0.95f;
		config->asymmetry_enabled     = 0;
		config->asymmetry_ratio       = 1.0f;
		cp_am_set_name(config, preset_name);
		return CP_OK;
	}
	if (strcmp(preset_name, "am-shortwave") == 0) {
		config->highpass_hz           = 80.0f;
		config->lowpass_hz            = 5000.0f;
		config->phase_rotator_enabled = 1;
		config->phase_rotator_stages  = 3;
		config->positive_peak_limit   = 0.95f;
		config->negative_peak_limit   = 0.90f;
		config->asymmetry_enabled     = 0;
		config->asymmetry_ratio       = 1.0f;
		cp_am_set_name(config, preset_name);
		return CP_OK;
	}
	if (strcmp(preset_name, "am-wide") == 0) {
		config->highpass_hz           = 40.0f;
		config->lowpass_hz            = 9000.0f;
		config->phase_rotator_enabled = 1;
		config->phase_rotator_stages  = 2;
		config->positive_peak_limit   = 0.98f;
		config->negative_peak_limit   = 0.95f;
		config->asymmetry_enabled     = 0;
		config->asymmetry_ratio       = 1.0f;
		cp_am_set_name(config, preset_name);
		return CP_OK;
	}
	if (strcmp(preset_name, "am-voice") == 0) {
		config->highpass_hz           = 120.0f;
		config->lowpass_hz            = 3500.0f;
		config->phase_rotator_enabled = 1;
		config->phase_rotator_stages  = 3;
		config->positive_peak_limit   = 0.95f;
		config->negative_peak_limit   = 0.90f;
		config->asymmetry_enabled     = 0;
		config->asymmetry_ratio       = 1.0f;
		cp_am_set_name(config, preset_name);
		return CP_OK;
	}

	return CP_ERR_RANGE;
}

void
cp_am_default_config(struct cp_am_config *config)
{
	if (config == NULL)
		return;

	config->enabled                 = CP_AM_DEFAULT_ENABLED;
	config->sample_rate             = CP_AM_DEFAULT_RATE;
	config->channel_count           = CP_CHANNELS_MONO;
	config->highpass_hz             = CP_AM_DEFAULT_HIGHPASS_HZ;
	config->lowpass_hz              = CP_AM_DEFAULT_LOWPASS_HZ;
	config->phase_rotator_enabled   = 1;
	config->phase_rotator_stages    = CP_AM_DEFAULT_PHASE_STAGES;
	config->positive_peak_limit     = CP_AM_DEFAULT_POS_PEAK;
	config->negative_peak_limit     = CP_AM_DEFAULT_NEG_PEAK;
	config->asymmetry_enabled       = 0;
	config->asymmetry_ratio         = CP_AM_DEFAULT_ASYM_RATIO;
	cp_am_set_name(config, "am-safe");
}

int
cp_am_init(struct cp_am *state, const struct cp_am_config *config)
{
	size_t stage;
	int status;

	if (state == NULL || config == NULL)
		return CP_ERR_NULL;
	if (!cp_am_valid_config(config))
		return CP_ERR_RANGE;

	state->config       = *config;
	state->channels     = config->channel_count;
	state->enabled      = config->enabled;
	state->phase_stages = config->phase_rotator_enabled ?
	    config->phase_rotator_stages : 0;

	status = cp_biquad_highpass(&state->highpass_coeff,
	    config->sample_rate, config->highpass_hz, CP_AM_FILTER_Q);
	if (status != CP_OK)
		return status;

	status = cp_biquad_lowpass(&state->lowpass_coeff,
	    config->sample_rate, config->lowpass_hz, CP_AM_FILTER_Q);
	if (status != CP_OK)
		return status;

	for (stage = 0; stage < CP_AM_MAX_PHASE_STAGES; stage++) {
		status = cp_biquad_allpass(&state->phase_coeff[stage],
		    config->sample_rate, cp_am_phase_frequency(stage),
		    CP_AM_FILTER_Q);
		if (status != CP_OK)
			return status;
	}

	return cp_am_reset(state);
}

int
cp_am_process(struct cp_am *state, const cp_sample_t *input,
	cp_sample_t *output, size_t frames)
{
	cp_sample_t sample;
	size_t channel;
	size_t frame;
	size_t index;
	size_t section;
	size_t stage;

	if (state == NULL || input == NULL || output == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (frames > (SIZE_MAX / state->channels))
		return CP_ERR_RANGE;

	for (frame = 0; frame < frames; frame++) {
		for (channel = 0; channel < state->channels; channel++) {
			index = (frame * state->channels) + channel;
			sample = cp_am_clean(input[index]);

			if (state->enabled) {
				for (section = 0; section < CP_AM_FILTER_SECTIONS;
				    section++) {
					sample = cp_biquad_process_sample(
					    &state->highpass_coeff,
					    &state->highpass_state[section][channel],
					    sample);
				}
				for (section = 0; section < CP_AM_FILTER_SECTIONS;
				    section++) {
					sample = cp_biquad_process_sample(
					    &state->lowpass_coeff,
					    &state->lowpass_state[section][channel],
					    sample);
				}
				for (stage = 0; stage < state->phase_stages;
				    stage++) {
					sample = cp_biquad_process_sample(
					    &state->phase_coeff[stage],
					    &state->phase_state[stage][channel],
					    sample);
				}
				sample = cp_am_limit_sample(state, sample);
			}

			output[index] = cp_am_clean(sample);
		}
	}

	return CP_OK;
}

int
cp_am_reset(struct cp_am *state)
{
	size_t channel;
	size_t section;
	size_t stage;

	if (state == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	for (section = 0; section < CP_AM_FILTER_SECTIONS; section++) {
		for (channel = 0; channel < CP_MAX_CHANNELS; channel++) {
			cp_biquad_reset(&state->highpass_state[section][channel]);
			cp_biquad_reset(&state->lowpass_state[section][channel]);
		}
	}
	for (stage = 0; stage < CP_AM_MAX_PHASE_STAGES; stage++) {
		for (channel = 0; channel < CP_MAX_CHANNELS; channel++)
			cp_biquad_reset(&state->phase_state[stage][channel]);
	}

	return CP_OK;
}

static cp_sample_t
cp_am_clean(cp_sample_t sample)
{
	if (!isfinite(sample))
		return 0.0f;
	if (fabsf(sample) < CP_AM_DENORMAL_FLOOR)
		return 0.0f;

	return sample;
}

static cp_sample_t
cp_am_limit_sample(const struct cp_am *state, cp_sample_t sample)
{
	cp_sample_t positive_limit;

	if (sample > 0.0f && state->config.asymmetry_enabled)
		sample *= state->config.asymmetry_ratio;

	positive_limit = state->config.positive_peak_limit;
	if (!state->config.asymmetry_enabled && positive_limit > 1.0f)
		positive_limit = 1.0f;

	if (sample > positive_limit)
		return positive_limit;
	if (sample < -state->config.negative_peak_limit)
		return -state->config.negative_peak_limit;

	return sample;
}

static cp_sample_t
cp_am_phase_frequency(size_t stage)
{
	static const cp_sample_t frequency[CP_AM_MAX_PHASE_STAGES] = {
		150.0f, 300.0f, 600.0f, 1200.0f
	};

	if (stage >= CP_AM_MAX_PHASE_STAGES)
		return frequency[CP_AM_MAX_PHASE_STAGES - 1];

	return frequency[stage];
}

static void
cp_am_set_name(struct cp_am_config *config, const char *preset_name)
{
	if (config == NULL || preset_name == NULL)
		return;

	(void)snprintf(config->preset_name, sizeof(config->preset_name), "%s",
	    preset_name);
}

static int
cp_am_valid_config(const struct cp_am_config *config)
{
	if (config == NULL)
		return 0;
	if (config->channel_count != CP_CHANNELS_MONO &&
	    config->channel_count != CP_CHANNELS_STEREO)
		return 0;
	if (!isfinite(config->sample_rate) || config->sample_rate < 8000.0f ||
	    config->sample_rate > 384000.0f)
		return 0;
	if (!isfinite(config->highpass_hz) ||
	    !isfinite(config->lowpass_hz))
		return 0;
	if (config->highpass_hz < CP_AM_MIN_HIGHPASS_HZ ||
	    config->highpass_hz > CP_AM_MAX_HIGHPASS_HZ)
		return 0;
	if (config->lowpass_hz < CP_AM_MIN_LOWPASS_HZ ||
	    config->lowpass_hz > CP_AM_MAX_LOWPASS_HZ)
		return 0;
	if (config->lowpass_hz >= (config->sample_rate * 0.5f))
		return 0;
	if (config->highpass_hz >= config->lowpass_hz)
		return 0;
	if (config->phase_rotator_stages > CP_AM_MAX_PHASE_STAGES)
		return 0;
	if (!isfinite(config->positive_peak_limit) ||
	    !isfinite(config->negative_peak_limit))
		return 0;
	if (config->positive_peak_limit < CP_AM_MIN_PEAK_LIMIT ||
	    config->positive_peak_limit > CP_AM_MAX_POS_PEAK_LIMIT)
		return 0;
	if (config->negative_peak_limit < CP_AM_MIN_PEAK_LIMIT ||
	    config->negative_peak_limit > CP_AM_MAX_NEG_PEAK_LIMIT)
		return 0;
	if (!isfinite(config->asymmetry_ratio) ||
	    config->asymmetry_ratio < CP_AM_MIN_ASYM_RATIO ||
	    config->asymmetry_ratio > CP_AM_MAX_ASYM_RATIO)
		return 0;
	if (!config->asymmetry_enabled &&
	    config->positive_peak_limit > CP_AM_MAX_NEG_PEAK_LIMIT)
		return 0;
	if (config->preset_name[0] == '\0')
		return 0;

	return 1;
}

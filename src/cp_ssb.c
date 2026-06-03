/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_ssb.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cp_ssb.h"

#define CP_SSB_FILTER_Q			(0.70710678f)
#define CP_SSB_DENORMAL_FLOOR		(0.00000000000000000001f)

static cp_sample_t	cp_ssb_clean(cp_sample_t);
static cp_sample_t	cp_ssb_limit_sample(const struct cp_ssb *,
			    cp_sample_t);
static cp_sample_t	cp_ssb_phase_frequency(size_t);
static void		cp_ssb_set_name(struct cp_ssb_config *,
			    const char *);
static int		cp_ssb_valid_config(const struct cp_ssb_config *);

int
cp_ssb_apply_preset(struct cp_ssb_config *config, const char *preset_name)
{
	enum cp_ssb_preset preset;

	if (config == NULL || preset_name == NULL)
		return CP_ERR_NULL;
	if (cp_ssb_preset_from_string(preset_name, &preset) != CP_OK)
		return CP_ERR_RANGE;

	return cp_ssb_apply_preset_id(config, preset);
}

int
cp_ssb_apply_preset_id(struct cp_ssb_config *config,
	enum cp_ssb_preset preset)
{
	const char *preset_name;

	if (config == NULL)
		return CP_ERR_NULL;

	preset_name = cp_ssb_preset_string(preset);
	if (preset_name == NULL)
		return CP_ERR_RANGE;

	if (preset == CP_SSB_PRESET_SPEECH) {
		config->highpass_hz           = 120.0f;
		config->lowpass_hz            = 2800.0f;
		config->phase_rotator_enabled = 1;
		config->phase_rotator_stages  = 2;
		config->peak_limit            = 0.95f;
		cp_ssb_set_name(config, preset_name);
		return CP_OK;
	}
	if (preset == CP_SSB_PRESET_NARROW) {
		config->highpass_hz           = 180.0f;
		config->lowpass_hz            = 2400.0f;
		config->phase_rotator_enabled = 1;
		config->phase_rotator_stages  = 3;
		config->peak_limit            = 0.90f;
		cp_ssb_set_name(config, preset_name);
		return CP_OK;
	}
	if (preset == CP_SSB_PRESET_WIDE) {
		config->highpass_hz           = 80.0f;
		config->lowpass_hz            = 3600.0f;
		config->phase_rotator_enabled = 1;
		config->phase_rotator_stages  = 2;
		config->peak_limit            = 0.95f;
		cp_ssb_set_name(config, preset_name);
		return CP_OK;
	}
	if (preset == CP_SSB_PRESET_GENTLE) {
		config->highpass_hz           = 100.0f;
		config->lowpass_hz            = 3000.0f;
		config->phase_rotator_enabled = 0;
		config->phase_rotator_stages  = 0;
		config->peak_limit            = 0.95f;
		cp_ssb_set_name(config, preset_name);
		return CP_OK;
	}

	return CP_ERR_RANGE;
}

void
cp_ssb_default_config(struct cp_ssb_config *config)
{
	if (config == NULL)
		return;

	config->enabled               = CP_SSB_DEFAULT_ENABLED;
	config->sample_rate           = CP_SSB_DEFAULT_RATE;
	config->channel_count         = CP_CHANNELS_MONO;
	config->highpass_hz           = CP_SSB_DEFAULT_HIGHPASS_HZ;
	config->lowpass_hz            = CP_SSB_DEFAULT_LOWPASS_HZ;
	config->phase_rotator_enabled = 1;
	config->phase_rotator_stages  = CP_SSB_DEFAULT_PHASE_STAGES;
	config->peak_limit            = CP_SSB_DEFAULT_PEAK;
	cp_ssb_set_name(config, "ssb-speech");
}

int
cp_ssb_init(struct cp_ssb *state, const struct cp_ssb_config *config)
{
	size_t stage;
	int status;

	if (state == NULL || config == NULL)
		return CP_ERR_NULL;
	if (!cp_ssb_valid_config(config))
		return CP_ERR_RANGE;

	state->config       = *config;
	state->channels     = config->channel_count;
	state->enabled      = config->enabled;
	state->phase_stages = config->phase_rotator_enabled ?
	    config->phase_rotator_stages : 0;

	status = cp_biquad_highpass(&state->highpass_coeff,
	    config->sample_rate, config->highpass_hz, CP_SSB_FILTER_Q);
	if (status != CP_OK)
		return status;

	status = cp_biquad_lowpass(&state->lowpass_coeff,
	    config->sample_rate, config->lowpass_hz, CP_SSB_FILTER_Q);
	if (status != CP_OK)
		return status;

	for (stage = 0; stage < CP_SSB_MAX_PHASE_STAGES; stage++) {
		status = cp_biquad_allpass(&state->phase_coeff[stage],
		    config->sample_rate, cp_ssb_phase_frequency(stage),
		    CP_SSB_FILTER_Q);
		if (status != CP_OK)
			return status;
	}

	return cp_ssb_reset(state);
}

int
cp_ssb_preset_from_string(const char *text, enum cp_ssb_preset *preset)
{
	if (text == NULL || preset == NULL)
		return CP_ERR_NULL;
	if (strcmp(text, "ssb-speech") == 0) {
		*preset = CP_SSB_PRESET_SPEECH;
		return CP_OK;
	}
	if (strcmp(text, "ssb-narrow") == 0) {
		*preset = CP_SSB_PRESET_NARROW;
		return CP_OK;
	}
	if (strcmp(text, "ssb-wide") == 0) {
		*preset = CP_SSB_PRESET_WIDE;
		return CP_OK;
	}
	if (strcmp(text, "ssb-gentle") == 0) {
		*preset = CP_SSB_PRESET_GENTLE;
		return CP_OK;
	}

	return CP_ERR_RANGE;
}

int
cp_ssb_process(struct cp_ssb *state, const cp_sample_t *input,
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
			sample = cp_ssb_clean(input[index]);

			if (state->enabled) {
				for (section = 0; section < CP_SSB_FILTER_SECTIONS;
				    section++) {
					sample = cp_biquad_process_sample(
					    &state->highpass_coeff,
					    &state->highpass_state[section][channel],
					    sample);
				}
				for (section = 0; section < CP_SSB_FILTER_SECTIONS;
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
				sample = cp_ssb_limit_sample(state, sample);
			}

			output[index] = cp_ssb_clean(sample);
		}
	}

	return CP_OK;
}

const char *
cp_ssb_preset_string(enum cp_ssb_preset preset)
{
	switch (preset) {
	case CP_SSB_PRESET_SPEECH:
		return "ssb-speech";
	case CP_SSB_PRESET_NARROW:
		return "ssb-narrow";
	case CP_SSB_PRESET_WIDE:
		return "ssb-wide";
	case CP_SSB_PRESET_GENTLE:
		return "ssb-gentle";
	default:
		return NULL;
	}
}

int
cp_ssb_reset(struct cp_ssb *state)
{
	size_t channel;
	size_t section;
	size_t stage;

	if (state == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	for (section = 0; section < CP_SSB_FILTER_SECTIONS; section++) {
		for (channel = 0; channel < CP_MAX_CHANNELS; channel++) {
			cp_biquad_reset(&state->highpass_state[section][channel]);
			cp_biquad_reset(&state->lowpass_state[section][channel]);
		}
	}
	for (stage = 0; stage < CP_SSB_MAX_PHASE_STAGES; stage++) {
		for (channel = 0; channel < CP_MAX_CHANNELS; channel++)
			cp_biquad_reset(&state->phase_state[stage][channel]);
	}

	return CP_OK;
}

static cp_sample_t
cp_ssb_clean(cp_sample_t sample)
{
	if (!isfinite(sample))
		return 0.0f;
	if (fabsf(sample) < CP_SSB_DENORMAL_FLOOR)
		return 0.0f;

	return sample;
}

static cp_sample_t
cp_ssb_limit_sample(const struct cp_ssb *state, cp_sample_t sample)
{
	if (sample > state->config.peak_limit)
		return state->config.peak_limit;
	if (sample < -state->config.peak_limit)
		return -state->config.peak_limit;

	return sample;
}

static cp_sample_t
cp_ssb_phase_frequency(size_t stage)
{
	static const cp_sample_t frequency[CP_SSB_MAX_PHASE_STAGES] = {
		150.0f, 300.0f, 600.0f, 1200.0f
	};

	if (stage >= CP_SSB_MAX_PHASE_STAGES)
		return frequency[CP_SSB_MAX_PHASE_STAGES - 1];

	return frequency[stage];
}

static void
cp_ssb_set_name(struct cp_ssb_config *config, const char *preset_name)
{
	if (config == NULL || preset_name == NULL)
		return;

	(void)snprintf(config->preset_name, sizeof(config->preset_name), "%s",
	    preset_name);
}

static int
cp_ssb_valid_config(const struct cp_ssb_config *config)
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
	if (config->highpass_hz < CP_SSB_MIN_HIGHPASS_HZ ||
	    config->highpass_hz > CP_SSB_MAX_HIGHPASS_HZ)
		return 0;
	if (config->lowpass_hz < CP_SSB_MIN_LOWPASS_HZ ||
	    config->lowpass_hz > CP_SSB_MAX_LOWPASS_HZ)
		return 0;
	if (config->lowpass_hz >= (config->sample_rate * 0.5f))
		return 0;
	if (config->highpass_hz >= config->lowpass_hz)
		return 0;
	if (config->phase_rotator_stages > CP_SSB_MAX_PHASE_STAGES)
		return 0;
	if (!isfinite(config->peak_limit) ||
	    config->peak_limit < CP_SSB_MIN_PEAK_LIMIT ||
	    config->peak_limit > CP_SSB_MAX_PEAK_LIMIT)
		return 0;
	if (config->preset_name[0] == '\0')
		return 0;

	return 1;
}

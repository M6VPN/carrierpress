/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_bass_eq.c */

#include <sys/types.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "cp_bass_eq.h"

#define CP_BASS_EQ_DENORMAL_FLOOR	0.00000000000000000001f

static cp_sample_t	cp_bass_eq_clean(cp_sample_t);
static cp_sample_t	cp_bass_eq_db_to_linear(cp_sample_t);
static int		cp_bass_eq_validate_config(
			    const struct cp_bass_eq_config *);

int
cp_bass_eq_apply_preset(struct cp_bass_eq_config *config, const char *name)
{
	enum cp_bass_eq_preset preset;

	if (config == NULL || name == NULL)
		return CP_ERR_NULL;
	if (cp_bass_eq_preset_from_string(name, &preset) != CP_OK)
		return CP_ERR_RANGE;

	return cp_bass_eq_apply_preset_id(config, preset);
}

int
cp_bass_eq_apply_preset_id(struct cp_bass_eq_config *config,
	enum cp_bass_eq_preset preset)
{
	if (config == NULL)
		return CP_ERR_NULL;

	switch (preset) {
	case CP_BASS_EQ_PRESET_FLAT:
		config->preset         = preset;
		config->low_shelf_hz   = 120.0f;
		config->low_gain_db    = 0.0f;
		config->high_shelf_hz  = 3000.0f;
		config->high_gain_db   = 0.0f;
		config->output_gain_db = 0.0f;
		break;
	case CP_BASS_EQ_PRESET_SPEECH:
		config->preset         = preset;
		config->low_shelf_hz   = 120.0f;
		config->low_gain_db    = -1.5f;
		config->high_shelf_hz  = 2800.0f;
		config->high_gain_db   = 1.5f;
		config->output_gain_db = -0.5f;
		break;
	case CP_BASS_EQ_PRESET_MUSIC:
		config->preset         = preset;
		config->low_shelf_hz   = 120.0f;
		config->low_gain_db    = 2.0f;
		config->high_shelf_hz  = 3500.0f;
		config->high_gain_db   = 0.5f;
		config->output_gain_db = -1.0f;
		break;
	case CP_BASS_EQ_PRESET_WARM:
		config->preset         = preset;
		config->low_shelf_hz   = 140.0f;
		config->low_gain_db    = 1.5f;
		config->high_shelf_hz  = 3000.0f;
		config->high_gain_db   = -0.5f;
		config->output_gain_db = -0.5f;
		break;
	default:
		return CP_ERR_RANGE;
	}

	return CP_OK;
}

void
cp_bass_eq_default_config(struct cp_bass_eq_config *config)
{
	if (config == NULL)
		return;

	config->enabled        = CP_BASS_EQ_DEFAULT_ENABLED;
	config->sample_rate    = CP_BASS_EQ_DEFAULT_RATE;
	config->channel_count  = CP_BASS_EQ_DEFAULT_CHANNELS;
	config->preset         = CP_BASS_EQ_PRESET_FLAT;
	config->low_shelf_hz   = CP_BASS_EQ_DEFAULT_LOW_HZ;
	config->low_gain_db    = CP_BASS_EQ_DEFAULT_GAIN_DB;
	config->high_shelf_hz  = CP_BASS_EQ_DEFAULT_HIGH_HZ;
	config->high_gain_db   = CP_BASS_EQ_DEFAULT_GAIN_DB;
	config->output_gain_db = CP_BASS_EQ_DEFAULT_OUTPUT_DB;
}

int
cp_bass_eq_init(struct cp_bass_eq *bass_eq,
	const struct cp_bass_eq_config *config)
{
	int status;

	if (bass_eq == NULL || config == NULL)
		return CP_ERR_NULL;
	status = cp_bass_eq_validate_config(config);
	if (status != CP_OK)
		return status;

	(void)memset(bass_eq, 0, sizeof(*bass_eq));
	bass_eq->config      = *config;
	bass_eq->channels    = config->channel_count;
	bass_eq->enabled     = config->enabled ? 1 : 0;
	bass_eq->output_gain = cp_bass_eq_db_to_linear(config->output_gain_db);
	if (!isfinite(bass_eq->output_gain) || bass_eq->output_gain <= 0.0f)
		return CP_ERR_RANGE;

	status = cp_biquad_low_shelf(&bass_eq->low_coeff,
	    config->sample_rate, config->low_shelf_hz, config->low_gain_db);
	if (status != CP_OK)
		return status;

	status = cp_biquad_high_shelf(&bass_eq->high_coeff,
	    config->sample_rate, config->high_shelf_hz, config->high_gain_db);
	if (status != CP_OK)
		return status;

	return cp_bass_eq_reset(bass_eq);
}

int
cp_bass_eq_preset_from_string(const char *name,
	enum cp_bass_eq_preset *preset)
{
	if (name == NULL || preset == NULL)
		return CP_ERR_NULL;
	if (strcmp(name, "flat") == 0) {
		*preset = CP_BASS_EQ_PRESET_FLAT;
		return CP_OK;
	}
	if (strcmp(name, "speech") == 0) {
		*preset = CP_BASS_EQ_PRESET_SPEECH;
		return CP_OK;
	}
	if (strcmp(name, "music") == 0) {
		*preset = CP_BASS_EQ_PRESET_MUSIC;
		return CP_OK;
	}
	if (strcmp(name, "warm") == 0) {
		*preset = CP_BASS_EQ_PRESET_WARM;
		return CP_OK;
	}

	return CP_ERR_RANGE;
}

const char *
cp_bass_eq_preset_string(enum cp_bass_eq_preset preset)
{
	switch (preset) {
	case CP_BASS_EQ_PRESET_FLAT:
		return "flat";
	case CP_BASS_EQ_PRESET_SPEECH:
		return "speech";
	case CP_BASS_EQ_PRESET_MUSIC:
		return "music";
	case CP_BASS_EQ_PRESET_WARM:
		return "warm";
	default:
		return "unknown";
	}
}

int
cp_bass_eq_process(struct cp_bass_eq *bass_eq, const cp_sample_t *input,
	cp_sample_t *output, size_t frames)
{
	cp_sample_t sample;
	size_t frame;
	size_t channel;
	size_t index;

	if (bass_eq == NULL || input == NULL || output == NULL)
		return CP_ERR_NULL;
	if (bass_eq->channels != CP_CHANNELS_MONO &&
	    bass_eq->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (frames == 0 || frames > (SIZE_MAX / bass_eq->channels))
		return CP_ERR_RANGE;

	for (frame = 0; frame < frames; frame++) {
		for (channel = 0; channel < bass_eq->channels; channel++) {
			index = (frame * bass_eq->channels) + channel;
			sample = cp_bass_eq_clean(input[index]);
			if (bass_eq->enabled) {
				sample = cp_biquad_process_sample(
				    &bass_eq->low_coeff,
				    &bass_eq->low_state[channel], sample);
				sample = cp_biquad_process_sample(
				    &bass_eq->high_coeff,
				    &bass_eq->high_state[channel], sample);
				sample *= bass_eq->output_gain;
			}
			output[index] = cp_bass_eq_clean(sample);
		}
	}

	return CP_OK;
}

int
cp_bass_eq_reset(struct cp_bass_eq *bass_eq)
{
	size_t channel;
	int status;

	if (bass_eq == NULL)
		return CP_ERR_NULL;
	if (bass_eq->channels != CP_CHANNELS_MONO &&
	    bass_eq->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	for (channel = 0; channel < bass_eq->channels; channel++) {
		status = cp_biquad_reset(&bass_eq->low_state[channel]);
		if (status != CP_OK)
			return status;
		status = cp_biquad_reset(&bass_eq->high_state[channel]);
		if (status != CP_OK)
			return status;
	}

	return CP_OK;
}

static cp_sample_t
cp_bass_eq_clean(cp_sample_t sample)
{
	if (!isfinite(sample))
		return 0.0f;
	if (fabsf(sample) < CP_BASS_EQ_DENORMAL_FLOOR)
		return 0.0f;

	return sample;
}

static cp_sample_t
cp_bass_eq_db_to_linear(cp_sample_t db)
{
	if (!isfinite(db))
		return 1.0f;

	return powf(10.0f, db / 20.0f);
}

static int
cp_bass_eq_validate_config(const struct cp_bass_eq_config *config)
{
	cp_sample_t nyquist;

	if (config == NULL)
		return CP_ERR_NULL;
	if (config->channel_count != CP_CHANNELS_MONO &&
	    config->channel_count != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (!isfinite(config->sample_rate) ||
	    config->sample_rate < CP_BASS_EQ_MIN_RATE ||
	    config->sample_rate > CP_BASS_EQ_MAX_RATE)
		return CP_ERR_RANGE;
	if (config->preset != CP_BASS_EQ_PRESET_FLAT &&
	    config->preset != CP_BASS_EQ_PRESET_SPEECH &&
	    config->preset != CP_BASS_EQ_PRESET_MUSIC &&
	    config->preset != CP_BASS_EQ_PRESET_WARM)
		return CP_ERR_RANGE;
	if (!isfinite(config->low_shelf_hz) ||
	    config->low_shelf_hz < CP_BASS_EQ_MIN_LOW_HZ ||
	    config->low_shelf_hz > CP_BASS_EQ_MAX_LOW_HZ)
		return CP_ERR_RANGE;
	if (!isfinite(config->high_shelf_hz) ||
	    config->high_shelf_hz < CP_BASS_EQ_MIN_HIGH_HZ ||
	    config->high_shelf_hz > CP_BASS_EQ_MAX_HIGH_HZ)
		return CP_ERR_RANGE;

	nyquist = config->sample_rate * 0.5f;
	if (config->low_shelf_hz >= nyquist ||
	    config->high_shelf_hz >= nyquist ||
	    config->low_shelf_hz >= config->high_shelf_hz)
		return CP_ERR_RANGE;
	if (!isfinite(config->low_gain_db) ||
	    config->low_gain_db < CP_BASS_EQ_MIN_GAIN_DB ||
	    config->low_gain_db > CP_BASS_EQ_MAX_GAIN_DB)
		return CP_ERR_RANGE;
	if (!isfinite(config->high_gain_db) ||
	    config->high_gain_db < CP_BASS_EQ_MIN_GAIN_DB ||
	    config->high_gain_db > CP_BASS_EQ_MAX_GAIN_DB)
		return CP_ERR_RANGE;
	if (!isfinite(config->output_gain_db) ||
	    config->output_gain_db < CP_BASS_EQ_MIN_OUTPUT_DB ||
	    config->output_gain_db > CP_BASS_EQ_MAX_OUTPUT_DB)
		return CP_ERR_RANGE;

	return CP_OK;
}

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_audio.c */

#include <sys/types.h>

#include <ctype.h>
#include <math.h>
#include <string.h>

#include "cp_audio.h"
#include "cp_bass_eq.h"
#include "cp_declipper.h"
#include "cp_dehummer.h"
#include "cp_low_level_boost.h"
#include "cp_natural_dynamics.h"
#include "cp_restoration.h"

static int	cp_audio_candidate_full_duplex(
		    const struct cp_audio_config *,
		    const struct cp_audio_device_candidate *);
static int	cp_audio_find_backend(
		    const struct cp_audio_device_candidate *, size_t,
		    const struct cp_audio_config *, enum cp_audio_backend,
		    int *);
static int	cp_audio_find_named(
		    const struct cp_audio_device_candidate *, size_t,
		    const struct cp_audio_config *, const char *, int *);
static int	cp_audio_name_contains(const char *, const char *);
static int	cp_audio_rate_valid(double);

int
cp_audio_backend_from_string(const char *text,
	enum cp_audio_backend *backend)
{
	if (text == NULL || backend == NULL)
		return CP_AUDIO_ERR_NULL;
	if (strcmp(text, "auto") == 0) {
		*backend = CP_AUDIO_BACKEND_AUTO;
		return CP_AUDIO_OK;
	}
	if (strcmp(text, "jack") == 0) {
		*backend = CP_AUDIO_BACKEND_JACK;
		return CP_AUDIO_OK;
	}
	if (strcmp(text, "alsa") == 0) {
		*backend = CP_AUDIO_BACKEND_ALSA;
		return CP_AUDIO_OK;
	}
	if (strcmp(text, "pulse") == 0) {
		*backend = CP_AUDIO_BACKEND_PULSE;
		return CP_AUDIO_OK;
	}
	if (strcmp(text, "default") == 0) {
		*backend = CP_AUDIO_BACKEND_DEFAULT;
		return CP_AUDIO_OK;
	}

	return CP_AUDIO_ERR_BACKEND;
}

const char *
cp_audio_backend_string(enum cp_audio_backend backend)
{
	switch (backend) {
	case CP_AUDIO_BACKEND_AUTO:
		return "auto";
	case CP_AUDIO_BACKEND_JACK:
		return "jack";
	case CP_AUDIO_BACKEND_ALSA:
		return "alsa";
	case CP_AUDIO_BACKEND_PULSE:
		return "pulse";
	case CP_AUDIO_BACKEND_DEFAULT:
		return "default";
	default:
		return "unknown";
	}
}

int
cp_audio_choose_sample_rate(const struct cp_audio_config *config,
	double input_default_rate, double output_default_rate,
	int requested_supported, int input_default_supported,
	int output_default_supported, double *sample_rate)
{
	if (config == NULL || sample_rate == NULL)
		return CP_AUDIO_ERR_NULL;
	if (requested_supported) {
		*sample_rate = config->sample_rate;
		return CP_AUDIO_OK;
	}
	if (config->sample_rate_explicit)
		return CP_AUDIO_ERR_RATE;
	if (input_default_supported && cp_audio_rate_valid(input_default_rate)) {
		*sample_rate = input_default_rate;
		return CP_AUDIO_OK;
	}
	if (output_default_supported &&
	    cp_audio_rate_valid(output_default_rate)) {
		*sample_rate = output_default_rate;
		return CP_AUDIO_OK;
	}

	return CP_AUDIO_ERR_RATE;
}

void
cp_audio_default_config(struct cp_audio_config *config)
{
	if (config == NULL)
		return;

	config->input_device      = CP_AUDIO_DEFAULT_DEVICE;
	config->output_device     = CP_AUDIO_DEFAULT_DEVICE;
	config->backend           = CP_AUDIO_BACKEND_AUTO;
	config->device_name       = NULL;
	config->sample_rate       = CP_AUDIO_DEFAULT_SAMPLE_RATE;
	config->sample_rate_explicit = 0;
	config->channels          = CP_AUDIO_DEFAULT_CHANNELS;
	config->block_size        = CP_AUDIO_DEFAULT_BLOCK_SIZE;
	config->meter_interval_ms = CP_AUDIO_DEFAULT_METER_MS;
	config->dehummer_enabled  = CP_DEHUMMER_DEFAULT_ENABLED;
	config->hum_base_frequency = CP_DEHUMMER_DEFAULT_BASE_HZ;
	config->hum_harmonic_count = CP_DEHUMMER_DEFAULT_HARMONICS;
	config->hum_q_factor      = CP_DEHUMMER_DEFAULT_Q;
	config->multiband_enabled = CP_MULTIBAND_DEFAULT_ENABLED;
	config->multiband_band_count = CP_MULTIBAND_DEFAULT_BANDS;
	config->multiband_preset  = CP_MULTIBAND_PRESET_SPEECH;
	cp_bass_eq_default_config(&config->bass_eq_config);
	config->bass_eq_config.channel_count = config->channels;
	config->bass_eq_config.sample_rate = (cp_sample_t)config->sample_rate;
	cp_am_default_config(&config->am_config);
	config->am_config.channel_count = config->channels;
	config->am_config.sample_rate = (cp_sample_t)config->sample_rate;
	cp_declipper_default_config(&config->declipper_config);
	config->declipper_config.channel_count = config->channels;
	config->declipper_config.sample_rate =
	    (cp_sample_t)config->sample_rate;
	cp_natural_dynamics_default_config(&config->natural_dynamics_config);
	config->natural_dynamics_config.channel_count = config->channels;
	config->natural_dynamics_config.sample_rate =
	    (cp_sample_t)config->sample_rate;
	cp_low_level_boost_default_config(&config->low_level_boost_config);
	config->low_level_boost_config.channel_count = config->channels;
	config->low_level_boost_config.sample_rate =
	    (cp_sample_t)config->sample_rate;
	cp_restoration_default_config(&config->restoration_config);
	config->restoration_config.channel_count = config->channels;
	config->restoration_config.sample_rate =
	    (cp_sample_t)config->sample_rate;
	cp_ssb_default_config(&config->ssb_config);
	config->ssb_config.channel_count = config->channels;
	config->ssb_config.sample_rate = (cp_sample_t)config->sample_rate;
	config->tui_enabled = 0;
}

int
cp_audio_select_device_candidate(const struct cp_audio_config *config,
	const struct cp_audio_device_candidate *candidates, size_t count,
	int *device_index)
{
	int status;

	if (config == NULL || candidates == NULL || device_index == NULL)
		return CP_AUDIO_ERR_NULL;

	*device_index = CP_AUDIO_DEFAULT_DEVICE;
	if (config->input_device != CP_AUDIO_DEFAULT_DEVICE ||
	    config->output_device != CP_AUDIO_DEFAULT_DEVICE)
		return CP_AUDIO_OK;

	if (config->device_name != NULL) {
		if (config->device_name[0] == '\0')
			return CP_AUDIO_ERR_DEVICE;
		return cp_audio_find_named(candidates, count, config,
		    config->device_name, device_index);
	}

	switch (config->backend) {
	case CP_AUDIO_BACKEND_AUTO:
		status = cp_audio_find_backend(candidates, count, config,
		    CP_AUDIO_BACKEND_JACK, device_index);
		if (status == CP_AUDIO_OK)
			return CP_AUDIO_OK;
		status = cp_audio_find_named(candidates, count, config,
		    "pipewire", device_index);
		if (status == CP_AUDIO_OK)
			return CP_AUDIO_OK;
		status = cp_audio_find_named(candidates, count, config,
		    "pulse", device_index);
		if (status == CP_AUDIO_OK)
			return CP_AUDIO_OK;
		status = cp_audio_find_named(candidates, count, config,
		    "default", device_index);
		if (status == CP_AUDIO_OK)
			return CP_AUDIO_OK;
		return CP_AUDIO_OK;
	case CP_AUDIO_BACKEND_JACK:
	case CP_AUDIO_BACKEND_ALSA:
	case CP_AUDIO_BACKEND_PULSE:
		return cp_audio_find_backend(candidates, count, config,
		    config->backend, device_index);
	case CP_AUDIO_BACKEND_DEFAULT:
		return CP_AUDIO_OK;
	default:
		return CP_AUDIO_ERR_BACKEND;
	}
}

const char *
cp_audio_status_string(int status)
{
	switch (status) {
	case CP_AUDIO_OK:
		return "ok";
	case CP_AUDIO_ERR_NULL:
		return "missing audio config";
	case CP_AUDIO_ERR_DEVICE:
		return "invalid audio device";
	case CP_AUDIO_ERR_RATE:
		return "invalid sample rate";
	case CP_AUDIO_ERR_CHANNEL:
		return "invalid channel count";
	case CP_AUDIO_ERR_BLOCK:
		return "invalid block size";
	case CP_AUDIO_ERR_METER:
		return "invalid meter interval";
	case CP_AUDIO_ERR_HUM:
		return "invalid dehummer settings";
	case CP_AUDIO_ERR_MB:
		return "invalid multiband settings";
	case CP_AUDIO_ERR_AM:
		return "invalid AM settings";
	case CP_AUDIO_ERR_BACKEND:
		return "invalid audio backend";
	case CP_AUDIO_ERR_SSB:
		return "invalid SSB settings";
	case CP_AUDIO_ERR_BASS_EQ:
		return "invalid bass EQ settings";
	case CP_AUDIO_ERR_RESTORATION:
		return "invalid restoration analysis settings";
	case CP_AUDIO_ERR_DECLIPPER:
		return "invalid declipper settings";
	case CP_AUDIO_ERR_NATURAL_DYNAMICS:
		return "invalid natural dynamics settings";
	case CP_AUDIO_ERR_LOW_LEVEL_BOOST:
		return "invalid low-level boost settings";
	default:
		return "unknown audio error";
	}
}

int
cp_audio_validate_config(const struct cp_audio_config *config)
{
	struct cp_am am;
	struct cp_bass_eq bass_eq;
	struct cp_declipper declipper;
	struct cp_low_level_boost low_level_boost;
	struct cp_natural_dynamics natural_dynamics;
	struct cp_restoration restoration;
	struct cp_ssb ssb;

	if (config == NULL)
		return CP_AUDIO_ERR_NULL;
	if (config->input_device < CP_AUDIO_DEFAULT_DEVICE ||
	    config->output_device < CP_AUDIO_DEFAULT_DEVICE)
		return CP_AUDIO_ERR_DEVICE;
	if (config->device_name != NULL &&
	    config->device_name[0] == '\0')
		return CP_AUDIO_ERR_DEVICE;
	if (config->device_name != NULL &&
	    (config->input_device != CP_AUDIO_DEFAULT_DEVICE ||
	    config->output_device != CP_AUDIO_DEFAULT_DEVICE))
		return CP_AUDIO_ERR_DEVICE;
	if (config->backend != CP_AUDIO_BACKEND_AUTO &&
	    config->backend != CP_AUDIO_BACKEND_JACK &&
	    config->backend != CP_AUDIO_BACKEND_ALSA &&
	    config->backend != CP_AUDIO_BACKEND_PULSE &&
	    config->backend != CP_AUDIO_BACKEND_DEFAULT)
		return CP_AUDIO_ERR_BACKEND;
	if (config->sample_rate < CP_AUDIO_MIN_SAMPLE_RATE ||
	    config->sample_rate > CP_AUDIO_MAX_SAMPLE_RATE)
		return CP_AUDIO_ERR_RATE;
	if (config->channels != CP_CHANNELS_MONO &&
	    config->channels != CP_CHANNELS_STEREO)
		return CP_AUDIO_ERR_CHANNEL;
	if (config->block_size < CP_AUDIO_MIN_BLOCK_SIZE ||
	    config->block_size > CP_AUDIO_MAX_BLOCK_SIZE)
		return CP_AUDIO_ERR_BLOCK;
	if (config->meter_interval_ms < CP_AUDIO_MIN_METER_MS ||
	    config->meter_interval_ms > CP_AUDIO_MAX_METER_MS)
		return CP_AUDIO_ERR_METER;
	if (config->hum_base_frequency < CP_DEHUMMER_MIN_BASE_HZ ||
	    config->hum_base_frequency > CP_DEHUMMER_MAX_BASE_HZ)
		return CP_AUDIO_ERR_HUM;
	if (config->hum_harmonic_count == 0 ||
	    config->hum_harmonic_count > CP_DEHUMMER_MAX_HARMONICS)
		return CP_AUDIO_ERR_HUM;
	if (config->hum_q_factor < CP_DEHUMMER_MIN_Q ||
	    config->hum_q_factor > CP_DEHUMMER_MAX_Q)
		return CP_AUDIO_ERR_HUM;
	if ((config->hum_base_frequency *
	    (cp_sample_t)config->hum_harmonic_count) >=
	    (cp_sample_t)(config->sample_rate * 0.5))
		return CP_AUDIO_ERR_HUM;
	if (config->multiband_band_count < CP_MULTIBAND_MIN_BANDS ||
	    config->multiband_band_count > CP_MULTIBAND_M5_MAX_BANDS)
		return CP_AUDIO_ERR_MB;
	if (config->multiband_preset != CP_MULTIBAND_PRESET_SPEECH &&
	    config->multiband_preset != CP_MULTIBAND_PRESET_MUSIC)
		return CP_AUDIO_ERR_MB;
	if (config->bass_eq_config.channel_count != config->channels)
		return CP_AUDIO_ERR_BASS_EQ;
	if (config->bass_eq_config.sample_rate !=
	    (cp_sample_t)config->sample_rate)
		return CP_AUDIO_ERR_BASS_EQ;
	if (config->am_config.channel_count != config->channels)
		return CP_AUDIO_ERR_AM;
	if (config->am_config.sample_rate != (cp_sample_t)config->sample_rate)
		return CP_AUDIO_ERR_AM;
	if (config->declipper_config.channel_count != config->channels)
		return CP_AUDIO_ERR_DECLIPPER;
	if (config->declipper_config.sample_rate !=
	    (cp_sample_t)config->sample_rate)
		return CP_AUDIO_ERR_DECLIPPER;
	if (config->natural_dynamics_config.channel_count !=
	    config->channels)
		return CP_AUDIO_ERR_NATURAL_DYNAMICS;
	if (config->natural_dynamics_config.sample_rate !=
	    (cp_sample_t)config->sample_rate)
		return CP_AUDIO_ERR_NATURAL_DYNAMICS;
	if (config->low_level_boost_config.channel_count !=
	    config->channels)
		return CP_AUDIO_ERR_LOW_LEVEL_BOOST;
	if (config->low_level_boost_config.sample_rate !=
	    (cp_sample_t)config->sample_rate)
		return CP_AUDIO_ERR_LOW_LEVEL_BOOST;
	if (config->ssb_config.channel_count != config->channels)
		return CP_AUDIO_ERR_SSB;
	if (config->ssb_config.sample_rate != (cp_sample_t)config->sample_rate)
		return CP_AUDIO_ERR_SSB;
	if (config->restoration_config.channel_count != config->channels)
		return CP_AUDIO_ERR_RESTORATION;
	if (config->restoration_config.sample_rate !=
	    (cp_sample_t)config->sample_rate)
		return CP_AUDIO_ERR_RESTORATION;
	if (config->am_config.enabled && config->ssb_config.enabled)
		return CP_AUDIO_ERR_SSB;
	if (config->am_config.enabled) {
		if (cp_am_init(&am, &config->am_config) != CP_OK)
			return CP_AUDIO_ERR_AM;
	}
	if (config->bass_eq_config.enabled) {
		if (cp_bass_eq_init(&bass_eq,
		    &config->bass_eq_config) != CP_OK)
			return CP_AUDIO_ERR_BASS_EQ;
	}
	if (config->declipper_config.enabled) {
		if (cp_declipper_init(&declipper,
		    &config->declipper_config) != CP_OK)
			return CP_AUDIO_ERR_DECLIPPER;
	}
	if (config->natural_dynamics_config.enabled) {
		if (cp_natural_dynamics_init(&natural_dynamics,
		    &config->natural_dynamics_config) != CP_OK)
			return CP_AUDIO_ERR_NATURAL_DYNAMICS;
	}
	if (config->low_level_boost_config.enabled) {
		if (cp_low_level_boost_init(&low_level_boost,
		    &config->low_level_boost_config) != CP_OK)
			return CP_AUDIO_ERR_LOW_LEVEL_BOOST;
	}
	if (config->ssb_config.enabled) {
		if (cp_ssb_init(&ssb, &config->ssb_config) != CP_OK)
			return CP_AUDIO_ERR_SSB;
	}
	if (config->restoration_config.enabled) {
		if (cp_restoration_init(&restoration,
		    &config->restoration_config) != CP_OK)
			return CP_AUDIO_ERR_RESTORATION;
	}

	return CP_AUDIO_OK;
}

static int
cp_audio_candidate_full_duplex(const struct cp_audio_config *config,
	const struct cp_audio_device_candidate *candidate)
{
	if (candidate == NULL || config == NULL)
		return 0;
	if (candidate->max_input_channels < (int)config->channels)
		return 0;
	if (candidate->max_output_channels < (int)config->channels)
		return 0;

	return 1;
}

static int
cp_audio_find_backend(const struct cp_audio_device_candidate *candidates,
	size_t count, const struct cp_audio_config *config,
	enum cp_audio_backend backend, int *device_index)
{
	const char *needle;
	size_t candidate;

	needle = cp_audio_backend_string(backend);
	for (candidate = 0; candidate < count; candidate++) {
		if (!cp_audio_candidate_full_duplex(config,
		    &candidates[candidate]))
			continue;
		if (cp_audio_name_contains(candidates[candidate].host_api,
		    needle) ||
		    cp_audio_name_contains(candidates[candidate].name, needle)) {
			*device_index = candidates[candidate].index;
			return CP_AUDIO_OK;
		}
	}

	return CP_AUDIO_ERR_DEVICE;
}

static int
cp_audio_find_named(const struct cp_audio_device_candidate *candidates,
	size_t count, const struct cp_audio_config *config, const char *name,
	int *device_index)
{
	size_t candidate;

	if (name == NULL)
		return CP_AUDIO_ERR_DEVICE;

	for (candidate = 0; candidate < count; candidate++) {
		if (!cp_audio_candidate_full_duplex(config,
		    &candidates[candidate]))
			continue;
		if (cp_audio_name_contains(candidates[candidate].name, name)) {
			*device_index = candidates[candidate].index;
			return CP_AUDIO_OK;
		}
	}

	return CP_AUDIO_ERR_DEVICE;
}

static int
cp_audio_name_contains(const char *text, const char *needle)
{
	size_t index;
	size_t offset;

	if (text == NULL || needle == NULL || needle[0] == '\0')
		return 0;

	for (offset = 0; text[offset] != '\0'; offset++) {
		for (index = 0; needle[index] != '\0'; index++) {
			if (text[offset + index] == '\0')
				return 0;
			if (tolower((unsigned char)text[offset + index]) !=
			    tolower((unsigned char)needle[index]))
				break;
		}
		if (needle[index] == '\0')
			return 1;
	}

	return 0;
}

static int
cp_audio_rate_valid(double rate)
{
	if (!isfinite(rate))
		return 0;
	if (rate < CP_AUDIO_MIN_SAMPLE_RATE || rate > CP_AUDIO_MAX_SAMPLE_RATE)
		return 0;

	return 1;
}

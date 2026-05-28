/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_audio.c */

#include "cp_audio.h"
#include "cp_dehummer.h"

void
cp_audio_default_config(struct cp_audio_config *config)
{
	if (config == NULL)
		return;

	config->input_device      = CP_AUDIO_DEFAULT_DEVICE;
	config->output_device     = CP_AUDIO_DEFAULT_DEVICE;
	config->sample_rate       = CP_AUDIO_DEFAULT_SAMPLE_RATE;
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
	cp_am_default_config(&config->am_config);
	config->am_config.channel_count = config->channels;
	config->am_config.sample_rate = (cp_sample_t)config->sample_rate;
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
	default:
		return "unknown audio error";
	}
}

int
cp_audio_validate_config(const struct cp_audio_config *config)
{
	struct cp_am am;

	if (config == NULL)
		return CP_AUDIO_ERR_NULL;
	if (config->input_device < CP_AUDIO_DEFAULT_DEVICE ||
	    config->output_device < CP_AUDIO_DEFAULT_DEVICE)
		return CP_AUDIO_ERR_DEVICE;
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
	if (config->am_config.channel_count != config->channels)
		return CP_AUDIO_ERR_AM;
	if (config->am_config.sample_rate != (cp_sample_t)config->sample_rate)
		return CP_AUDIO_ERR_AM;
	if (config->am_config.enabled) {
		if (cp_am_init(&am, &config->am_config) != CP_OK)
			return CP_AUDIO_ERR_AM;
	}

	return CP_AUDIO_OK;
}

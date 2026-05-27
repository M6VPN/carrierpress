/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_audio.c */

#include "cp_audio.h"

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
	default:
		return "unknown audio error";
	}
}

int
cp_audio_validate_config(const struct cp_audio_config *config)
{
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

	return CP_AUDIO_OK;
}

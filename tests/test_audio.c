/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_audio.c */

#include <sys/types.h>

#include <stdio.h>

#include "cp_audio.h"

int
main(void)
{
	struct cp_audio_config config;

	cp_audio_default_config(&config);
	if (cp_audio_validate_config(&config) != CP_AUDIO_OK) {
		printf("test_audio: default config rejected\n");
		return 1;
	}

	config.sample_rate = CP_AUDIO_MIN_SAMPLE_RATE - 1.0;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_RATE) {
		printf("test_audio: invalid sample rate accepted\n");
		return 1;
	}

	cp_audio_default_config(&config);
	config.channels = 3;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_CHANNEL) {
		printf("test_audio: invalid channel count accepted\n");
		return 1;
	}

	cp_audio_default_config(&config);
	config.block_size = CP_AUDIO_MAX_BLOCK_SIZE + 1;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_BLOCK) {
		printf("test_audio: invalid block size accepted\n");
		return 1;
	}

	cp_audio_default_config(&config);
	config.input_device = -2;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_DEVICE) {
		printf("test_audio: invalid device accepted\n");
		return 1;
	}

	cp_audio_default_config(&config);
	config.meter_interval_ms = CP_AUDIO_MIN_METER_MS - 1;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_METER) {
		printf("test_audio: invalid meter interval accepted\n");
		return 1;
	}

	cp_audio_default_config(&config);
	config.multiband_band_count = CP_MULTIBAND_M5_MAX_BANDS + 1;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_MB) {
		printf("test_audio: invalid multiband band count accepted\n");
		return 1;
	}

	return 0;
}

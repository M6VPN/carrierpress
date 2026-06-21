/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/examples/libcarrierpress-minimal.c */

#include <sys/types.h>

#include <stdio.h>

#include "carrierpress.h"

int
main(void)
{
	enum {
		frames = 16,
		channels = CP_CHANNELS_MONO
	};
	struct cp_block_config config;
	struct cp_block_processor processor;
	struct cp_monitor_snapshot monitor;
	cp_sample_t input[frames * channels];
	cp_sample_t output[frames * channels];
	cp_sample_t scratch[frames * channels];
	size_t i;
	int status;

	for (i = 0; i < frames; i++)
		input[i] = (i & 1u) ? 0.10f : -0.10f;

	cp_block_default_config(&config, channels);
	config.sample_rate = 48000.0f;

	status = cp_block_init(&processor, &config);
	if (status != CP_OK) {
		(void)fprintf(stderr, "cp_block_init failed: %d\n", status);
		return 1;
	}

	status = cp_block_process(&processor, input, output, scratch,
	    frames * channels, frames);
	if (status != CP_OK) {
		(void)fprintf(stderr, "cp_block_process failed: %d\n", status);
		return 1;
	}

	status = cp_monitor_snapshot_from_processor(&processor, &monitor);
	if (status != CP_OK) {
		(void)fprintf(stderr, "monitor snapshot failed: %d\n", status);
		return 1;
	}

	(void)printf("carrierpress_version=%s\n", CP_VERSION_STRING);
	(void)printf("status=ok\n");
	(void)printf("output_peak=%.6f\n",
	    (double)cp_monitor_level_to_sample(monitor.output_peak));
	(void)printf("output_rms=%.6f\n",
	    (double)cp_monitor_level_to_sample(monitor.output_rms));

	return 0;
}

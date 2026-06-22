/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/examples/libcarrierpress-core-smoke.c */

#include <sys/types.h>

#include <stdio.h>

#include "carrierpress_core.h"

int
main(void)
{
	enum {
		frames = 8,
		channels = CP_CHANNELS_MONO
	};
	struct cp_block_config config;
	struct cp_block_processor processor;
	struct cp_monitor_snapshot monitor;
	cp_sample_t input[frames * channels];
	cp_sample_t output[frames * channels];
	cp_sample_t scratch[frames * channels];
	size_t index;
	int status;

	for (index = 0; index < frames; index++)
		input[index] = index < 4 ? 0.05f : -0.05f;

	cp_block_default_config(&config, channels);
	status = cp_block_init(&processor, &config);
	if (status != CP_OK)
		return 1;
	status = cp_block_process(&processor, input, output, scratch,
	    frames * channels, frames);
	if (status != CP_OK)
		return 1;
	status = cp_monitor_snapshot_from_processor(&processor, &monitor);
	if (status != CP_OK)
		return 1;

	(void)printf("header=carrierpress_core.h\n");
	(void)printf("status=ok\n");
	(void)printf("channels=%zu\n", config.channels);
	(void)printf("output_peak=%.6f\n",
	    (double)cp_monitor_level_to_sample(monitor.output_peak));

	return 0;
}

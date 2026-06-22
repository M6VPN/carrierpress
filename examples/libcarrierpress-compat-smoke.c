/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/examples/libcarrierpress-compat-smoke.c */

#include <sys/types.h>

#include <stdio.h>

#include "carrierpress.h"

int
main(void)
{
	struct cp_block_config block;
	struct cp_config_file config;

	cp_block_default_config(&block, CP_CHANNELS_MONO);
	cp_config_file_init(&config);
	if (block.channels != CP_CHANNELS_MONO || config.seen_profile != 0)
		return 1;

	(void)printf("header=carrierpress.h\n");
	(void)printf("status=ok\n");
	(void)printf("version=%s\n", CP_VERSION_STRING);

	return 0;
}

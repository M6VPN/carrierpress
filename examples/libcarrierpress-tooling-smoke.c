/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/examples/libcarrierpress-tooling-smoke.c */

#include <sys/types.h>

#include <stdio.h>

#include "carrierpress_tooling.h"

int
main(void)
{
	struct cp_batch_plan batch;
	struct cp_config_file config;
	struct cp_profile profile;
	struct cp_report_metrics metrics;

	cp_batch_plan_init(&batch);
	cp_config_file_init(&config);
	cp_profile_init(&profile);
	cp_report_metrics_init(&metrics);

	if (batch.item_count != 0 || config.seen_profile != 0 ||
	    profile.name[0] != '\0' || metrics.samples != 0)
		return 1;

	(void)printf("header=carrierpress_tooling.h\n");
	(void)printf("status=ok\n");
	(void)printf("schema_version=%d\n", CP_REPORT_SCHEMA_VERSION);

	return 0;
}

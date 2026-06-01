/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_monitor.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_monitor.h"

static int	test_centibel_conversion(void);
static int	test_level_conversion(void);
static int	test_snapshot_clear(void);

int
main(void)
{
	if (!test_level_conversion())
		return 1;
	if (!test_centibel_conversion())
		return 1;
	if (!test_snapshot_clear())
		return 1;

	return 0;
}

static int
test_centibel_conversion(void)
{
	if (cp_monitor_db_to_centibel(1.25f) != 125)
		return 0;
	if (cp_monitor_db_to_centibel(200.0f) != CP_MONITOR_MAX_DB)
		return 0;
	if (cp_monitor_db_to_centibel(-200.0f) != CP_MONITOR_MIN_DB)
		return 0;
	if (cp_monitor_db_to_centibel(NAN) != 0)
		return 0;
	if (fabsf(cp_monitor_centibel_to_db(125) - 1.25f) > 0.001f)
		return 0;

	return 1;
}

static int
test_level_conversion(void)
{
	if (cp_monitor_sample_to_level(0.5f) != 500000u) {
		printf("test_monitor: level conversion failed\n");
		return 0;
	}
	if (cp_monitor_sample_to_level(-1.0f) != 0u)
		return 0;
	if (cp_monitor_sample_to_level(NAN) != 0u)
		return 0;
	if (cp_monitor_sample_to_level(10.0f) != CP_MONITOR_MAX_LEVEL)
		return 0;
	if (fabsf(cp_monitor_level_to_sample(500000u) - 0.5f) > 0.001f)
		return 0;

	return 1;
}

static int
test_snapshot_clear(void)
{
	struct cp_monitor_snapshot snapshot;

	snapshot.input_peak = 1u;
	snapshot.band_count = CP_MONITOR_MAX_BANDS;
	snapshot.band_rms[0] = 2u;
	cp_monitor_snapshot_clear(&snapshot);
	if (snapshot.input_peak != 0u || snapshot.band_count != 0 ||
	    snapshot.band_rms[0] != 0u) {
		printf("test_monitor: snapshot clear failed\n");
		return 0;
	}

	return 1;
}

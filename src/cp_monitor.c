/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_monitor.c */

#include <math.h>
#include <string.h>

#include "cp_monitor.h"

int
cp_monitor_db_to_centibel(cp_sample_t db)
{
	if (!isfinite(db))
		return 0;
	if (db > ((cp_sample_t)CP_MONITOR_MAX_DB / 100.0f))
		return CP_MONITOR_MAX_DB;
	if (db < ((cp_sample_t)CP_MONITOR_MIN_DB / 100.0f))
		return CP_MONITOR_MIN_DB;

	return (int)lrintf(db * 100.0f);
}

cp_sample_t
cp_monitor_centibel_to_db(int centibel)
{
	if (centibel > CP_MONITOR_MAX_DB)
		centibel = CP_MONITOR_MAX_DB;
	if (centibel < CP_MONITOR_MIN_DB)
		centibel = CP_MONITOR_MIN_DB;

	return (cp_sample_t)centibel / 100.0f;
}

cp_sample_t
cp_monitor_level_to_sample(unsigned int value)
{
	if (value > CP_MONITOR_MAX_LEVEL)
		value = CP_MONITOR_MAX_LEVEL;

	return (cp_sample_t)value / (cp_sample_t)CP_MONITOR_SCALE;
}

unsigned int
cp_monitor_sample_to_level(cp_sample_t value)
{
	if (!isfinite(value) || value <= 0.0f)
		return 0u;
	if (value >= ((cp_sample_t)CP_MONITOR_MAX_LEVEL /
	    (cp_sample_t)CP_MONITOR_SCALE))
		return CP_MONITOR_MAX_LEVEL;

	return (unsigned int)lrintf(value * (cp_sample_t)CP_MONITOR_SCALE);
}

void
cp_monitor_snapshot_clear(struct cp_monitor_snapshot *snapshot)
{
	if (snapshot == NULL)
		return;

	(void)memset(snapshot, 0, sizeof(*snapshot));
}

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_waveform.h */

#ifndef CP_WAVEFORM_H
#define CP_WAVEFORM_H

#include <sys/types.h>

#include "cp_types.h"

#define CP_WAVEFORM_POINTS	256
#define CP_WAVEFORM_SCALE	10000
#define CP_WAVEFORM_MIN_VALUE	(-CP_WAVEFORM_SCALE)
#define CP_WAVEFORM_MAX_VALUE	CP_WAVEFORM_SCALE

struct cp_waveform_snapshot {
	int values[CP_WAVEFORM_POINTS];
	size_t point_count;
	size_t channel_count;
	int valid;
};

int		cp_waveform_capture(struct cp_waveform_snapshot *,
		    const cp_sample_t *, size_t, size_t);
void		cp_waveform_clear(struct cp_waveform_snapshot *);
cp_sample_t	cp_waveform_value_to_sample(int);

#endif

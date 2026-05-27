/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_meter.h */

#ifndef CP_METER_H
#define CP_METER_H

#include "cp_types.h"

struct cp_meter {
	size_t channels;
	cp_sample_t peak[CP_MAX_CHANNELS];
	cp_sample_t rms[CP_MAX_CHANNELS];
};

int	cp_meter_init(struct cp_meter *, size_t);
int	cp_meter_process(struct cp_meter *, const cp_sample_t *, size_t);
int	cp_meter_reset(struct cp_meter *);

#endif

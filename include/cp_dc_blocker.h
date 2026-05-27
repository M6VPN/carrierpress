/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_dc_blocker.h */

#ifndef CP_DC_BLOCKER_H
#define CP_DC_BLOCKER_H

#include "cp_types.h"

struct cp_dc_blocker_channel {
	cp_sample_t x_prev;
	cp_sample_t y_prev;
};

struct cp_dc_blocker {
	cp_sample_t coefficient;
	size_t channels;
	struct cp_dc_blocker_channel channel[CP_MAX_CHANNELS];
};

int	cp_dc_blocker_init(struct cp_dc_blocker *, size_t, cp_sample_t);
int	cp_dc_blocker_process(struct cp_dc_blocker *, const cp_sample_t *,
	    cp_sample_t *, size_t);
int	cp_dc_blocker_reset(struct cp_dc_blocker *);

#endif

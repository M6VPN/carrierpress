/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_limiter.h */

#ifndef CP_LIMITER_H
#define CP_LIMITER_H

#include "cp_types.h"

struct cp_limiter {
	size_t channels;
	cp_sample_t ceiling;
};

int	cp_limiter_init(struct cp_limiter *, size_t, cp_sample_t);
int	cp_limiter_process(struct cp_limiter *, const cp_sample_t *,
	    cp_sample_t *, size_t);
int	cp_limiter_reset(struct cp_limiter *);

#endif

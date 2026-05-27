/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_agc.h */

#ifndef CP_AGC_H
#define CP_AGC_H

#include "cp_types.h"

struct cp_agc {
	size_t channels;
	cp_sample_t target_rms;
	cp_sample_t max_gain;
	cp_sample_t attack_coeff;
	cp_sample_t release_coeff;
	cp_sample_t smooth_coeff;
	cp_sample_t envelope_rms;
	cp_sample_t gain;
};

int	cp_agc_init(struct cp_agc *, size_t, cp_sample_t, cp_sample_t,
	    cp_sample_t, cp_sample_t, cp_sample_t);
int	cp_agc_process(struct cp_agc *, const cp_sample_t *, cp_sample_t *,
	    size_t);
int	cp_agc_reset(struct cp_agc *);

#endif

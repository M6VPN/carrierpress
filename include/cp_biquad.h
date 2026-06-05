/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_biquad.h */

#ifndef CP_BIQUAD_H
#define CP_BIQUAD_H

#include "cp_types.h"

struct cp_biquad_coeff {
	cp_sample_t b0;
	cp_sample_t b1;
	cp_sample_t b2;
	cp_sample_t a1;
	cp_sample_t a2;
};

struct cp_biquad_state {
	cp_sample_t z1;
	cp_sample_t z2;
};

int		cp_biquad_notch(struct cp_biquad_coeff *, cp_sample_t,
		    cp_sample_t, cp_sample_t);
int		cp_biquad_lowpass(struct cp_biquad_coeff *, cp_sample_t,
		    cp_sample_t, cp_sample_t);
int		cp_biquad_highpass(struct cp_biquad_coeff *, cp_sample_t,
		    cp_sample_t, cp_sample_t);
int		cp_biquad_allpass(struct cp_biquad_coeff *, cp_sample_t,
		    cp_sample_t, cp_sample_t);
int		cp_biquad_low_shelf(struct cp_biquad_coeff *, cp_sample_t,
		    cp_sample_t, cp_sample_t);
int		cp_biquad_high_shelf(struct cp_biquad_coeff *, cp_sample_t,
		    cp_sample_t, cp_sample_t);
cp_sample_t	cp_biquad_process_sample(const struct cp_biquad_coeff *,
		    struct cp_biquad_state *, cp_sample_t);
int		cp_biquad_reset(struct cp_biquad_state *);

#endif

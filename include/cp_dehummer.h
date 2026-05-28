/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_dehummer.h */

#ifndef CP_DEHUMMER_H
#define CP_DEHUMMER_H

#include "cp_biquad.h"
#include "cp_types.h"

#define CP_DEHUMMER_DEFAULT_ENABLED		0
#define CP_DEHUMMER_DEFAULT_BASE_HZ		(50.0f)
#define CP_DEHUMMER_DEFAULT_HARMONICS		4
#define CP_DEHUMMER_DEFAULT_Q			(35.0f)
#define CP_DEHUMMER_DEFAULT_SAMPLE_RATE		(48000.0f)
#define CP_DEHUMMER_MAX_HARMONICS		16
#define CP_DEHUMMER_MIN_BASE_HZ			(40.0f)
#define CP_DEHUMMER_MAX_BASE_HZ			(70.0f)
#define CP_DEHUMMER_MIN_Q			(5.0f)
#define CP_DEHUMMER_MAX_Q			(200.0f)
#define CP_DEHUMMER_MIN_SAMPLE_RATE		(8000.0f)
#define CP_DEHUMMER_MAX_SAMPLE_RATE		(384000.0f)

struct cp_dehummer_config {
	cp_sample_t sample_rate;
	cp_sample_t base_frequency;
	size_t harmonic_count;
	cp_sample_t q_factor;
	int enabled;
	size_t channel_count;
};

struct cp_dehummer {
	struct cp_dehummer_config config;
	size_t channels;
	size_t harmonic_count;
	int enabled;
	struct cp_biquad_coeff coeff[CP_DEHUMMER_MAX_HARMONICS];
	struct cp_biquad_state state[CP_DEHUMMER_MAX_HARMONICS][CP_MAX_CHANNELS];
};

void	cp_dehummer_default_config(struct cp_dehummer_config *);
int	cp_dehummer_init(struct cp_dehummer *,
	    const struct cp_dehummer_config *);
int	cp_dehummer_process(struct cp_dehummer *, const cp_sample_t *,
	    cp_sample_t *, size_t);
int	cp_dehummer_reset(struct cp_dehummer *);

#endif

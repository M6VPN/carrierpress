/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_crossover.h */

#ifndef CP_CROSSOVER_H
#define CP_CROSSOVER_H

#include "cp_biquad.h"
#include "cp_types.h"

#define CP_CROSSOVER_MIN_BANDS		2
#define CP_CROSSOVER_MAX_BANDS		9
#define CP_CROSSOVER_M5_MAX_BANDS	4
#define CP_CROSSOVER_MAX_POINTS		(CP_CROSSOVER_MAX_BANDS - 1)
#define CP_CROSSOVER_SECTIONS		2
#define CP_CROSSOVER_DEFAULT_Q		(0.70710678f)
#define CP_CROSSOVER_DEFAULT_RATE	(48000.0f)

struct cp_crossover_config {
	cp_sample_t sample_rate;
	size_t channels;
	size_t band_count;
	cp_sample_t frequency[CP_CROSSOVER_MAX_POINTS];
};

struct cp_crossover {
	struct cp_crossover_config config;
	size_t channels;
	size_t band_count;
	size_t point_count;
	struct cp_biquad_coeff coeff[CP_CROSSOVER_MAX_POINTS];
	struct cp_biquad_state state[CP_CROSSOVER_MAX_POINTS]
	    [CP_CROSSOVER_SECTIONS][CP_MAX_CHANNELS];
};

void	cp_crossover_default_config(struct cp_crossover_config *, size_t,
	    size_t);
int	cp_crossover_init(struct cp_crossover *,
	    const struct cp_crossover_config *);
int	cp_crossover_process(struct cp_crossover *, const cp_sample_t *,
	    cp_sample_t **, size_t);
int	cp_crossover_process_sample(struct cp_crossover *, cp_sample_t,
	    size_t, cp_sample_t *);
int	cp_crossover_reset(struct cp_crossover *);

#endif

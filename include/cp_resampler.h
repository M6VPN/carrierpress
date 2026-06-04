/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_resampler.h */

#ifndef CP_RESAMPLER_H
#define CP_RESAMPLER_H

#include "cp_types.h"

#define CP_RESAMPLER_DEFAULT_RATE	(48000.0)
#define CP_RESAMPLER_MIN_RATE		(8000.0)
#define CP_RESAMPLER_MAX_RATE		(192000.0)

struct cp_resampler_config {
	double input_rate;
	double output_rate;
	size_t channel_count;
	int enabled;
};

struct cp_resampler {
	struct cp_resampler_config config;
	double step;
	double position;
	cp_sample_t previous[CP_MAX_CHANNELS];
	size_t channels;
	int enabled;
	int has_previous;
};

void	cp_resampler_default_config(struct cp_resampler_config *);
int	cp_resampler_init(struct cp_resampler *,
	    const struct cp_resampler_config *);
size_t	cp_resampler_output_capacity(size_t, double, double);
int	cp_resampler_process(struct cp_resampler *, const cp_sample_t *,
	    size_t, cp_sample_t *, size_t, size_t *);
int	cp_resampler_reset(struct cp_resampler *);

#endif

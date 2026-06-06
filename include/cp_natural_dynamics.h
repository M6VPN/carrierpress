/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_natural_dynamics.h */

#ifndef CP_NATURAL_DYNAMICS_H
#define CP_NATURAL_DYNAMICS_H

#include "cp_types.h"

#define CP_ND_DEFAULT_ENABLED		0
#define CP_ND_DEFAULT_SAMPLE_RATE	48000.0f
#define CP_ND_DEFAULT_CHANNELS		CP_CHANNELS_STEREO
#define CP_ND_DEFAULT_THRESHOLD_DB	(-12.0f)
#define CP_ND_DEFAULT_RATIO		1.35f
#define CP_ND_DEFAULT_ATTACK_MS		35.0f
#define CP_ND_DEFAULT_RELEASE_MS	450.0f
#define CP_ND_DEFAULT_MAX_REDUCTION_DB	4.0f
#define CP_ND_MIN_SAMPLE_RATE		8000.0f
#define CP_ND_MAX_SAMPLE_RATE		192000.0f
#define CP_ND_MIN_THRESHOLD_DB		(-60.0f)
#define CP_ND_MAX_THRESHOLD_DB		0.0f
#define CP_ND_MIN_RATIO			1.0f
#define CP_ND_MAX_RATIO			4.0f
#define CP_ND_MIN_TIME_MS		0.0f
#define CP_ND_MAX_TIME_MS		5000.0f
#define CP_ND_MIN_REDUCTION_DB		0.0f
#define CP_ND_MAX_REDUCTION_DB		24.0f

struct cp_natural_dynamics_config {
	int enabled;
	cp_sample_t sample_rate;
	size_t channel_count;
	cp_sample_t threshold_db;
	cp_sample_t ratio;
	cp_sample_t attack_ms;
	cp_sample_t release_ms;
	cp_sample_t max_gain_reduction_db;
};

struct cp_natural_dynamics {
	struct cp_natural_dynamics_config config;
	cp_sample_t last_rms;
	cp_sample_t gain;
	cp_sample_t gain_db;
	cp_sample_t gain_reduction_db;
};

void	cp_natural_dynamics_default_config(
	    struct cp_natural_dynamics_config *);
int	cp_natural_dynamics_init(struct cp_natural_dynamics *,
	    const struct cp_natural_dynamics_config *);
int	cp_natural_dynamics_process(struct cp_natural_dynamics *,
	    const cp_sample_t *, cp_sample_t *, size_t);
int	cp_natural_dynamics_reset(struct cp_natural_dynamics *);

#endif

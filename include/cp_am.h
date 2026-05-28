/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_am.h */

#ifndef CP_AM_H
#define CP_AM_H

#include "cp_biquad.h"
#include "cp_types.h"

#define CP_AM_DEFAULT_ENABLED		0
#define CP_AM_DEFAULT_RATE		(48000.0f)
#define CP_AM_DEFAULT_HIGHPASS_HZ	(60.0f)
#define CP_AM_DEFAULT_LOWPASS_HZ	(5000.0f)
#define CP_AM_DEFAULT_POS_PEAK		(0.95f)
#define CP_AM_DEFAULT_NEG_PEAK		(0.95f)
#define CP_AM_DEFAULT_ASYM_RATIO	(1.0f)
#define CP_AM_DEFAULT_PHASE_STAGES	2
#define CP_AM_MAX_PHASE_STAGES		4
#define CP_AM_FILTER_SECTIONS		2
#define CP_AM_PRESET_NAME_SIZE		16
#define CP_AM_MIN_HIGHPASS_HZ		(10.0f)
#define CP_AM_MAX_HIGHPASS_HZ		(300.0f)
#define CP_AM_MIN_LOWPASS_HZ		(1000.0f)
#define CP_AM_MAX_LOWPASS_HZ		(12000.0f)
#define CP_AM_MIN_PEAK_LIMIT		(0.05f)
#define CP_AM_MAX_POS_PEAK_LIMIT	(2.0f)
#define CP_AM_MAX_NEG_PEAK_LIMIT	(1.0f)
#define CP_AM_MIN_ASYM_RATIO		(1.0f)
#define CP_AM_MAX_ASYM_RATIO		(2.0f)

struct cp_am_config {
	int enabled;
	cp_sample_t sample_rate;
	size_t channel_count;
	cp_sample_t highpass_hz;
	cp_sample_t lowpass_hz;
	int phase_rotator_enabled;
	size_t phase_rotator_stages;
	cp_sample_t positive_peak_limit;
	cp_sample_t negative_peak_limit;
	int asymmetry_enabled;
	cp_sample_t asymmetry_ratio;
	char preset_name[CP_AM_PRESET_NAME_SIZE];
};

struct cp_am {
	struct cp_am_config config;
	size_t channels;
	size_t phase_stages;
	int enabled;
	struct cp_biquad_coeff highpass_coeff;
	struct cp_biquad_coeff lowpass_coeff;
	struct cp_biquad_coeff phase_coeff[CP_AM_MAX_PHASE_STAGES];
	struct cp_biquad_state highpass_state[CP_AM_FILTER_SECTIONS]
	    [CP_MAX_CHANNELS];
	struct cp_biquad_state lowpass_state[CP_AM_FILTER_SECTIONS]
	    [CP_MAX_CHANNELS];
	struct cp_biquad_state phase_state[CP_AM_MAX_PHASE_STAGES]
	    [CP_MAX_CHANNELS];
};

int		cp_am_apply_preset(struct cp_am_config *, const char *);
void		cp_am_default_config(struct cp_am_config *);
int		cp_am_init(struct cp_am *, const struct cp_am_config *);
int		cp_am_process(struct cp_am *, const cp_sample_t *,
		    cp_sample_t *, size_t);
int		cp_am_reset(struct cp_am *);

#endif

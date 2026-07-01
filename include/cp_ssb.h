/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_ssb.h */

#ifndef CP_SSB_H
#define CP_SSB_H

#include "cp_biquad.h"
#include "cp_types.h"

#define CP_SSB_DEFAULT_ENABLED		0
#define CP_SSB_DEFAULT_RATE		(48000.0f)
#define CP_SSB_DEFAULT_HIGHPASS_HZ	(120.0f)
#define CP_SSB_DEFAULT_LOWPASS_HZ	(2800.0f)
#define CP_SSB_DEFAULT_PEAK		(0.95f)
#define CP_SSB_DEFAULT_PHASE_STAGES	2
#define CP_SSB_MAX_PHASE_STAGES		4
#define CP_SSB_FILTER_SECTIONS		2
#define CP_SSB_PRESET_NAME_SIZE		16
#define CP_SSB_MIN_HIGHPASS_HZ		(20.0f)
#define CP_SSB_MAX_HIGHPASS_HZ		(500.0f)
#define CP_SSB_MIN_LOWPASS_HZ		(1000.0f)
#define CP_SSB_MAX_LOWPASS_HZ		(6000.0f)
#define CP_SSB_MIN_PEAK_LIMIT		(0.05f)
#define CP_SSB_MAX_PEAK_LIMIT		(1.0f)

enum cp_ssb_preset {
	CP_SSB_PRESET_SPEECH = 0,
	CP_SSB_PRESET_NARROW,
	CP_SSB_PRESET_WIDE,
	CP_SSB_PRESET_GENTLE,
	CP_SSB_PRESET_HF_VOICE,
	CP_SSB_PRESET_HF_NARROW,
	CP_SSB_PRESET_VHF_FM
};

struct cp_ssb_config {
	int enabled;
	cp_sample_t sample_rate;
	size_t channel_count;
	cp_sample_t highpass_hz;
	cp_sample_t lowpass_hz;
	int phase_rotator_enabled;
	size_t phase_rotator_stages;
	cp_sample_t peak_limit;
	char preset_name[CP_SSB_PRESET_NAME_SIZE];
};

struct cp_ssb {
	struct cp_ssb_config config;
	size_t channels;
	size_t phase_stages;
	int enabled;
	struct cp_biquad_coeff highpass_coeff;
	struct cp_biquad_coeff lowpass_coeff;
	struct cp_biquad_coeff phase_coeff[CP_SSB_MAX_PHASE_STAGES];
	struct cp_biquad_state highpass_state[CP_SSB_FILTER_SECTIONS]
	    [CP_MAX_CHANNELS];
	struct cp_biquad_state lowpass_state[CP_SSB_FILTER_SECTIONS]
	    [CP_MAX_CHANNELS];
	struct cp_biquad_state phase_state[CP_SSB_MAX_PHASE_STAGES]
	    [CP_MAX_CHANNELS];
};

int		cp_ssb_apply_preset(struct cp_ssb_config *, const char *);
int		cp_ssb_apply_preset_id(struct cp_ssb_config *,
		    enum cp_ssb_preset);
void		cp_ssb_default_config(struct cp_ssb_config *);
int		cp_ssb_init(struct cp_ssb *, const struct cp_ssb_config *);
int		cp_ssb_preset_from_string(const char *,
		    enum cp_ssb_preset *);
int		cp_ssb_process(struct cp_ssb *, const cp_sample_t *,
		    cp_sample_t *, size_t);
const char	*cp_ssb_preset_string(enum cp_ssb_preset);
int		cp_ssb_reset(struct cp_ssb *);

#endif

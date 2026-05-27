/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_agc.h */

#ifndef CP_AGC_H
#define CP_AGC_H

#include "cp_types.h"

enum cp_agc_gate_state {
	CP_AGC_STATE_OPEN   = 0,
	CP_AGC_STATE_HELD   = 1,
	CP_AGC_STATE_SILENT = 2
};

#define CP_AGC_DEFAULT_MIN_GAIN		(0.125f)
#define CP_AGC_DEFAULT_ATTACK_MS	(50.0f)
#define CP_AGC_DEFAULT_RELEASE_MS	(1500.0f)
#define CP_AGC_DEFAULT_FAST_ATTACK_MS	(5.0f)
#define CP_AGC_DEFAULT_HOLD_MS		(200.0f)
#define CP_AGC_DEFAULT_GATE_DB		(-45.0f)
#define CP_AGC_DEFAULT_SILENCE_DB	(-70.0f)
#define CP_AGC_DEFAULT_MAX_STEP_DB	(6.0f)
#define CP_AGC_DEFAULT_SAMPLE_RATE	(48000.0f)

struct cp_agc_config {
	cp_sample_t target_rms;
	cp_sample_t min_gain;
	cp_sample_t max_gain;
	cp_sample_t attack_ms;
	cp_sample_t release_ms;
	cp_sample_t fast_attack_ms;
	cp_sample_t hold_ms;
	cp_sample_t gate_threshold_db;
	cp_sample_t silence_threshold_db;
	cp_sample_t max_gain_step_db;
	cp_sample_t sample_rate;
};

struct cp_agc {
	size_t channels;
	struct cp_agc_config config;
	cp_sample_t target_rms;
	cp_sample_t min_gain;
	cp_sample_t max_gain;
	cp_sample_t attack_coeff;
	cp_sample_t release_coeff;
	cp_sample_t smooth_coeff;
	cp_sample_t attack_ms;
	cp_sample_t release_ms;
	cp_sample_t fast_attack_ms;
	cp_sample_t hold_ms;
	cp_sample_t gate_threshold_db;
	cp_sample_t silence_threshold_db;
	cp_sample_t max_gain_step_db;
	cp_sample_t sample_rate;
	cp_sample_t envelope_rms;
	cp_sample_t last_rms;
	cp_sample_t gain;
	cp_sample_t gain_db;
	enum cp_agc_gate_state gate_state;
	size_t hold_remaining_frames;
};

void	cp_agc_default_config(struct cp_agc_config *);
int	cp_agc_init_config(struct cp_agc *, size_t,
	    const struct cp_agc_config *);
int	cp_agc_init(struct cp_agc *, size_t, cp_sample_t, cp_sample_t,
	    cp_sample_t, cp_sample_t, cp_sample_t);
int	cp_agc_process(struct cp_agc *, const cp_sample_t *, cp_sample_t *,
	    size_t);
int	cp_agc_reset(struct cp_agc *);
const char	*cp_agc_state_string(enum cp_agc_gate_state);

#endif

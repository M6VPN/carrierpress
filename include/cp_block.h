/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_block.h */

#ifndef CP_BLOCK_H
#define CP_BLOCK_H

#include "cp_agc.h"
#include "cp_dc_blocker.h"
#include "cp_dehummer.h"
#include "cp_limiter.h"
#include "cp_meter.h"
#include "cp_types.h"

struct cp_block_config {
	size_t channels;
	cp_sample_t dc_coefficient;
	int dehummer_enabled;
	cp_sample_t hum_base_frequency;
	size_t hum_harmonic_count;
	cp_sample_t hum_q_factor;
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
	cp_sample_t limiter_ceiling;
};

struct cp_block_processor {
	size_t channels;
	struct cp_dc_blocker dc_blocker;
	struct cp_dehummer dehummer;
	struct cp_agc agc;
	struct cp_limiter limiter;
	struct cp_meter input_meter;
	struct cp_meter output_meter;
};

void	cp_block_default_config(struct cp_block_config *, size_t);
int	cp_block_init(struct cp_block_processor *, const struct cp_block_config *);
int	cp_block_process(struct cp_block_processor *, const cp_sample_t *,
	    cp_sample_t *, cp_sample_t *, size_t, size_t);
int	cp_block_reset(struct cp_block_processor *);

#endif

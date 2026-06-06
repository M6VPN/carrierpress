/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_block.h */

#ifndef CP_BLOCK_H
#define CP_BLOCK_H

#include "cp_agc.h"
#include "cp_am.h"
#include "cp_auto_eq.h"
#include "cp_bass_eq.h"
#include "cp_dc_blocker.h"
#include "cp_declipper.h"
#include "cp_dehummer.h"
#include "cp_limiter.h"
#include "cp_low_level_boost.h"
#include "cp_meter.h"
#include "cp_multiband.h"
#include "cp_natural_dynamics.h"
#include "cp_restoration.h"
#include "cp_ssb.h"
#include "cp_types.h"

struct cp_audio_config;

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
	int multiband_enabled;
	size_t multiband_band_count;
	enum cp_multiband_preset multiband_preset;
	int multiband2_enabled;
	size_t multiband2_band_count;
	enum cp_multiband_preset multiband2_preset;
	struct cp_bass_eq_config bass_eq_config;
	struct cp_am_config am_config;
	struct cp_declipper_config declipper_config;
	struct cp_auto_eq_config auto_eq_config;
	struct cp_natural_dynamics_config natural_dynamics_config;
	struct cp_low_level_boost_config low_level_boost_config;
	struct cp_restoration_config restoration_config;
	struct cp_ssb_config ssb_config;
	cp_sample_t limiter_ceiling;
};

struct cp_block_processor {
	size_t channels;
	struct cp_dc_blocker dc_blocker;
	struct cp_dehummer dehummer;
	struct cp_agc agc;
	struct cp_multiband multiband;
	struct cp_bass_eq bass_eq;
	struct cp_multiband multiband2;
	struct cp_am am;
	struct cp_restoration restoration;
	struct cp_declipper declipper;
	struct cp_auto_eq auto_eq;
	struct cp_natural_dynamics natural_dynamics;
	struct cp_low_level_boost low_level_boost;
	struct cp_ssb ssb;
	struct cp_limiter limiter;
	struct cp_meter input_meter;
	struct cp_meter output_meter;
};

void	cp_block_default_config(struct cp_block_config *, size_t);
int	cp_block_config_from_audio(struct cp_block_config *,
	    const struct cp_audio_config *, size_t, cp_sample_t);
int	cp_block_init(struct cp_block_processor *, const struct cp_block_config *);
int	cp_block_process(struct cp_block_processor *, const cp_sample_t *,
	    cp_sample_t *, cp_sample_t *, size_t, size_t);
int	cp_block_reset(struct cp_block_processor *);

#endif

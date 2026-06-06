/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_low_level_boost.h */

#ifndef CP_LOW_LEVEL_BOOST_H
#define CP_LOW_LEVEL_BOOST_H

#include "cp_agc.h"
#include "cp_types.h"

#define CP_LLB_DEFAULT_ENABLED		0
#define CP_LLB_DEFAULT_SAMPLE_RATE	48000.0f
#define CP_LLB_DEFAULT_CHANNELS		CP_CHANNELS_STEREO
#define CP_LLB_DEFAULT_TARGET_RMS	0.12f
#define CP_LLB_DEFAULT_MAX_BOOST_DB	6.0f
#define CP_LLB_DEFAULT_ATTACK_MS	120.0f
#define CP_LLB_DEFAULT_RELEASE_MS	650.0f
#define CP_LLB_DEFAULT_GATE_DB		(-52.0f)
#define CP_LLB_DEFAULT_SILENCE_DB	(-72.0f)
#define CP_LLB_MIN_SAMPLE_RATE		8000.0f
#define CP_LLB_MAX_SAMPLE_RATE		192000.0f
#define CP_LLB_MIN_TARGET_RMS		0.001f
#define CP_LLB_MAX_TARGET_RMS		0.75f
#define CP_LLB_MIN_BOOST_DB		0.0f
#define CP_LLB_MAX_BOOST_DB		18.0f
#define CP_LLB_MIN_TIME_MS		0.0f
#define CP_LLB_MAX_TIME_MS		5000.0f
#define CP_LLB_MIN_GATE_DB		(-90.0f)
#define CP_LLB_MAX_GATE_DB		(-10.0f)

struct cp_low_level_boost_config {
	int enabled;
	cp_sample_t sample_rate;
	size_t channel_count;
	cp_sample_t target_rms;
	cp_sample_t max_boost_db;
	cp_sample_t attack_ms;
	cp_sample_t release_ms;
	cp_sample_t gate_threshold_db;
	cp_sample_t silence_threshold_db;
};

struct cp_low_level_boost {
	struct cp_low_level_boost_config config;
	cp_sample_t last_rms;
	cp_sample_t gain;
	cp_sample_t gain_db;
	enum cp_agc_gate_state gate_state;
};

void	cp_low_level_boost_default_config(
	    struct cp_low_level_boost_config *);
int	cp_low_level_boost_init(struct cp_low_level_boost *,
	    const struct cp_low_level_boost_config *);
int	cp_low_level_boost_process(struct cp_low_level_boost *,
	    const cp_sample_t *, cp_sample_t *, size_t);
int	cp_low_level_boost_reset(struct cp_low_level_boost *);

#endif

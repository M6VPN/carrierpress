/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_declipper.h */

#ifndef CP_DECLIPPER_H
#define CP_DECLIPPER_H

#include "cp_restoration.h"
#include "cp_types.h"

#define CP_DECLIPPER_DEFAULT_ENABLED		0
#define CP_DECLIPPER_DEFAULT_SAMPLE_RATE		48000.0f
#define CP_DECLIPPER_DEFAULT_CHANNELS		CP_CHANNELS_STEREO
#define CP_DECLIPPER_DEFAULT_CLIP_THRESHOLD	0.98f
#define CP_DECLIPPER_DEFAULT_LOW_CEILING		0.45f
#define CP_DECLIPPER_DEFAULT_STRENGTH		0.35f
#define CP_DECLIPPER_DEFAULT_MAX_REPAIR		16u
#define CP_DECLIPPER_DEFAULT_ANALYSIS_REQUIRED	1
#define CP_DECLIPPER_MIN_SAMPLE_RATE		8000.0f
#define CP_DECLIPPER_MAX_SAMPLE_RATE		192000.0f
#define CP_DECLIPPER_MIN_CLIP_THRESHOLD		0.50f
#define CP_DECLIPPER_MAX_CLIP_THRESHOLD		1.0f
#define CP_DECLIPPER_MIN_LOW_CEILING		0.10f
#define CP_DECLIPPER_MAX_LOW_CEILING		0.95f
#define CP_DECLIPPER_MIN_STRENGTH		0.0f
#define CP_DECLIPPER_MAX_STRENGTH		1.0f
#define CP_DECLIPPER_MIN_REPAIR_SAMPLES		2u
#define CP_DECLIPPER_MAX_REPAIR_SAMPLES		64u

enum cp_declipper_mode {
	CP_DECLIPPER_MODE_CONSERVATIVE = 0
};

enum cp_declipper_bypass_reason {
	CP_DECLIPPER_BYPASS_NONE = 0,
	CP_DECLIPPER_BYPASS_DISABLED,
	CP_DECLIPPER_BYPASS_ANALYSIS_REQUIRED,
	CP_DECLIPPER_BYPASS_LOW_CONFIDENCE,
	CP_DECLIPPER_BYPASS_TRANSIENT,
	CP_DECLIPPER_BYPASS_LONG_RUN,
	CP_DECLIPPER_BYPASS_NONFINITE
};

struct cp_declipper_config {
	int enabled;
	cp_sample_t sample_rate;
	size_t channel_count;
	enum cp_declipper_mode mode;
	cp_sample_t clip_threshold;
	cp_sample_t low_ceiling_threshold;
	cp_sample_t repair_strength;
	size_t max_repair_samples;
	int analysis_required;
};

struct cp_declipper_metrics {
	size_t repaired_sample_count;
	size_t repaired_run_count;
	cp_sample_t max_repair_delta;
	enum cp_declipper_bypass_reason bypass_reason;
	int finite;
};

struct cp_declipper_channel {
	cp_sample_t previous_sample;
	cp_sample_t held_sample;
	size_t run_length;
	int active;
	int previous_valid;
};

struct cp_declipper {
	struct cp_declipper_config config;
	struct cp_declipper_metrics metrics;
	struct cp_declipper_channel channel[CP_MAX_CHANNELS];
};

void		cp_declipper_default_config(struct cp_declipper_config *);
const char	*cp_declipper_bypass_reason_string(
		    enum cp_declipper_bypass_reason);
int		cp_declipper_init(struct cp_declipper *,
		    const struct cp_declipper_config *);
int		cp_declipper_process(struct cp_declipper *,
		    const struct cp_restoration_metrics *, const cp_sample_t *,
		    cp_sample_t *, size_t);
int		cp_declipper_reset(struct cp_declipper *);

#endif

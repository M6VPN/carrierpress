/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_restoration.h */

#ifndef CP_RESTORATION_H
#define CP_RESTORATION_H

#include "cp_types.h"

#define CP_RESTORATION_DEFAULT_ENABLED		0
#define CP_RESTORATION_DEFAULT_SAMPLE_RATE	48000.0f
#define CP_RESTORATION_DEFAULT_CHANNELS		CP_CHANNELS_STEREO
#define CP_RESTORATION_DEFAULT_CLIP_THRESHOLD	0.98f
#define CP_RESTORATION_DEFAULT_FLAT_RUN_MIN	3u
#define CP_RESTORATION_DEFAULT_WINDOW_FRAMES	4096u
#define CP_RESTORATION_MIN_SAMPLE_RATE		8000.0f
#define CP_RESTORATION_MAX_SAMPLE_RATE		192000.0f
#define CP_RESTORATION_MIN_CLIP_THRESHOLD	0.50f
#define CP_RESTORATION_MAX_CLIP_THRESHOLD	1.0f
#define CP_RESTORATION_MIN_FLAT_RUN_MIN		2u
#define CP_RESTORATION_MAX_FLAT_RUN_MIN		64u
#define CP_RESTORATION_MIN_WINDOW_FRAMES		64u
#define CP_RESTORATION_MAX_WINDOW_FRAMES		65536u

#define CP_RESTORATION_REASON_HARD_CLIP		(1u << 0)
#define CP_RESTORATION_REASON_LOW_CEILING	(1u << 1)
#define CP_RESTORATION_REASON_TRANSIENT		(1u << 2)
#define CP_RESTORATION_REASON_LOW_HF		(1u << 3)
#define CP_RESTORATION_REASON_NONFINITE		(1u << 4)

enum cp_restoration_source_profile {
	CP_RESTORATION_SOURCE_UNKNOWN = 0,
	CP_RESTORATION_SOURCE_SILENCE,
	CP_RESTORATION_SOURCE_WIDEBAND,
	CP_RESTORATION_SOURCE_AM_LIMITED,
	CP_RESTORATION_SOURCE_SSB_VOICE,
	CP_RESTORATION_SOURCE_HIGH_FREQUENCY_LOSS
};

struct cp_restoration_config {
	int enabled;
	cp_sample_t sample_rate;
	size_t channel_count;
	cp_sample_t clip_threshold;
	size_t flat_run_min;
	size_t analysis_window_frames;
};

struct cp_restoration_metrics {
	cp_sample_t clipped_sample_ratio;
	cp_sample_t high_frequency_ratio;
	cp_sample_t clipping_confidence;
	cp_sample_t lossy_confidence;
	cp_sample_t low_ceiling_clipping_confidence;
	cp_sample_t transient_confidence;
	cp_sample_t flat_run_ratio;
	cp_sample_t peak_repeat_ratio;
	cp_sample_t low_ceiling_flat_run_ratio;
	cp_sample_t low_ceiling_peak_repeat_ratio;
	cp_sample_t observed_peak;
	cp_sample_t crest_factor;
	size_t flat_run_count;
	size_t peak_repeat_count;
	size_t clipped_sample_count;
	size_t total_sample_count;
	size_t low_ceiling_flat_run_count;
	size_t low_ceiling_peak_repeat_count;
	enum cp_restoration_source_profile source_profile;
	unsigned int reason_flags;
	int finite;
};

struct cp_restoration {
	struct cp_restoration_config config;
	struct cp_restoration_metrics metrics;
	cp_sample_t previous_sample[CP_MAX_CHANNELS];
	cp_sample_t previous_abs[CP_MAX_CHANNELS];
	size_t flat_run_length[CP_MAX_CHANNELS];
	size_t low_ceiling_flat_run_length[CP_MAX_CHANNELS];
	size_t window_frames_seen;
	double sample_energy;
	double difference_energy;
	int have_previous[CP_MAX_CHANNELS];
};

void	cp_restoration_default_config(struct cp_restoration_config *);
int	cp_restoration_init(struct cp_restoration *,
	    const struct cp_restoration_config *);
const struct cp_restoration_metrics *cp_restoration_get_metrics(
	    const struct cp_restoration *);
const char *	cp_restoration_source_profile_string(
	    enum cp_restoration_source_profile);
int	cp_restoration_process(struct cp_restoration *, const cp_sample_t *,
	    size_t);
int	cp_restoration_reset(struct cp_restoration *);

#endif

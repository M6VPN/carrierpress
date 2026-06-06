/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_auto_eq.h */

#ifndef CP_AUTO_EQ_H
#define CP_AUTO_EQ_H

#include "cp_biquad.h"
#include "cp_types.h"

#define CP_AUTO_EQ_DEFAULT_ENABLED		0
#define CP_AUTO_EQ_DEFAULT_SAMPLE_RATE		48000.0f
#define CP_AUTO_EQ_DEFAULT_CHANNELS		CP_CHANNELS_STEREO
#define CP_AUTO_EQ_DEFAULT_WINDOW_FRAMES	4096u
#define CP_AUTO_EQ_MIN_SAMPLE_RATE		8000.0f
#define CP_AUTO_EQ_MAX_SAMPLE_RATE		192000.0f
#define CP_AUTO_EQ_MIN_WINDOW_FRAMES		64u
#define CP_AUTO_EQ_MAX_WINDOW_FRAMES		65536u
#define CP_AUTO_EQ_BAND_COUNT			5u
#define CP_AUTO_EQ_DB_FLOOR			(-96.0f)

enum cp_auto_eq_band {
	CP_AUTO_EQ_BAND_BASS = 0,
	CP_AUTO_EQ_BAND_LOW_MID,
	CP_AUTO_EQ_BAND_MID,
	CP_AUTO_EQ_BAND_PRESENCE,
	CP_AUTO_EQ_BAND_HIGH
};

enum cp_auto_eq_source_hint {
	CP_AUTO_EQ_SOURCE_UNKNOWN = 0,
	CP_AUTO_EQ_SOURCE_SILENCE,
	CP_AUTO_EQ_SOURCE_BASS_HEAVY,
	CP_AUTO_EQ_SOURCE_THIN,
	CP_AUTO_EQ_SOURCE_DARK,
	CP_AUTO_EQ_SOURCE_BRIGHT,
	CP_AUTO_EQ_SOURCE_BALANCED,
	CP_AUTO_EQ_SOURCE_LIMITED_BAND
};

struct cp_auto_eq_config {
	int enabled;
	cp_sample_t sample_rate;
	size_t channel_count;
	size_t analysis_window_frames;
};

struct cp_auto_eq_metrics {
	cp_sample_t total_rms;
	cp_sample_t band_rms[CP_AUTO_EQ_BAND_COUNT];
	cp_sample_t band_relative_db[CP_AUTO_EQ_BAND_COUNT];
	cp_sample_t spectral_tilt_db;
	cp_sample_t low_frequency_weight;
	cp_sample_t presence_weight;
	cp_sample_t high_frequency_weight;
	enum cp_auto_eq_source_hint source_hint;
	int band_enabled[CP_AUTO_EQ_BAND_COUNT];
	int finite;
};

struct cp_auto_eq {
	struct cp_auto_eq_config config;
	struct cp_auto_eq_metrics metrics;
	struct cp_biquad_coeff highpass_coeff[CP_AUTO_EQ_BAND_COUNT];
	struct cp_biquad_coeff lowpass_coeff[CP_AUTO_EQ_BAND_COUNT];
	struct cp_biquad_state highpass_state[CP_AUTO_EQ_BAND_COUNT]
	    [CP_MAX_CHANNELS];
	struct cp_biquad_state lowpass_state[CP_AUTO_EQ_BAND_COUNT]
	    [CP_MAX_CHANNELS];
	cp_sample_t low_cut[CP_AUTO_EQ_BAND_COUNT];
	cp_sample_t high_cut[CP_AUTO_EQ_BAND_COUNT];
	double total_energy;
	double band_energy[CP_AUTO_EQ_BAND_COUNT];
	size_t total_sample_count;
	size_t window_frames_seen;
};

void		cp_auto_eq_default_config(struct cp_auto_eq_config *);
const struct cp_auto_eq_metrics *cp_auto_eq_get_metrics(
		    const struct cp_auto_eq *);
int		cp_auto_eq_init(struct cp_auto_eq *,
		    const struct cp_auto_eq_config *);
const char	*cp_auto_eq_source_hint_string(
		    enum cp_auto_eq_source_hint);
int		cp_auto_eq_process(struct cp_auto_eq *, const cp_sample_t *,
		    size_t);
int		cp_auto_eq_reset(struct cp_auto_eq *);

#endif

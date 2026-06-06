/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_bass_eq.h */

#ifndef CP_BASS_EQ_H
#define CP_BASS_EQ_H

#include <sys/types.h>

#include "cp_auto_eq.h"
#include "cp_biquad.h"
#include "cp_types.h"

#define CP_BASS_EQ_DEFAULT_ENABLED		0
#define CP_BASS_EQ_DEFAULT_RATE			48000.0f
#define CP_BASS_EQ_DEFAULT_CHANNELS		CP_CHANNELS_MONO
#define CP_BASS_EQ_DEFAULT_LOW_HZ		120.0f
#define CP_BASS_EQ_DEFAULT_HIGH_HZ		3000.0f
#define CP_BASS_EQ_DEFAULT_GAIN_DB		0.0f
#define CP_BASS_EQ_DEFAULT_OUTPUT_DB		0.0f
#define CP_BASS_EQ_MIN_RATE			8000.0f
#define CP_BASS_EQ_MAX_RATE			384000.0f
#define CP_BASS_EQ_MIN_LOW_HZ			40.0f
#define CP_BASS_EQ_MAX_LOW_HZ			300.0f
#define CP_BASS_EQ_MIN_HIGH_HZ			1000.0f
#define CP_BASS_EQ_MAX_HIGH_HZ			8000.0f
#define CP_BASS_EQ_MIN_GAIN_DB			(-6.0f)
#define CP_BASS_EQ_MAX_GAIN_DB			6.0f
#define CP_BASS_EQ_MIN_OUTPUT_DB		(-12.0f)
#define CP_BASS_EQ_MAX_OUTPUT_DB		6.0f
#define CP_BASS_EQ_RECOMMEND_CONFIDENCE_MIN	0.0f
#define CP_BASS_EQ_RECOMMEND_CONFIDENCE_MAX	1.0f

enum cp_bass_eq_preset {
	CP_BASS_EQ_PRESET_FLAT = 0,
	CP_BASS_EQ_PRESET_SPEECH,
	CP_BASS_EQ_PRESET_MUSIC,
	CP_BASS_EQ_PRESET_WARM
};

struct cp_bass_eq_recommendation {
	int valid;
	enum cp_bass_eq_preset preset;
	cp_sample_t low_gain_db;
	cp_sample_t high_gain_db;
	cp_sample_t output_gain_db;
	cp_sample_t confidence;
	enum cp_auto_eq_source_hint source_hint;
};

struct cp_bass_eq_config {
	int enabled;
	cp_sample_t sample_rate;
	size_t channel_count;
	enum cp_bass_eq_preset preset;
	cp_sample_t low_shelf_hz;
	cp_sample_t low_gain_db;
	cp_sample_t high_shelf_hz;
	cp_sample_t high_gain_db;
	cp_sample_t output_gain_db;
};

struct cp_bass_eq {
	struct cp_bass_eq_config config;
	struct cp_biquad_coeff low_coeff;
	struct cp_biquad_coeff high_coeff;
	struct cp_biquad_state low_state[CP_MAX_CHANNELS];
	struct cp_biquad_state high_state[CP_MAX_CHANNELS];
	cp_sample_t output_gain;
	size_t channels;
	int enabled;
};

int		cp_bass_eq_apply_preset(struct cp_bass_eq_config *,
		    const char *);
int		cp_bass_eq_apply_preset_id(struct cp_bass_eq_config *,
		    enum cp_bass_eq_preset);
void		cp_bass_eq_default_config(struct cp_bass_eq_config *);
int		cp_bass_eq_init(struct cp_bass_eq *,
		    const struct cp_bass_eq_config *);
int		cp_bass_eq_preset_from_string(const char *,
		    enum cp_bass_eq_preset *);
int		cp_bass_eq_recommend(
		    const struct cp_auto_eq_metrics *,
		    struct cp_bass_eq_recommendation *);
const char	*cp_bass_eq_preset_string(enum cp_bass_eq_preset);
int		cp_bass_eq_process(struct cp_bass_eq *, const cp_sample_t *,
		    cp_sample_t *, size_t);
int		cp_bass_eq_reset(struct cp_bass_eq *);

#endif

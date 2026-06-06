/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_professional_check.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_block.h"
#include "cp_declipper.h"

#define PC_BLOCK_FRAMES		256
#define PC_TOTAL_FRAMES		16384
#define PC_RATE			(48000.0f)
#define PC_PI			(3.14159265358979323846f)
#define PC_LIMIT_EPSILON	(0.002f)
#define PC_SILENCE_LIMIT	(0.0001f)

enum pc_fixture {
	PC_FIXTURE_SILENCE = 0,
	PC_FIXTURE_SPEECH_STEPS,
	PC_FIXTURE_MUSIC_MIX,
	PC_FIXTURE_CLIPPED,
	PC_FIXTURE_LOW_CEILING_CLIPPED,
	PC_FIXTURE_BURST,
	PC_FIXTURE_DC_OFFSET,
	PC_FIXTURE_HUM_50,
	PC_FIXTURE_HUM_60,
	PC_FIXTURE_LOW_TONE,
	PC_FIXTURE_HIGH_TONE,
	PC_FIXTURE_AM_LIMITED,
	PC_FIXTURE_SSB_LIMITED,
	PC_FIXTURE_STEREO_IMBALANCE
};

enum pc_profile {
	PC_PROFILE_DEFAULT = 0,
	PC_PROFILE_DEHUM_50,
	PC_PROFILE_DEHUM_60,
	PC_PROFILE_DECLIPPER,
	PC_PROFILE_DYNAMICS,
	PC_PROFILE_AUTO_EQ,
	PC_PROFILE_MB_BASS,
	PC_PROFILE_MB2,
	PC_PROFILE_AM_SAFE,
	PC_PROFILE_AM_SHORTWAVE,
	PC_PROFILE_AM_WIDE,
	PC_PROFILE_AM_VOICE,
	PC_PROFILE_SSB_SPEECH,
	PC_PROFILE_SSB_NARROW,
	PC_PROFILE_SSB_WIDE
};

enum pc_check {
	PC_CHECK_GENERAL = 0,
	PC_CHECK_SILENCE,
	PC_CHECK_SPEECH_RMS,
	PC_CHECK_MUSIC_RMS,
	PC_CHECK_DC,
	PC_CHECK_DEHUM_50,
	PC_CHECK_DEHUM_60,
	PC_CHECK_DECLIPPER_CLIPPED,
	PC_CHECK_DECLIPPER_LOW_CEILING,
	PC_CHECK_DECLIPPER_BURST,
	PC_CHECK_DYNAMICS_SILENCE,
	PC_CHECK_DYNAMICS_SPEECH,
	PC_CHECK_AUTO_EQ_MUSIC,
	PC_CHECK_AUTO_EQ_LIMITED,
	PC_CHECK_AM_LOWPASS,
	PC_CHECK_AM_HIGHPASS,
	PC_CHECK_SSB_LOWPASS,
	PC_CHECK_SSB_HIGHPASS,
	PC_CHECK_STEREO,
	PC_CHECK_AM_LIMITED_SOURCE,
	PC_CHECK_SSB_LIMITED_SOURCE
};

struct pc_case {
	enum pc_profile profile;
	enum pc_fixture fixture;
	enum pc_check check;
};

struct pc_metrics {
	double input_square;
	double output_square;
	double input_sum;
	double output_sum;
	double output_left_square;
	double output_right_square;
	double input_hum50_sin;
	double input_hum50_cos;
	double input_hum60_sin;
	double input_hum60_cos;
	double output_hum50_sin;
	double output_hum50_cos;
	double output_hum60_sin;
	double output_hum60_cos;
	cp_sample_t input_peak;
	cp_sample_t output_peak;
	cp_sample_t output_min;
	cp_sample_t output_max;
	cp_sample_t final_gain;
	cp_sample_t analysis_clip_confidence;
	cp_sample_t analysis_low_ceiling_confidence;
	cp_sample_t analysis_transient_confidence;
	cp_sample_t declipper_max_delta;
	cp_sample_t natural_dynamics_gr_db;
	cp_sample_t low_level_boost_gain_db;
	cp_sample_t auto_eq_total_rms;
	cp_sample_t auto_eq_spectral_tilt_db;
	cp_sample_t auto_eq_low_weight;
	cp_sample_t auto_eq_presence_weight;
	cp_sample_t auto_eq_high_weight;
	enum cp_agc_gate_state final_gate_state;
	enum cp_auto_eq_source_hint auto_eq_source_hint;
	enum cp_restoration_source_profile analysis_source_profile;
	unsigned int analysis_reason_flags;
	size_t declipper_repaired_samples;
	size_t declipper_repaired_runs;
	int declipper_bypass_reason;
	int finite;
};

static cp_sample_t	pc_abs(cp_sample_t);
static int		pc_check_case(const struct pc_case *,
			    const struct pc_metrics *,
			    const struct cp_block_config *);
static void		pc_config(struct cp_block_config *, enum pc_profile);
static cp_sample_t	pc_crest(const struct pc_metrics *, int);
static int		pc_fail(const struct pc_case *,
			    const struct pc_metrics *, const char *);
static int		pc_generate(enum pc_fixture, cp_sample_t *, size_t,
			    size_t);
static cp_sample_t	pc_hum_amp(const struct pc_metrics *, int, int);
static const char	*pc_fixture_name(enum pc_fixture);
static cp_sample_t	pc_noise(size_t, size_t);
static int		pc_process(const struct pc_case *);
static const char	*pc_profile_name(enum pc_profile);
static void		pc_update(struct pc_metrics *, const cp_sample_t *,
			    const cp_sample_t *, size_t, size_t);

int
main(void)
{
	static const struct pc_case cases[] = {
		{ PC_PROFILE_DEFAULT, PC_FIXTURE_SILENCE,
		    PC_CHECK_SILENCE },
		{ PC_PROFILE_DEFAULT, PC_FIXTURE_SPEECH_STEPS,
		    PC_CHECK_SPEECH_RMS },
		{ PC_PROFILE_DEFAULT, PC_FIXTURE_MUSIC_MIX,
		    PC_CHECK_MUSIC_RMS },
		{ PC_PROFILE_DEFAULT, PC_FIXTURE_CLIPPED,
		    PC_CHECK_GENERAL },
		{ PC_PROFILE_DEFAULT, PC_FIXTURE_LOW_CEILING_CLIPPED,
		    PC_CHECK_GENERAL },
		{ PC_PROFILE_DEFAULT, PC_FIXTURE_BURST,
		    PC_CHECK_GENERAL },
		{ PC_PROFILE_DEFAULT, PC_FIXTURE_DC_OFFSET, PC_CHECK_DC },
		{ PC_PROFILE_DEHUM_50, PC_FIXTURE_HUM_50,
		    PC_CHECK_DEHUM_50 },
		{ PC_PROFILE_DEHUM_60, PC_FIXTURE_HUM_60,
		    PC_CHECK_DEHUM_60 },
		{ PC_PROFILE_DECLIPPER, PC_FIXTURE_CLIPPED,
		    PC_CHECK_DECLIPPER_CLIPPED },
		{ PC_PROFILE_DECLIPPER, PC_FIXTURE_LOW_CEILING_CLIPPED,
		    PC_CHECK_DECLIPPER_LOW_CEILING },
		{ PC_PROFILE_DECLIPPER, PC_FIXTURE_BURST,
		    PC_CHECK_DECLIPPER_BURST },
		{ PC_PROFILE_DYNAMICS, PC_FIXTURE_SILENCE,
		    PC_CHECK_DYNAMICS_SILENCE },
		{ PC_PROFILE_DYNAMICS, PC_FIXTURE_SPEECH_STEPS,
		    PC_CHECK_DYNAMICS_SPEECH },
		{ PC_PROFILE_AUTO_EQ, PC_FIXTURE_MUSIC_MIX,
		    PC_CHECK_AUTO_EQ_MUSIC },
		{ PC_PROFILE_AUTO_EQ, PC_FIXTURE_AM_LIMITED,
		    PC_CHECK_AUTO_EQ_LIMITED },
		{ PC_PROFILE_MB_BASS, PC_FIXTURE_MUSIC_MIX,
		    PC_CHECK_MUSIC_RMS },
		{ PC_PROFILE_MB_BASS, PC_FIXTURE_STEREO_IMBALANCE,
		    PC_CHECK_STEREO },
		{ PC_PROFILE_MB2, PC_FIXTURE_MUSIC_MIX,
		    PC_CHECK_MUSIC_RMS },
		{ PC_PROFILE_MB2, PC_FIXTURE_STEREO_IMBALANCE,
		    PC_CHECK_STEREO },
		{ PC_PROFILE_AM_SAFE, PC_FIXTURE_SPEECH_STEPS,
		    PC_CHECK_SPEECH_RMS },
		{ PC_PROFILE_AM_SAFE, PC_FIXTURE_HIGH_TONE,
		    PC_CHECK_AM_LOWPASS },
		{ PC_PROFILE_AM_SAFE, PC_FIXTURE_LOW_TONE,
		    PC_CHECK_AM_HIGHPASS },
		{ PC_PROFILE_AM_SHORTWAVE, PC_FIXTURE_SPEECH_STEPS,
		    PC_CHECK_SPEECH_RMS },
		{ PC_PROFILE_AM_SHORTWAVE, PC_FIXTURE_HIGH_TONE,
		    PC_CHECK_AM_LOWPASS },
		{ PC_PROFILE_AM_WIDE, PC_FIXTURE_SPEECH_STEPS,
		    PC_CHECK_SPEECH_RMS },
		{ PC_PROFILE_AM_WIDE, PC_FIXTURE_HIGH_TONE,
		    PC_CHECK_AM_LOWPASS },
		{ PC_PROFILE_AM_VOICE, PC_FIXTURE_SPEECH_STEPS,
		    PC_CHECK_SPEECH_RMS },
		{ PC_PROFILE_AM_VOICE, PC_FIXTURE_HIGH_TONE,
		    PC_CHECK_AM_LOWPASS },
		{ PC_PROFILE_SSB_SPEECH, PC_FIXTURE_SPEECH_STEPS,
		    PC_CHECK_SPEECH_RMS },
		{ PC_PROFILE_SSB_SPEECH, PC_FIXTURE_HIGH_TONE,
		    PC_CHECK_SSB_LOWPASS },
		{ PC_PROFILE_SSB_SPEECH, PC_FIXTURE_LOW_TONE,
		    PC_CHECK_SSB_HIGHPASS },
		{ PC_PROFILE_SSB_NARROW, PC_FIXTURE_SPEECH_STEPS,
		    PC_CHECK_SPEECH_RMS },
		{ PC_PROFILE_SSB_NARROW, PC_FIXTURE_HIGH_TONE,
		    PC_CHECK_SSB_LOWPASS },
		{ PC_PROFILE_SSB_WIDE, PC_FIXTURE_SPEECH_STEPS,
		    PC_CHECK_SPEECH_RMS },
		{ PC_PROFILE_SSB_WIDE, PC_FIXTURE_HIGH_TONE,
		    PC_CHECK_SSB_LOWPASS },
		{ PC_PROFILE_DEFAULT, PC_FIXTURE_AM_LIMITED,
		    PC_CHECK_AM_LIMITED_SOURCE },
		{ PC_PROFILE_DEFAULT, PC_FIXTURE_SSB_LIMITED,
		    PC_CHECK_SSB_LIMITED_SOURCE }
	};
	size_t i;

	for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		if (!pc_process(&cases[i]))
			return 1;
	}

	printf("professional_check cases=%zu status=pass\n",
	    sizeof(cases) / sizeof(cases[0]));
	return 0;
}

static cp_sample_t
pc_abs(cp_sample_t sample)
{
	return sample < 0.0f ? -sample : sample;
}

static int
pc_check_case(const struct pc_case *test, const struct pc_metrics *metrics,
	const struct cp_block_config *config)
{
	cp_sample_t input_dc;
	cp_sample_t input_hum;
	cp_sample_t output_dc;
	cp_sample_t output_hum;

	if (test == NULL || metrics == NULL || config == NULL)
		return 0;
	if (!metrics->finite)
		return pc_fail(test, metrics, "non-finite");
	if (metrics->output_peak > CP_DEFAULT_CEILING + PC_LIMIT_EPSILON)
		return pc_fail(test, metrics, "limiter");
	if (config->am_config.enabled &&
	    metrics->output_min < -config->am_config.negative_peak_limit -
	    PC_LIMIT_EPSILON)
		return pc_fail(test, metrics, "am-negative-peak");
	if (config->ssb_config.enabled &&
	    metrics->output_peak > config->ssb_config.peak_limit +
	    PC_LIMIT_EPSILON)
		return pc_fail(test, metrics, "ssb-peak");
	if (metrics->final_gain < config->min_gain ||
	    metrics->final_gain > config->max_gain)
		return pc_fail(test, metrics, "agc-gain");

	switch (test->check) {
	case PC_CHECK_GENERAL:
		if (test->fixture == PC_FIXTURE_CLIPPED &&
		    metrics->analysis_clip_confidence < 0.50f)
			return pc_fail(test, metrics, "clip-analysis");
		if (test->fixture == PC_FIXTURE_LOW_CEILING_CLIPPED &&
		    metrics->analysis_low_ceiling_confidence < 0.50f)
			return pc_fail(test, metrics, "low-ceiling-analysis");
		if (test->fixture == PC_FIXTURE_BURST &&
		    metrics->analysis_transient_confidence < 0.50f)
			return pc_fail(test, metrics, "transient-analysis");
		break;
	case PC_CHECK_SILENCE:
		if (metrics->output_peak > PC_SILENCE_LIMIT ||
		    metrics->final_gate_state != CP_AGC_STATE_SILENT ||
		    metrics->final_gain > 1.01f)
			return pc_fail(test, metrics, "silence");
		break;
	case PC_CHECK_SPEECH_RMS:
		if (metrics->output_square < 0.045 ||
		    metrics->output_square > 0.260 ||
		    pc_crest(metrics, 0) > 8.0f)
			return pc_fail(test, metrics, "speech-range");
		break;
	case PC_CHECK_MUSIC_RMS:
		if (metrics->output_square < 0.050 ||
		    metrics->output_square > 0.180 ||
		    pc_crest(metrics, 0) > 6.0f)
			return pc_fail(test, metrics, "music-range");
		break;
	case PC_CHECK_DC:
		input_dc = pc_abs((cp_sample_t)metrics->input_sum);
		output_dc = pc_abs((cp_sample_t)metrics->output_sum);
		if (output_dc >= input_dc * 0.25f)
			return pc_fail(test, metrics, "dc-reduction");
		break;
	case PC_CHECK_DEHUM_50:
		input_hum = pc_hum_amp(metrics, 50, 1);
		output_hum = pc_hum_amp(metrics, 50, 0);
		if (output_hum >= input_hum * 0.70f)
			return pc_fail(test, metrics, "dehum-50");
		break;
	case PC_CHECK_DEHUM_60:
		input_hum = pc_hum_amp(metrics, 60, 1);
		output_hum = pc_hum_amp(metrics, 60, 0);
		if (output_hum >= input_hum * 0.70f)
			return pc_fail(test, metrics, "dehum-60");
		break;
	case PC_CHECK_DECLIPPER_CLIPPED:
		if (metrics->declipper_repaired_samples == 0 ||
		    metrics->declipper_repaired_runs == 0 ||
		    metrics->analysis_clip_confidence < 0.50f)
			return pc_fail(test, metrics, "declipper-clipped");
		break;
	case PC_CHECK_DECLIPPER_LOW_CEILING:
		if (metrics->declipper_repaired_samples == 0 ||
		    metrics->declipper_max_delta <= 0.0f ||
		    metrics->analysis_low_ceiling_confidence < 0.50f)
			return pc_fail(test, metrics, "declipper-low-ceiling");
		break;
	case PC_CHECK_DECLIPPER_BURST:
		if (metrics->declipper_repaired_samples != 0 ||
		    metrics->analysis_transient_confidence < 0.50f ||
		    (metrics->declipper_bypass_reason !=
		    CP_DECLIPPER_BYPASS_TRANSIENT &&
		    metrics->declipper_bypass_reason !=
		    CP_DECLIPPER_BYPASS_LOW_CONFIDENCE))
			return pc_fail(test, metrics, "declipper-burst");
		break;
	case PC_CHECK_DYNAMICS_SILENCE:
		if (metrics->output_peak > PC_SILENCE_LIMIT ||
		    metrics->low_level_boost_gain_db > 0.10f)
			return pc_fail(test, metrics, "dynamics-silence");
		break;
	case PC_CHECK_DYNAMICS_SPEECH:
		if (metrics->natural_dynamics_gr_db < 0.10f &&
		    metrics->low_level_boost_gain_db < 0.10f)
			return pc_fail(test, metrics, "dynamics-speech");
		break;
	case PC_CHECK_AUTO_EQ_MUSIC:
		if (metrics->auto_eq_total_rms <= 0.0f ||
		    !isfinite(metrics->auto_eq_spectral_tilt_db) ||
		    metrics->auto_eq_source_hint ==
		    CP_AUTO_EQ_SOURCE_SILENCE)
			return pc_fail(test, metrics, "auto-eq-music");
		break;
	case PC_CHECK_AUTO_EQ_LIMITED:
		if (metrics->auto_eq_total_rms <= 0.0f ||
		    metrics->auto_eq_source_hint == CP_AUTO_EQ_SOURCE_BRIGHT ||
		    metrics->auto_eq_source_hint == CP_AUTO_EQ_SOURCE_SILENCE)
			return pc_fail(test, metrics, "auto-eq-limited");
		break;
	case PC_CHECK_AM_LOWPASS:
		if (test->profile == PC_PROFILE_AM_WIDE) {
			if (metrics->output_square >= metrics->input_square *
			    0.45)
				return pc_fail(test, metrics, "am-lowpass");
			break;
		}
		if (metrics->output_square >= metrics->input_square * 0.35)
			return pc_fail(test, metrics, "am-lowpass");
		break;
	case PC_CHECK_AM_HIGHPASS:
		if (metrics->output_square >= metrics->input_square * 0.80)
			return pc_fail(test, metrics, "am-highpass");
		break;
	case PC_CHECK_SSB_LOWPASS:
		if (metrics->output_square >= metrics->input_square * 0.30)
			return pc_fail(test, metrics, "ssb-lowpass");
		break;
	case PC_CHECK_SSB_HIGHPASS:
		if (metrics->output_square >= metrics->input_square * 0.70)
			return pc_fail(test, metrics, "ssb-highpass");
		break;
	case PC_CHECK_STEREO:
		if (metrics->output_left_square <=
		    metrics->output_right_square * 2.0)
			return pc_fail(test, metrics, "stereo");
		break;
	case PC_CHECK_AM_LIMITED_SOURCE:
		if (metrics->analysis_source_profile !=
		    CP_RESTORATION_SOURCE_AM_LIMITED)
			return pc_fail(test, metrics, "am-source");
		break;
	case PC_CHECK_SSB_LIMITED_SOURCE:
		if (metrics->analysis_source_profile !=
		    CP_RESTORATION_SOURCE_SSB_VOICE)
			return pc_fail(test, metrics, "ssb-source");
		break;
	default:
		return pc_fail(test, metrics, "unknown-check");
	}

	return 1;
}

static void
pc_config(struct cp_block_config *config, enum pc_profile profile)
{
	cp_block_default_config(config, CP_CHANNELS_STEREO);
	config->sample_rate = PC_RATE;
	config->limiter_ceiling = CP_DEFAULT_CEILING;
	config->restoration_config.enabled = 1;
	config->restoration_config.analysis_window_frames = PC_TOTAL_FRAMES;

	switch (profile) {
	case PC_PROFILE_DEFAULT:
		break;
	case PC_PROFILE_DEHUM_50:
		config->dehummer_enabled = 1;
		config->hum_base_frequency = 50.0f;
		config->hum_harmonic_count = 4;
		break;
	case PC_PROFILE_DEHUM_60:
		config->dehummer_enabled = 1;
		config->hum_base_frequency = 60.0f;
		config->hum_harmonic_count = 4;
		break;
	case PC_PROFILE_DECLIPPER:
		config->declipper_config.enabled = 1;
		config->declipper_config.repair_strength = 0.35f;
		config->declipper_config.max_repair_samples = 16u;
		break;
	case PC_PROFILE_DYNAMICS:
		config->natural_dynamics_config.enabled = 1;
		config->natural_dynamics_config.threshold_db = -18.0f;
		config->natural_dynamics_config.ratio = 1.5f;
		config->natural_dynamics_config.max_gain_reduction_db = 4.0f;
		config->low_level_boost_config.enabled = 1;
		config->low_level_boost_config.target_rms = 0.12f;
		config->low_level_boost_config.max_boost_db = 5.0f;
		break;
	case PC_PROFILE_AUTO_EQ:
		config->auto_eq_config.enabled = 1;
		config->auto_eq_config.analysis_window_frames =
		    PC_TOTAL_FRAMES;
		break;
	case PC_PROFILE_MB_BASS:
		config->multiband_enabled = 1;
		config->multiband_band_count = 3;
		config->multiband_preset = CP_MULTIBAND_PRESET_MUSIC;
		config->bass_eq_config.enabled = 1;
		(void)cp_bass_eq_apply_preset(&config->bass_eq_config,
		    "music");
		break;
	case PC_PROFILE_MB2:
		config->multiband_enabled = 1;
		config->multiband_band_count = 3;
		config->multiband_preset = CP_MULTIBAND_PRESET_MUSIC;
		config->bass_eq_config.enabled = 1;
		(void)cp_bass_eq_apply_preset(&config->bass_eq_config,
		    "music");
		config->multiband2_enabled = 1;
		config->multiband2_band_count = 3;
		config->multiband2_preset = CP_MULTIBAND_PRESET_MUSIC;
		break;
	case PC_PROFILE_AM_SAFE:
	case PC_PROFILE_AM_SHORTWAVE:
	case PC_PROFILE_AM_WIDE:
	case PC_PROFILE_AM_VOICE:
		config->dehummer_enabled = 1;
		config->hum_base_frequency = 50.0f;
		config->hum_harmonic_count = 4;
		config->multiband_enabled = 1;
		config->multiband_band_count = 3;
		config->multiband_preset = CP_MULTIBAND_PRESET_SPEECH;
		config->bass_eq_config.enabled = 1;
		(void)cp_bass_eq_apply_preset(&config->bass_eq_config,
		    "speech");
		if (profile == PC_PROFILE_AM_SAFE)
			(void)cp_am_apply_preset(&config->am_config,
			    "am-safe");
		else if (profile == PC_PROFILE_AM_WIDE)
			(void)cp_am_apply_preset(&config->am_config,
			    "am-wide");
		else if (profile == PC_PROFILE_AM_VOICE)
			(void)cp_am_apply_preset(&config->am_config,
			    "am-voice");
		else
			(void)cp_am_apply_preset(&config->am_config,
			    "am-shortwave");
		config->am_config.enabled = 1;
		break;
	case PC_PROFILE_SSB_SPEECH:
	case PC_PROFILE_SSB_NARROW:
	case PC_PROFILE_SSB_WIDE:
		config->dehummer_enabled = 1;
		config->hum_base_frequency = 50.0f;
		config->hum_harmonic_count = 4;
		config->multiband_enabled = 1;
		config->multiband_band_count = 3;
		config->multiband_preset = CP_MULTIBAND_PRESET_SPEECH;
		config->bass_eq_config.enabled = 1;
		(void)cp_bass_eq_apply_preset(&config->bass_eq_config,
		    "speech");
		if (profile == PC_PROFILE_SSB_NARROW)
			(void)cp_ssb_apply_preset(&config->ssb_config,
			    "ssb-narrow");
		else if (profile == PC_PROFILE_SSB_WIDE)
			(void)cp_ssb_apply_preset(&config->ssb_config,
			    "ssb-wide");
		else
			(void)cp_ssb_apply_preset(&config->ssb_config,
			    "ssb-speech");
		config->ssb_config.enabled = 1;
		break;
	default:
		break;
	}
}

static cp_sample_t
pc_crest(const struct pc_metrics *metrics, int input)
{
	cp_sample_t peak;
	double rms;

	if (metrics == NULL)
		return 0.0f;

	peak = input ? metrics->input_peak : metrics->output_peak;
	rms = input ? metrics->input_square : metrics->output_square;
	if (rms <= 0.0)
		return 0.0f;

	return peak / (cp_sample_t)rms;
}

static int
pc_fail(const struct pc_case *test, const struct pc_metrics *metrics,
	const char *reason)
{
	printf("professional_check profile=%s fixture=%s status=fail "
	    "reason=%s input_rms=%0.6f output_rms=%0.6f "
	    "output_peak=%0.6f output_min=%0.6f output_crest=%0.6f "
	    "gain=%0.6f gate=%d natural_gr_db=%0.6f "
	    "low_boost_gain_db=%0.6f auto_eq_source=%s "
	    "auto_eq_rms=%0.6f declipper_samples=%zu "
	    "declipper_runs=%zu\n",
	    pc_profile_name(test->profile), pc_fixture_name(test->fixture),
	    reason, metrics->input_square, metrics->output_square,
	    metrics->output_peak, metrics->output_min, pc_crest(metrics, 0),
	    metrics->final_gain, metrics->final_gate_state,
	    metrics->natural_dynamics_gr_db,
	    metrics->low_level_boost_gain_db,
	    cp_auto_eq_source_hint_string(metrics->auto_eq_source_hint),
	    metrics->auto_eq_total_rms,
	    metrics->declipper_repaired_samples,
	    metrics->declipper_repaired_runs);
	return 0;
}

static int
pc_generate(enum pc_fixture fixture, cp_sample_t *buffer, size_t offset,
	size_t frames)
{
	cp_sample_t left;
	cp_sample_t phase;
	cp_sample_t right;
	cp_sample_t value;
	size_t channel;
	size_t frame;
	size_t index;

	if (buffer == NULL)
		return 0;

	for (frame = 0; frame < frames; frame++) {
		index = offset + frame;
		phase = 2.0f * PC_PI * (cp_sample_t)index / PC_RATE;
		switch (fixture) {
		case PC_FIXTURE_SILENCE:
			left = 0.0f;
			right = 0.0f;
			break;
		case PC_FIXTURE_SPEECH_STEPS:
			if (index < PC_TOTAL_FRAMES / 4)
				value = 0.04f;
			else if (index < PC_TOTAL_FRAMES / 2)
				value = 0.18f;
			else if (index < (PC_TOTAL_FRAMES * 3) / 4)
				value = 0.42f;
			else
				value = 0.08f;
			left = value * ((0.70f * sinf(phase * 650.0f)) +
			    (0.20f * sinf(phase * 1300.0f)) +
			    (0.10f * pc_noise(index, 0)));
			right = left;
			break;
		case PC_FIXTURE_MUSIC_MIX:
			left = (0.12f * sinf(phase * 110.0f)) +
			    (0.08f * sinf(phase * 220.0f)) +
			    (0.06f * sinf(phase * 440.0f)) +
			    (0.04f * sinf(phase * 880.0f)) +
			    (0.02f * sinf(phase * 1760.0f));
			right = left;
			break;
		case PC_FIXTURE_CLIPPED:
			value = 1.4f * sinf(phase * 700.0f);
			if (value > 0.98f)
				value = 0.98f;
			if (value < -0.98f)
				value = -0.98f;
			left = value;
			right = value;
			break;
		case PC_FIXTURE_LOW_CEILING_CLIPPED:
			value = 0.90f * sinf(phase * 700.0f);
			if (value > 0.45f)
				value = 0.45f;
			if (value < -0.45f)
				value = -0.45f;
			left = value;
			right = value;
			break;
		case PC_FIXTURE_BURST:
			if ((index % 2048u) < 4u)
				left = 0.99f;
			else if ((index % 1024u) < 40u)
				left = 0.85f * sinf(phase * 1000.0f);
			else
				left = 0.03f * sinf(phase * 1000.0f);
			right = left;
			break;
		case PC_FIXTURE_DC_OFFSET:
			left = 0.22f + (0.04f * sinf(phase * 400.0f));
			right = left;
			break;
		case PC_FIXTURE_HUM_50:
			left = (0.20f * sinf(phase * 50.0f)) +
			    (0.04f * sinf(phase * 1000.0f));
			right = left;
			break;
		case PC_FIXTURE_HUM_60:
			left = (0.20f * sinf(phase * 60.0f)) +
			    (0.04f * sinf(phase * 1000.0f));
			right = left;
			break;
		case PC_FIXTURE_LOW_TONE:
			left = 0.20f * sinf(phase * 40.0f);
			right = left;
			break;
		case PC_FIXTURE_HIGH_TONE:
			left = 0.20f * sinf(phase * 10000.0f);
			right = left;
			break;
		case PC_FIXTURE_AM_LIMITED:
			left = (0.20f * sinf(phase * 2500.0f)) +
			    (0.04f * sinf(phase * 1200.0f));
			right = left;
			break;
		case PC_FIXTURE_SSB_LIMITED:
			left = (0.20f * sinf(phase * 900.0f)) +
			    (0.04f * sinf(phase * 1600.0f));
			right = left;
			break;
		case PC_FIXTURE_STEREO_IMBALANCE:
			left = 0.30f * sinf(phase * 1000.0f);
			right = 0.03f * sinf(phase * 1000.0f);
			break;
		default:
			return 0;
		}

		for (channel = 0; channel < CP_CHANNELS_STEREO; channel++)
			buffer[(frame * CP_CHANNELS_STEREO) + channel] =
			    channel == 0 ? left : right;
	}

	return 1;
}

static cp_sample_t
pc_hum_amp(const struct pc_metrics *metrics, int hum_hz, int input)
{
	double cosine;
	double sine;

	if (metrics == NULL)
		return 0.0f;

	if (hum_hz == 50) {
		sine = input ? metrics->input_hum50_sin :
		    metrics->output_hum50_sin;
		cosine = input ? metrics->input_hum50_cos :
		    metrics->output_hum50_cos;
	} else {
		sine = input ? metrics->input_hum60_sin :
		    metrics->output_hum60_sin;
		cosine = input ? metrics->input_hum60_cos :
		    metrics->output_hum60_cos;
	}

	return (cp_sample_t)(2.0 * sqrt((sine * sine) + (cosine * cosine)) /
	    (double)PC_TOTAL_FRAMES);
}

static const char *
pc_fixture_name(enum pc_fixture fixture)
{
	switch (fixture) {
	case PC_FIXTURE_SILENCE:
		return "silence";
	case PC_FIXTURE_SPEECH_STEPS:
		return "speech-steps";
	case PC_FIXTURE_MUSIC_MIX:
		return "music-mix";
	case PC_FIXTURE_CLIPPED:
		return "clipped";
	case PC_FIXTURE_LOW_CEILING_CLIPPED:
		return "low-ceiling-clipped";
	case PC_FIXTURE_BURST:
		return "burst";
	case PC_FIXTURE_DC_OFFSET:
		return "dc-offset";
	case PC_FIXTURE_HUM_50:
		return "hum-50";
	case PC_FIXTURE_HUM_60:
		return "hum-60";
	case PC_FIXTURE_LOW_TONE:
		return "low-tone";
	case PC_FIXTURE_HIGH_TONE:
		return "high-tone";
	case PC_FIXTURE_AM_LIMITED:
		return "am-limited";
	case PC_FIXTURE_SSB_LIMITED:
		return "ssb-limited";
	case PC_FIXTURE_STEREO_IMBALANCE:
		return "stereo-imbalance";
	default:
		return "unknown";
	}
}

static cp_sample_t
pc_noise(size_t frame, size_t channel)
{
	unsigned int value;

	value = (unsigned int)(frame + (channel * 257u));
	value ^= value >> 16;
	value *= 2246822519u;
	value ^= value >> 13;
	value *= 3266489917u;
	value ^= value >> 16;

	return ((cp_sample_t)(value & 0xffffu) / 32768.0f) - 1.0f;
}

static int
pc_process(const struct pc_case *test)
{
	cp_sample_t input[PC_BLOCK_FRAMES * CP_CHANNELS_STEREO];
	cp_sample_t output[PC_BLOCK_FRAMES * CP_CHANNELS_STEREO];
	cp_sample_t scratch[PC_BLOCK_FRAMES * CP_CHANNELS_STEREO];
	struct cp_block_config config;
	struct cp_block_processor processor;
	struct pc_metrics metrics;
	size_t frames;
	size_t offset;
	int pass;
	int status;

	if (test == NULL)
		return 0;

	pc_config(&config, test->profile);
	status = cp_block_init(&processor, &config);
	if (status != CP_OK) {
		printf("professional_check profile=%s fixture=%s "
		    "status=fail reason=init code=%d\n",
		    pc_profile_name(test->profile),
		    pc_fixture_name(test->fixture), status);
		return 0;
	}

	metrics.input_square = 0.0;
	metrics.output_square = 0.0;
	metrics.input_sum = 0.0;
	metrics.output_sum = 0.0;
	metrics.output_left_square = 0.0;
	metrics.output_right_square = 0.0;
	metrics.input_hum50_sin = 0.0;
	metrics.input_hum50_cos = 0.0;
	metrics.input_hum60_sin = 0.0;
	metrics.input_hum60_cos = 0.0;
	metrics.output_hum50_sin = 0.0;
	metrics.output_hum50_cos = 0.0;
	metrics.output_hum60_sin = 0.0;
	metrics.output_hum60_cos = 0.0;
	metrics.input_peak = 0.0f;
	metrics.output_peak = 0.0f;
	metrics.output_min = 0.0f;
	metrics.output_max = 0.0f;
	metrics.final_gain = 1.0f;
	metrics.analysis_clip_confidence = 0.0f;
	metrics.analysis_low_ceiling_confidence = 0.0f;
	metrics.analysis_transient_confidence = 0.0f;
	metrics.declipper_max_delta = 0.0f;
	metrics.natural_dynamics_gr_db = 0.0f;
	metrics.low_level_boost_gain_db = 0.0f;
	metrics.auto_eq_total_rms = 0.0f;
	metrics.auto_eq_spectral_tilt_db = 0.0f;
	metrics.auto_eq_low_weight = 0.0f;
	metrics.auto_eq_presence_weight = 0.0f;
	metrics.auto_eq_high_weight = 0.0f;
	metrics.final_gate_state = CP_AGC_STATE_OPEN;
	metrics.auto_eq_source_hint = CP_AUTO_EQ_SOURCE_UNKNOWN;
	metrics.analysis_source_profile = CP_RESTORATION_SOURCE_UNKNOWN;
	metrics.analysis_reason_flags = 0u;
	metrics.declipper_repaired_samples = 0;
	metrics.declipper_repaired_runs = 0;
	metrics.declipper_bypass_reason = CP_DECLIPPER_BYPASS_DISABLED;
	metrics.finite = 1;

	for (offset = 0; offset < PC_TOTAL_FRAMES; offset += PC_BLOCK_FRAMES) {
		frames = PC_BLOCK_FRAMES;
		if (offset + frames > PC_TOTAL_FRAMES)
			frames = PC_TOTAL_FRAMES - offset;
		if (!pc_generate(test->fixture, input, offset, frames))
			return 0;
		status = cp_block_process(&processor, input, output, scratch,
		    frames * CP_CHANNELS_STEREO, frames);
		if (status != CP_OK) {
			printf("professional_check profile=%s fixture=%s "
			    "status=fail reason=process code=%d\n",
			    pc_profile_name(test->profile),
			    pc_fixture_name(test->fixture), status);
			return 0;
		}
		pc_update(&metrics, input, output, offset, frames);
	}

	metrics.input_square = sqrt(metrics.input_square /
	    (double)(PC_TOTAL_FRAMES * CP_CHANNELS_STEREO));
	metrics.output_square = sqrt(metrics.output_square /
	    (double)(PC_TOTAL_FRAMES * CP_CHANNELS_STEREO));
	metrics.input_sum = metrics.input_sum /
	    (double)(PC_TOTAL_FRAMES * CP_CHANNELS_STEREO);
	metrics.output_sum = metrics.output_sum /
	    (double)(PC_TOTAL_FRAMES * CP_CHANNELS_STEREO);
	metrics.output_left_square = sqrt(metrics.output_left_square /
	    (double)PC_TOTAL_FRAMES);
	metrics.output_right_square = sqrt(metrics.output_right_square /
	    (double)PC_TOTAL_FRAMES);
	metrics.final_gain = processor.agc.gain;
	metrics.final_gate_state = processor.agc.gate_state;
	metrics.analysis_clip_confidence =
	    processor.restoration.metrics.clipping_confidence;
	metrics.analysis_low_ceiling_confidence =
	    processor.restoration.metrics.low_ceiling_clipping_confidence;
	metrics.analysis_transient_confidence =
	    processor.restoration.metrics.transient_confidence;
	metrics.analysis_source_profile =
	    processor.restoration.metrics.source_profile;
	metrics.analysis_reason_flags =
	    processor.restoration.metrics.reason_flags;
	metrics.declipper_repaired_samples =
	    processor.declipper.metrics.repaired_sample_count;
	metrics.declipper_repaired_runs =
	    processor.declipper.metrics.repaired_run_count;
	metrics.declipper_max_delta =
	    processor.declipper.metrics.max_repair_delta;
	metrics.declipper_bypass_reason =
	    processor.declipper.metrics.bypass_reason;
	metrics.natural_dynamics_gr_db =
	    processor.natural_dynamics.gain_reduction_db;
	metrics.low_level_boost_gain_db =
	    processor.low_level_boost.gain_db;
	metrics.auto_eq_total_rms = processor.auto_eq.metrics.total_rms;
	metrics.auto_eq_spectral_tilt_db =
	    processor.auto_eq.metrics.spectral_tilt_db;
	metrics.auto_eq_low_weight =
	    processor.auto_eq.metrics.low_frequency_weight;
	metrics.auto_eq_presence_weight =
	    processor.auto_eq.metrics.presence_weight;
	metrics.auto_eq_high_weight =
	    processor.auto_eq.metrics.high_frequency_weight;
	metrics.auto_eq_source_hint =
	    processor.auto_eq.metrics.source_hint;

	pass = pc_check_case(test, &metrics, &config);
	if (pass) {
		printf("professional_check profile=%s fixture=%s "
		    "input_rms=%0.6f output_rms=%0.6f "
		    "output_peak=%0.6f output_crest=%0.6f status=pass\n",
		    pc_profile_name(test->profile),
		    pc_fixture_name(test->fixture), metrics.input_square,
		    metrics.output_square, metrics.output_peak,
		    pc_crest(&metrics, 0));
	}

	return pass;
}

static const char *
pc_profile_name(enum pc_profile profile)
{
	switch (profile) {
	case PC_PROFILE_DEFAULT:
		return "default";
	case PC_PROFILE_DEHUM_50:
		return "dehum-50";
	case PC_PROFILE_DEHUM_60:
		return "dehum-60";
	case PC_PROFILE_DECLIPPER:
		return "declipper";
	case PC_PROFILE_DYNAMICS:
		return "dynamics";
	case PC_PROFILE_AUTO_EQ:
		return "auto-eq";
	case PC_PROFILE_MB_BASS:
		return "multiband-bass";
	case PC_PROFILE_MB2:
		return "multiband2";
	case PC_PROFILE_AM_SAFE:
		return "am-safe";
	case PC_PROFILE_AM_SHORTWAVE:
		return "am-shortwave";
	case PC_PROFILE_AM_WIDE:
		return "am-wide";
	case PC_PROFILE_AM_VOICE:
		return "am-voice";
	case PC_PROFILE_SSB_SPEECH:
		return "ssb-speech";
	case PC_PROFILE_SSB_NARROW:
		return "ssb-narrow";
	case PC_PROFILE_SSB_WIDE:
		return "ssb-wide";
	default:
		return "unknown";
	}
}

static void
pc_update(struct pc_metrics *metrics, const cp_sample_t *input,
	const cp_sample_t *output, size_t offset, size_t frames)
{
	cp_sample_t input_abs;
	cp_sample_t input_mono;
	cp_sample_t output_abs;
	cp_sample_t output_mono;
	double phase50;
	double phase60;
	size_t frame;
	size_t index;
	size_t sample;
	size_t samples;

	samples = frames * CP_CHANNELS_STEREO;
	for (sample = 0; sample < samples; sample++) {
		if (!isfinite(input[sample]) || !isfinite(output[sample])) {
			metrics->finite = 0;
			continue;
		}
		input_abs = pc_abs(input[sample]);
		output_abs = pc_abs(output[sample]);
		if (input_abs > metrics->input_peak)
			metrics->input_peak = input_abs;
		if (output_abs > metrics->output_peak)
			metrics->output_peak = output_abs;
		if (output[sample] < metrics->output_min)
			metrics->output_min = output[sample];
		if (output[sample] > metrics->output_max)
			metrics->output_max = output[sample];
		metrics->input_square +=
		    (double)input[sample] * (double)input[sample];
		metrics->output_square +=
		    (double)output[sample] * (double)output[sample];
		metrics->input_sum += (double)input[sample];
		metrics->output_sum += (double)output[sample];
	}

	for (frame = 0; frame < frames; frame++) {
		index = frame * CP_CHANNELS_STEREO;
		metrics->output_left_square +=
		    (double)output[index] * (double)output[index];
		metrics->output_right_square +=
		    (double)output[index + 1] * (double)output[index + 1];

		input_mono = (input[index] + input[index + 1]) * 0.5f;
		output_mono = (output[index] + output[index + 1]) * 0.5f;
		phase50 = 2.0 * (double)PC_PI * 50.0 *
		    (double)(offset + frame) / (double)PC_RATE;
		phase60 = 2.0 * (double)PC_PI * 60.0 *
		    (double)(offset + frame) / (double)PC_RATE;

		metrics->input_hum50_sin += (double)input_mono *
		    sin(phase50);
		metrics->input_hum50_cos += (double)input_mono *
		    cos(phase50);
		metrics->input_hum60_sin += (double)input_mono *
		    sin(phase60);
		metrics->input_hum60_cos += (double)input_mono *
		    cos(phase60);
		metrics->output_hum50_sin += (double)output_mono *
		    sin(phase50);
		metrics->output_hum50_cos += (double)output_mono *
		    cos(phase50);
		metrics->output_hum60_sin += (double)output_mono *
		    sin(phase60);
		metrics->output_hum60_cos += (double)output_mono *
		    cos(phase60);
	}
}

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_quality_report.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_block.h"
#include "cp_declipper.h"

#define QR_BLOCK_FRAMES		256
#define QR_TOTAL_FRAMES		8192
#define QR_RATE			(48000.0f)
#define QR_PI			(3.14159265358979323846f)
#define QR_LIMIT_EPSILON	(0.002f)
#define QR_SILENCE_LIMIT	(0.0001f)

enum qr_fixture {
	QR_FIXTURE_SILENCE = 0,
	QR_FIXTURE_SPEECH_STEPS,
	QR_FIXTURE_MUSIC_MIX,
	QR_FIXTURE_CLIPPED,
	QR_FIXTURE_LOW_CEILING_CLIPPED,
	QR_FIXTURE_DC_OFFSET,
	QR_FIXTURE_HUM_50,
	QR_FIXTURE_HUM_60,
	QR_FIXTURE_BURST,
	QR_FIXTURE_HIGH_TONE,
	QR_FIXTURE_AM_LIMITED,
	QR_FIXTURE_SSB_LIMITED,
	QR_FIXTURE_STEREO_IMBALANCE
};

enum qr_profile {
	QR_PROFILE_DEFAULT = 0,
	QR_PROFILE_DEHUM_50,
	QR_PROFILE_DEHUM_60,
	QR_PROFILE_DECLIPPER,
	QR_PROFILE_DYNAMICS,
	QR_PROFILE_AUTO_EQ,
	QR_PROFILE_MB_BASS,
	QR_PROFILE_MB2,
	QR_PROFILE_AM_SHORTWAVE,
	QR_PROFILE_SSB_NARROW
};

struct qr_case {
	enum qr_profile profile;
	enum qr_fixture fixture;
	const char *check;
};

struct qr_metrics {
	double input_square;
	double output_square;
	double input_sum;
	double output_sum;
	double input_left_square;
	double input_right_square;
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
	cp_sample_t analysis_clip_ratio;
	cp_sample_t analysis_hf_ratio;
	cp_sample_t analysis_clip_confidence;
	cp_sample_t analysis_lossy_confidence;
	cp_sample_t analysis_low_ceiling_confidence;
	cp_sample_t analysis_transient_confidence;
	cp_sample_t analysis_flat_run_ratio;
	cp_sample_t analysis_peak_repeat_ratio;
	cp_sample_t analysis_observed_peak;
	cp_sample_t analysis_crest_factor;
	cp_sample_t declipper_max_delta;
	cp_sample_t natural_dynamics_gr_db;
	cp_sample_t low_level_boost_gain_db;
	cp_sample_t auto_eq_total_rms;
	cp_sample_t auto_eq_low_weight;
	cp_sample_t auto_eq_presence_weight;
	cp_sample_t auto_eq_high_weight;
	cp_sample_t auto_eq_spectral_tilt_db;
	cp_sample_t bass_eq_recommend_low_gain_db;
	cp_sample_t bass_eq_recommend_high_gain_db;
	cp_sample_t bass_eq_recommend_output_gain_db;
	cp_sample_t bass_eq_recommend_confidence;
	enum cp_auto_eq_source_hint auto_eq_source_hint;
	enum cp_restoration_source_profile analysis_source_profile;
	unsigned int analysis_reason_flags;
	size_t declipper_repaired_samples;
	size_t declipper_repaired_runs;
	int declipper_bypass_reason;
	int bass_eq_recommend_valid;
	int bass_eq_recommend_preset;
	int finite;
};

static cp_sample_t	qr_abs(cp_sample_t);
static int		qr_check_case(const struct qr_case *,
			    const struct qr_metrics *);
static void		qr_config(struct cp_block_config *, enum qr_profile);
static cp_sample_t	qr_crest(const struct qr_metrics *, int);
static cp_sample_t	qr_hum_amp(const struct qr_metrics *, int, int);
static int		qr_generate(enum qr_fixture, cp_sample_t *, size_t,
			    size_t);
static const char	*qr_fixture_name(enum qr_fixture);
static cp_sample_t	qr_noise(size_t, size_t);
static const char	*qr_profile_name(enum qr_profile);
static int		qr_run_case(const struct qr_case *);
static void		qr_update(struct qr_metrics *, const cp_sample_t *,
			    const cp_sample_t *, size_t, size_t);

int
main(void)
{
	static const struct qr_case cases[] = {
		{ QR_PROFILE_DEFAULT, QR_FIXTURE_SILENCE, "silence" },
		{ QR_PROFILE_DEFAULT, QR_FIXTURE_SPEECH_STEPS, "speech" },
		{ QR_PROFILE_DEFAULT, QR_FIXTURE_MUSIC_MIX, "music" },
		{ QR_PROFILE_DEFAULT, QR_FIXTURE_CLIPPED, "clipped" },
		{ QR_PROFILE_DEFAULT, QR_FIXTURE_LOW_CEILING_CLIPPED,
		    "low-ceiling" },
		{ QR_PROFILE_DEFAULT, QR_FIXTURE_BURST, "burst" },
		{ QR_PROFILE_DEFAULT, QR_FIXTURE_AM_LIMITED, "am-limited" },
		{ QR_PROFILE_DEFAULT, QR_FIXTURE_SSB_LIMITED, "ssb-limited" },
		{ QR_PROFILE_DEFAULT, QR_FIXTURE_DC_OFFSET, "dc" },
		{ QR_PROFILE_DEHUM_50, QR_FIXTURE_HUM_50, "dehum-50" },
		{ QR_PROFILE_DEHUM_60, QR_FIXTURE_HUM_60, "dehum-60" },
		{ QR_PROFILE_DECLIPPER, QR_FIXTURE_CLIPPED,
		    "declipper-clipped" },
		{ QR_PROFILE_DECLIPPER, QR_FIXTURE_LOW_CEILING_CLIPPED,
		    "declipper-low-ceiling" },
		{ QR_PROFILE_DECLIPPER, QR_FIXTURE_BURST,
		    "declipper-burst" },
		{ QR_PROFILE_DYNAMICS, QR_FIXTURE_SILENCE,
		    "dynamics-silence" },
		{ QR_PROFILE_DYNAMICS, QR_FIXTURE_SPEECH_STEPS,
		    "dynamics-speech" },
		{ QR_PROFILE_AUTO_EQ, QR_FIXTURE_MUSIC_MIX, "auto-eq-music" },
		{ QR_PROFILE_AUTO_EQ, QR_FIXTURE_AM_LIMITED,
		    "auto-eq-am-limited" },
		{ QR_PROFILE_MB_BASS, QR_FIXTURE_MUSIC_MIX, "mb-music" },
		{ QR_PROFILE_MB_BASS, QR_FIXTURE_STEREO_IMBALANCE, "stereo" },
		{ QR_PROFILE_MB2, QR_FIXTURE_MUSIC_MIX, "mb2-music" },
		{ QR_PROFILE_AM_SHORTWAVE, QR_FIXTURE_HIGH_TONE, "am-lpf" },
		{ QR_PROFILE_AM_SHORTWAVE, QR_FIXTURE_SPEECH_STEPS,
		    "am-speech" },
		{ QR_PROFILE_SSB_NARROW, QR_FIXTURE_HIGH_TONE, "ssb-lpf" },
		{ QR_PROFILE_SSB_NARROW, QR_FIXTURE_SPEECH_STEPS,
		    "ssb-speech" }
	};
	size_t i;

	for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		if (!qr_run_case(&cases[i]))
			return 1;
	}

	printf("quality cases=%zu status=pass\n",
	    sizeof(cases) / sizeof(cases[0]));
	return 0;
}

static cp_sample_t
qr_abs(cp_sample_t sample)
{
	return sample < 0.0f ? -sample : sample;
}

static int
qr_check_case(const struct qr_case *test, const struct qr_metrics *metrics)
{
	cp_sample_t input_dc;
	cp_sample_t output_dc;
	cp_sample_t input_hum;
	cp_sample_t output_hum;

	if (test == NULL || metrics == NULL)
		return 0;
	if (!metrics->finite)
		return 0;
	if (metrics->output_peak > CP_DEFAULT_CEILING + QR_LIMIT_EPSILON)
		return 0;

	input_dc = qr_abs((cp_sample_t)metrics->input_sum);
	output_dc = qr_abs((cp_sample_t)metrics->output_sum);
	if (test->fixture == QR_FIXTURE_SILENCE &&
	    metrics->output_peak > QR_SILENCE_LIMIT)
		return 0;
	if (test->fixture == QR_FIXTURE_DC_OFFSET &&
	    output_dc >= input_dc * 0.40f)
		return 0;
	if (test->profile == QR_PROFILE_DEHUM_50) {
		input_hum = qr_hum_amp(metrics, 50, 1);
		output_hum = qr_hum_amp(metrics, 50, 0);
		if (output_hum >= input_hum * 0.65f)
			return 0;
	}
	if (test->profile == QR_PROFILE_DEHUM_60) {
		input_hum = qr_hum_amp(metrics, 60, 1);
		output_hum = qr_hum_amp(metrics, 60, 0);
		if (output_hum >= input_hum * 0.65f)
			return 0;
	}
	if (test->profile == QR_PROFILE_DECLIPPER &&
	    test->fixture == QR_FIXTURE_CLIPPED) {
		if (metrics->declipper_repaired_samples == 0 ||
		    metrics->declipper_repaired_runs == 0)
			return 0;
	}
	if (test->profile == QR_PROFILE_DECLIPPER &&
	    test->fixture == QR_FIXTURE_LOW_CEILING_CLIPPED) {
		if (metrics->declipper_repaired_samples == 0 ||
		    metrics->declipper_max_delta <= 0.0f)
			return 0;
	}
	if (test->profile == QR_PROFILE_DECLIPPER &&
	    test->fixture == QR_FIXTURE_BURST) {
		if (metrics->declipper_repaired_samples != 0)
			return 0;
		if (metrics->declipper_bypass_reason !=
		    CP_DECLIPPER_BYPASS_TRANSIENT &&
		    metrics->declipper_bypass_reason !=
		    CP_DECLIPPER_BYPASS_LOW_CONFIDENCE)
			return 0;
	}
	if (test->profile == QR_PROFILE_DYNAMICS &&
	    test->fixture == QR_FIXTURE_SILENCE) {
		if (metrics->low_level_boost_gain_db > 0.10f)
			return 0;
	}
	if (test->profile == QR_PROFILE_DYNAMICS &&
	    test->fixture == QR_FIXTURE_SPEECH_STEPS) {
		if (metrics->natural_dynamics_gr_db < 0.10f &&
		    metrics->low_level_boost_gain_db < 0.10f)
			return 0;
	}
	if (test->profile == QR_PROFILE_AUTO_EQ) {
		if (metrics->auto_eq_total_rms <= 0.0f ||
		    !isfinite(metrics->auto_eq_spectral_tilt_db) ||
		    !metrics->bass_eq_recommend_valid)
			return 0;
		if (!isfinite(metrics->bass_eq_recommend_low_gain_db) ||
		    !isfinite(metrics->bass_eq_recommend_high_gain_db) ||
		    !isfinite(metrics->bass_eq_recommend_output_gain_db) ||
		    metrics->bass_eq_recommend_low_gain_db < -3.0f ||
		    metrics->bass_eq_recommend_low_gain_db > 3.0f ||
		    metrics->bass_eq_recommend_high_gain_db < -3.0f ||
		    metrics->bass_eq_recommend_high_gain_db > 3.0f ||
		    metrics->bass_eq_recommend_output_gain_db < -3.0f ||
		    metrics->bass_eq_recommend_output_gain_db > 0.0f ||
		    metrics->bass_eq_recommend_confidence <= 0.0f ||
		    metrics->bass_eq_recommend_confidence > 1.0f)
			return 0;
		if (test->fixture == QR_FIXTURE_AM_LIMITED &&
		    (metrics->auto_eq_source_hint ==
		    CP_AUTO_EQ_SOURCE_BRIGHT ||
		    metrics->auto_eq_source_hint == CP_AUTO_EQ_SOURCE_SILENCE))
			return 0;
	}
	if (test->check[0] == 'a' && test->fixture == QR_FIXTURE_HIGH_TONE &&
	    metrics->output_square >= metrics->input_square * 0.45)
		return 0;
	if (test->check[0] == 's' && test->fixture == QR_FIXTURE_HIGH_TONE &&
	    metrics->output_square >= metrics->input_square * 0.35)
		return 0;
	if (test->fixture == QR_FIXTURE_STEREO_IMBALANCE &&
	    metrics->output_left_square <= metrics->output_right_square * 2.0)
		return 0;
	if (test->fixture == QR_FIXTURE_CLIPPED) {
		if (metrics->analysis_clip_confidence < 0.50f)
			return 0;
		if ((metrics->analysis_reason_flags &
		    CP_RESTORATION_REASON_HARD_CLIP) == 0)
			return 0;
	}
	if (test->fixture == QR_FIXTURE_LOW_CEILING_CLIPPED) {
		if (metrics->analysis_low_ceiling_confidence < 0.50f)
			return 0;
		if ((metrics->analysis_reason_flags &
		    CP_RESTORATION_REASON_LOW_CEILING) == 0)
			return 0;
	}
	if (test->fixture == QR_FIXTURE_BURST) {
		if (metrics->analysis_clip_confidence > 0.40f)
			return 0;
		if (metrics->analysis_transient_confidence < 0.50f)
			return 0;
		if ((metrics->analysis_reason_flags &
		    CP_RESTORATION_REASON_TRANSIENT) == 0)
			return 0;
	}
	if (test->fixture == QR_FIXTURE_AM_LIMITED &&
	    metrics->analysis_source_profile != CP_RESTORATION_SOURCE_AM_LIMITED)
		return 0;
	if (test->fixture == QR_FIXTURE_SSB_LIMITED &&
	    metrics->analysis_source_profile != CP_RESTORATION_SOURCE_SSB_VOICE)
		return 0;

	return 1;
}

static void
qr_config(struct cp_block_config *config, enum qr_profile profile)
{
	cp_block_default_config(config, CP_CHANNELS_STEREO);
	config->sample_rate = QR_RATE;
	config->limiter_ceiling = CP_DEFAULT_CEILING;
	config->restoration_config.enabled = 1;
	config->restoration_config.analysis_window_frames = QR_TOTAL_FRAMES;

	switch (profile) {
	case QR_PROFILE_DEFAULT:
		break;
	case QR_PROFILE_DEHUM_50:
		config->dehummer_enabled = 1;
		config->hum_base_frequency = 50.0f;
		config->hum_harmonic_count = 4;
		break;
	case QR_PROFILE_DEHUM_60:
		config->dehummer_enabled = 1;
		config->hum_base_frequency = 60.0f;
		config->hum_harmonic_count = 4;
		break;
	case QR_PROFILE_DECLIPPER:
		config->declipper_config.enabled = 1;
		config->declipper_config.repair_strength = 0.35f;
		config->declipper_config.max_repair_samples = 16u;
		break;
	case QR_PROFILE_DYNAMICS:
		config->natural_dynamics_config.enabled = 1;
		config->natural_dynamics_config.threshold_db = -18.0f;
		config->natural_dynamics_config.ratio = 1.5f;
		config->natural_dynamics_config.max_gain_reduction_db = 4.0f;
		config->low_level_boost_config.enabled = 1;
		config->low_level_boost_config.target_rms = 0.12f;
		config->low_level_boost_config.max_boost_db = 5.0f;
		break;
	case QR_PROFILE_AUTO_EQ:
		config->auto_eq_config.enabled = 1;
		config->auto_eq_config.analysis_window_frames =
		    QR_TOTAL_FRAMES;
		break;
	case QR_PROFILE_MB_BASS:
		config->multiband_enabled = 1;
		config->multiband_band_count = 3;
		config->multiband_preset = CP_MULTIBAND_PRESET_MUSIC;
		config->bass_eq_config.enabled = 1;
		(void)cp_bass_eq_apply_preset(&config->bass_eq_config,
		    "music");
		break;
	case QR_PROFILE_MB2:
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
	case QR_PROFILE_AM_SHORTWAVE:
		config->dehummer_enabled = 1;
		config->hum_base_frequency = 50.0f;
		config->hum_harmonic_count = 4;
		config->multiband_enabled = 1;
		config->multiband_band_count = 3;
		config->multiband_preset = CP_MULTIBAND_PRESET_SPEECH;
		config->bass_eq_config.enabled = 1;
		(void)cp_bass_eq_apply_preset(&config->bass_eq_config,
		    "speech");
		(void)cp_am_apply_preset(&config->am_config, "am-shortwave");
		config->am_config.enabled = 1;
		break;
	case QR_PROFILE_SSB_NARROW:
		config->dehummer_enabled = 1;
		config->hum_base_frequency = 50.0f;
		config->hum_harmonic_count = 4;
		config->multiband_enabled = 1;
		config->multiband_band_count = 3;
		config->multiband_preset = CP_MULTIBAND_PRESET_SPEECH;
		config->bass_eq_config.enabled = 1;
		(void)cp_bass_eq_apply_preset(&config->bass_eq_config,
		    "speech");
		(void)cp_ssb_apply_preset(&config->ssb_config, "ssb-narrow");
		config->ssb_config.enabled = 1;
		break;
	default:
		break;
	}
}

static cp_sample_t
qr_crest(const struct qr_metrics *metrics, int input)
{
	cp_sample_t peak;
	double square;

	if (metrics == NULL)
		return 0.0f;

	peak = input ? metrics->input_peak : metrics->output_peak;
	square = input ? metrics->input_square : metrics->output_square;
	if (square <= 0.0)
		return 0.0f;

	return peak / (cp_sample_t)square;
}

static int
qr_generate(enum qr_fixture fixture, cp_sample_t *buffer, size_t offset,
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
		phase = 2.0f * QR_PI * (cp_sample_t)index / QR_RATE;
		switch (fixture) {
		case QR_FIXTURE_SILENCE:
			left = 0.0f;
			right = 0.0f;
			break;
		case QR_FIXTURE_SPEECH_STEPS:
			if (index < QR_TOTAL_FRAMES / 4)
				value = 0.04f;
			else if (index < QR_TOTAL_FRAMES / 2)
				value = 0.18f;
			else if (index < (QR_TOTAL_FRAMES * 3) / 4)
				value = 0.42f;
			else
				value = 0.08f;
			left = value * ((0.70f * sinf(phase * 650.0f)) +
			    (0.20f * sinf(phase * 1300.0f)) +
			    (0.10f * qr_noise(index, 0)));
			right = left;
			break;
		case QR_FIXTURE_MUSIC_MIX:
			left = (0.12f * sinf(phase * 110.0f)) +
			    (0.08f * sinf(phase * 220.0f)) +
			    (0.06f * sinf(phase * 440.0f)) +
			    (0.04f * sinf(phase * 880.0f)) +
			    (0.02f * sinf(phase * 1760.0f));
			right = left;
			break;
		case QR_FIXTURE_CLIPPED:
			value = 1.4f * sinf(phase * 700.0f);
			if (value > 0.98f)
				value = 0.98f;
			if (value < -0.98f)
				value = -0.98f;
			left = value;
			right = value;
			break;
		case QR_FIXTURE_LOW_CEILING_CLIPPED:
			value = 0.90f * sinf(phase * 700.0f);
			if (value > 0.45f)
				value = 0.45f;
			if (value < -0.45f)
				value = -0.45f;
			left = value;
			right = value;
			break;
		case QR_FIXTURE_DC_OFFSET:
			left = 0.22f + (0.04f * sinf(phase * 400.0f));
			right = left;
			break;
		case QR_FIXTURE_HUM_50:
			left = (0.20f * sinf(phase * 50.0f)) +
			    (0.04f * sinf(phase * 1000.0f));
			right = left;
			break;
		case QR_FIXTURE_HUM_60:
			left = (0.20f * sinf(phase * 60.0f)) +
			    (0.04f * sinf(phase * 1000.0f));
			right = left;
			break;
		case QR_FIXTURE_BURST:
			if ((index % 2048u) < 4u)
				left = 0.99f;
			else if ((index % 1024u) < 40u)
				left = 0.85f * sinf(phase * 1000.0f);
			else
				left = 0.03f * sinf(phase * 1000.0f);
			right = left;
			break;
		case QR_FIXTURE_HIGH_TONE:
			left = 0.20f * sinf(phase * 10000.0f);
			right = left;
			break;
		case QR_FIXTURE_AM_LIMITED:
			left = (0.20f * sinf(phase * 2500.0f)) +
			    (0.04f * sinf(phase * 1200.0f));
			right = left;
			break;
		case QR_FIXTURE_SSB_LIMITED:
			left = (0.20f * sinf(phase * 900.0f)) +
			    (0.04f * sinf(phase * 1600.0f));
			right = left;
			break;
		case QR_FIXTURE_STEREO_IMBALANCE:
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
qr_hum_amp(const struct qr_metrics *metrics, int hum_hz, int input)
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
	    (double)QR_TOTAL_FRAMES);
}

static const char *
qr_fixture_name(enum qr_fixture fixture)
{
	switch (fixture) {
	case QR_FIXTURE_SILENCE:
		return "silence";
	case QR_FIXTURE_SPEECH_STEPS:
		return "speech-steps";
	case QR_FIXTURE_MUSIC_MIX:
		return "music-mix";
	case QR_FIXTURE_CLIPPED:
		return "clipped";
	case QR_FIXTURE_LOW_CEILING_CLIPPED:
		return "low-ceiling-clipped";
	case QR_FIXTURE_DC_OFFSET:
		return "dc-offset";
	case QR_FIXTURE_HUM_50:
		return "hum-50";
	case QR_FIXTURE_HUM_60:
		return "hum-60";
	case QR_FIXTURE_BURST:
		return "burst";
	case QR_FIXTURE_HIGH_TONE:
		return "high-tone";
	case QR_FIXTURE_AM_LIMITED:
		return "am-limited";
	case QR_FIXTURE_SSB_LIMITED:
		return "ssb-limited";
	case QR_FIXTURE_STEREO_IMBALANCE:
		return "stereo-imbalance";
	default:
		return "unknown";
	}
}

static cp_sample_t
qr_noise(size_t frame, size_t channel)
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

static const char *
qr_profile_name(enum qr_profile profile)
{
	switch (profile) {
	case QR_PROFILE_DEFAULT:
		return "default";
	case QR_PROFILE_DEHUM_50:
		return "dehum-50";
	case QR_PROFILE_DEHUM_60:
		return "dehum-60";
	case QR_PROFILE_DECLIPPER:
		return "declipper";
	case QR_PROFILE_DYNAMICS:
		return "dynamics";
	case QR_PROFILE_AUTO_EQ:
		return "auto-eq";
	case QR_PROFILE_MB_BASS:
		return "multiband-bass";
	case QR_PROFILE_MB2:
		return "multiband2";
	case QR_PROFILE_AM_SHORTWAVE:
		return "am-shortwave";
	case QR_PROFILE_SSB_NARROW:
		return "ssb-narrow";
	default:
		return "unknown";
	}
}

static int
qr_run_case(const struct qr_case *test)
{
	cp_sample_t input[QR_BLOCK_FRAMES * CP_CHANNELS_STEREO];
	cp_sample_t output[QR_BLOCK_FRAMES * CP_CHANNELS_STEREO];
	cp_sample_t scratch[QR_BLOCK_FRAMES * CP_CHANNELS_STEREO];
	struct cp_block_config config;
	struct cp_block_processor processor;
	struct cp_bass_eq_recommendation recommendation;
	struct qr_metrics metrics;
	size_t frames;
	size_t offset;
	int pass;
	int status;

	if (test == NULL)
		return 0;

	qr_config(&config, test->profile);
	status = cp_block_init(&processor, &config);
	if (status != CP_OK) {
		printf("quality profile=%s fixture=%s check=%s "
		    "status=fail reason=init code=%d\n",
		    qr_profile_name(test->profile),
		    qr_fixture_name(test->fixture), test->check, status);
		return 0;
	}

	metrics.input_square = 0.0;
	metrics.output_square = 0.0;
	metrics.input_sum = 0.0;
	metrics.output_sum = 0.0;
	metrics.input_left_square = 0.0;
	metrics.input_right_square = 0.0;
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
	metrics.analysis_clip_ratio = 0.0f;
	metrics.analysis_hf_ratio = 0.0f;
	metrics.analysis_clip_confidence = 0.0f;
	metrics.analysis_lossy_confidence = 0.0f;
	metrics.analysis_low_ceiling_confidence = 0.0f;
	metrics.analysis_transient_confidence = 0.0f;
	metrics.analysis_flat_run_ratio = 0.0f;
	metrics.analysis_peak_repeat_ratio = 0.0f;
	metrics.analysis_observed_peak = 0.0f;
	metrics.analysis_crest_factor = 0.0f;
	metrics.declipper_max_delta = 0.0f;
	metrics.natural_dynamics_gr_db = 0.0f;
	metrics.low_level_boost_gain_db = 0.0f;
	metrics.auto_eq_total_rms = 0.0f;
	metrics.auto_eq_low_weight = 0.0f;
	metrics.auto_eq_presence_weight = 0.0f;
	metrics.auto_eq_high_weight = 0.0f;
	metrics.auto_eq_spectral_tilt_db = 0.0f;
	metrics.bass_eq_recommend_low_gain_db = 0.0f;
	metrics.bass_eq_recommend_high_gain_db = 0.0f;
	metrics.bass_eq_recommend_output_gain_db = 0.0f;
	metrics.bass_eq_recommend_confidence = 0.0f;
	metrics.auto_eq_source_hint = CP_AUTO_EQ_SOURCE_UNKNOWN;
	metrics.analysis_source_profile = CP_RESTORATION_SOURCE_UNKNOWN;
	metrics.analysis_reason_flags = 0u;
	metrics.declipper_repaired_samples = 0;
	metrics.declipper_repaired_runs = 0;
	metrics.declipper_bypass_reason = CP_DECLIPPER_BYPASS_DISABLED;
	metrics.bass_eq_recommend_valid = 0;
	metrics.bass_eq_recommend_preset = CP_BASS_EQ_PRESET_FLAT;
	metrics.finite = 1;

	for (offset = 0; offset < QR_TOTAL_FRAMES; offset += QR_BLOCK_FRAMES) {
		frames = QR_BLOCK_FRAMES;
		if (offset + frames > QR_TOTAL_FRAMES)
			frames = QR_TOTAL_FRAMES - offset;
		if (!qr_generate(test->fixture, input, offset, frames))
			return 0;
		status = cp_block_process(&processor, input, output, scratch,
		    frames * CP_CHANNELS_STEREO, frames);
		if (status != CP_OK) {
			printf("quality profile=%s fixture=%s check=%s "
			    "status=fail reason=process code=%d\n",
			    qr_profile_name(test->profile),
			    qr_fixture_name(test->fixture), test->check,
			    status);
			return 0;
		}
		qr_update(&metrics, input, output, offset, frames);
	}

	metrics.input_square = sqrt(metrics.input_square /
	    (double)(QR_TOTAL_FRAMES * CP_CHANNELS_STEREO));
	metrics.output_square = sqrt(metrics.output_square /
	    (double)(QR_TOTAL_FRAMES * CP_CHANNELS_STEREO));
	metrics.input_sum = metrics.input_sum /
	    (double)(QR_TOTAL_FRAMES * CP_CHANNELS_STEREO);
	metrics.output_sum = metrics.output_sum /
	    (double)(QR_TOTAL_FRAMES * CP_CHANNELS_STEREO);
	metrics.input_left_square = sqrt(metrics.input_left_square /
	    (double)QR_TOTAL_FRAMES);
	metrics.input_right_square = sqrt(metrics.input_right_square /
	    (double)QR_TOTAL_FRAMES);
	metrics.output_left_square = sqrt(metrics.output_left_square /
	    (double)QR_TOTAL_FRAMES);
	metrics.output_right_square = sqrt(metrics.output_right_square /
	    (double)QR_TOTAL_FRAMES);
	metrics.analysis_clip_ratio =
	    processor.restoration.metrics.clipped_sample_ratio;
	metrics.analysis_hf_ratio =
	    processor.restoration.metrics.high_frequency_ratio;
	metrics.analysis_clip_confidence =
	    processor.restoration.metrics.clipping_confidence;
	metrics.analysis_lossy_confidence =
	    processor.restoration.metrics.lossy_confidence;
	metrics.analysis_low_ceiling_confidence =
	    processor.restoration.metrics.low_ceiling_clipping_confidence;
	metrics.analysis_transient_confidence =
	    processor.restoration.metrics.transient_confidence;
	metrics.analysis_flat_run_ratio =
	    processor.restoration.metrics.flat_run_ratio;
	metrics.analysis_peak_repeat_ratio =
	    processor.restoration.metrics.peak_repeat_ratio;
	metrics.analysis_observed_peak =
	    processor.restoration.metrics.observed_peak;
	metrics.analysis_crest_factor =
	    processor.restoration.metrics.crest_factor;
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
	metrics.auto_eq_low_weight =
	    processor.auto_eq.metrics.low_frequency_weight;
	metrics.auto_eq_presence_weight =
	    processor.auto_eq.metrics.presence_weight;
	metrics.auto_eq_high_weight =
	    processor.auto_eq.metrics.high_frequency_weight;
	metrics.auto_eq_spectral_tilt_db =
	    processor.auto_eq.metrics.spectral_tilt_db;
	metrics.auto_eq_source_hint =
	    processor.auto_eq.metrics.source_hint;
	if (cp_bass_eq_recommend(&processor.auto_eq.metrics,
	    &recommendation) == CP_OK) {
		metrics.bass_eq_recommend_valid = recommendation.valid;
		metrics.bass_eq_recommend_preset = recommendation.preset;
		metrics.bass_eq_recommend_low_gain_db =
		    recommendation.low_gain_db;
		metrics.bass_eq_recommend_high_gain_db =
		    recommendation.high_gain_db;
		metrics.bass_eq_recommend_output_gain_db =
		    recommendation.output_gain_db;
		metrics.bass_eq_recommend_confidence =
		    recommendation.confidence;
	}
	pass = qr_check_case(test, &metrics);

	printf("quality profile=%s fixture=%s check=%s input_rms=%0.6f "
	    "output_rms=%0.6f input_peak=%0.6f output_peak=%0.6f "
	    "output_min=%0.6f output_max=%0.6f input_crest=%0.6f "
	    "output_crest=%0.6f input_dc=%0.6f output_dc=%0.6f "
	    "input_hum50=%0.6f output_hum50=%0.6f input_hum60=%0.6f "
	    "output_hum60=%0.6f output_left_rms=%0.6f "
	    "output_right_rms=%0.6f analysis_clip_ratio=%0.6f "
	    "analysis_hf_ratio=%0.6f analysis_clip_confidence=%0.6f "
	    "analysis_low_ceiling_confidence=%0.6f "
	    "analysis_transient_confidence=%0.6f "
	    "analysis_lossy_confidence=%0.6f analysis_flat_ratio=%0.6f "
	    "analysis_peak_repeat_ratio=%0.6f analysis_peak=%0.6f "
	    "analysis_crest=%0.6f analysis_profile=%s "
	    "analysis_reason_flags=0x%08x declipper_samples=%zu "
	    "declipper_runs=%zu declipper_delta=%0.6f "
	    "declipper_bypass=%s natural_gr_db=%0.6f "
	    "low_boost_gain_db=%0.6f auto_eq_source=%s "
	    "auto_eq_rms=%0.6f auto_eq_tilt_db=%0.6f "
	    "auto_eq_low=%0.6f auto_eq_presence=%0.6f "
	    "auto_eq_high=%0.6f bass_eq_recommend=%s "
	    "bass_eq_recommend_preset=%s "
	    "bass_eq_recommend_low_db=%0.6f "
	    "bass_eq_recommend_high_db=%0.6f "
	    "bass_eq_recommend_output_db=%0.6f "
	    "bass_eq_recommend_confidence=%0.6f status=%s\n",
	    qr_profile_name(test->profile), qr_fixture_name(test->fixture),
	    test->check, metrics.input_square, metrics.output_square,
	    metrics.input_peak, metrics.output_peak, metrics.output_min,
	    metrics.output_max, qr_crest(&metrics, 1), qr_crest(&metrics, 0),
	    qr_abs((cp_sample_t)metrics.input_sum),
	    qr_abs((cp_sample_t)metrics.output_sum),
	    qr_hum_amp(&metrics, 50, 1), qr_hum_amp(&metrics, 50, 0),
	    qr_hum_amp(&metrics, 60, 1), qr_hum_amp(&metrics, 60, 0),
	    metrics.output_left_square, metrics.output_right_square,
	    metrics.analysis_clip_ratio, metrics.analysis_hf_ratio,
	    metrics.analysis_clip_confidence,
	    metrics.analysis_low_ceiling_confidence,
	    metrics.analysis_transient_confidence,
	    metrics.analysis_lossy_confidence,
	    metrics.analysis_flat_run_ratio,
	    metrics.analysis_peak_repeat_ratio,
	    metrics.analysis_observed_peak,
	    metrics.analysis_crest_factor,
	    cp_restoration_source_profile_string(
	    metrics.analysis_source_profile),
	    metrics.analysis_reason_flags,
	    metrics.declipper_repaired_samples,
	    metrics.declipper_repaired_runs,
	    metrics.declipper_max_delta,
	    cp_declipper_bypass_reason_string(
	    (enum cp_declipper_bypass_reason)
	    metrics.declipper_bypass_reason),
	    metrics.natural_dynamics_gr_db,
	    metrics.low_level_boost_gain_db,
	    cp_auto_eq_source_hint_string(metrics.auto_eq_source_hint),
	    metrics.auto_eq_total_rms,
	    metrics.auto_eq_spectral_tilt_db,
	    metrics.auto_eq_low_weight,
	    metrics.auto_eq_presence_weight,
	    metrics.auto_eq_high_weight,
	    metrics.bass_eq_recommend_valid ? "valid" : "invalid",
	    cp_bass_eq_preset_string(
	    (enum cp_bass_eq_preset)metrics.bass_eq_recommend_preset),
	    metrics.bass_eq_recommend_low_gain_db,
	    metrics.bass_eq_recommend_high_gain_db,
	    metrics.bass_eq_recommend_output_gain_db,
	    metrics.bass_eq_recommend_confidence,
	    pass ? "pass" : "fail");

	return pass;
}

static void
qr_update(struct qr_metrics *metrics, const cp_sample_t *input,
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
		input_abs = qr_abs(input[sample]);
		output_abs = qr_abs(output[sample]);
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
		metrics->input_left_square +=
		    (double)input[index] * (double)input[index];
		metrics->input_right_square +=
		    (double)input[index + 1] * (double)input[index + 1];
		metrics->output_left_square +=
		    (double)output[index] * (double)output[index];
		metrics->output_right_square +=
		    (double)output[index + 1] * (double)output[index + 1];

		input_mono = (input[index] + input[index + 1]) * 0.5f;
		output_mono = (output[index] + output[index + 1]) * 0.5f;
		phase50 = 2.0 * (double)QR_PI * 50.0 *
		    (double)(offset + frame) / (double)QR_RATE;
		phase60 = 2.0 * (double)QR_PI * 60.0 *
		    (double)(offset + frame) / (double)QR_RATE;

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

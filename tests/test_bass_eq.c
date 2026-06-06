/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_bass_eq.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "cp_bass_eq.h"

#define TEST_FRAMES	48000
#define TEST_RATE	48000.0f
#define TEST_TWO_PI	6.28318530717958647692f

static int		fill_sine(cp_sample_t *, size_t, size_t, cp_sample_t,
			    cp_sample_t);
static cp_sample_t	rms_tail(const cp_sample_t *, size_t, size_t,
			    size_t);
static int		test_bass_boost_and_cut(void);
static int		test_disabled_passthrough(void);
static int		test_invalid_config(void);
static int		test_midrange_conservative(void);
static int		test_non_finite_input(void);
static int		test_preset_parse(void);
static int		test_recommendation_bounds(void);
static int		test_recommendation_bright(void);
static int		test_recommendation_dark(void);
static int		test_recommendation_non_finite(void);
static int		test_recommendation_silence(void);
static int		test_recommendation_source_shapes(void);
static int		test_stereo_stable(void);

int
main(void)
{
	if (!test_disabled_passthrough())
		return 1;
	if (!test_bass_boost_and_cut())
		return 1;
	if (!test_midrange_conservative())
		return 1;
	if (!test_invalid_config())
		return 1;
	if (!test_preset_parse())
		return 1;
	if (!test_recommendation_silence())
		return 1;
	if (!test_recommendation_source_shapes())
		return 1;
	if (!test_recommendation_dark())
		return 1;
	if (!test_recommendation_bright())
		return 1;
	if (!test_recommendation_bounds())
		return 1;
	if (!test_recommendation_non_finite())
		return 1;
	if (!test_stereo_stable())
		return 1;
	if (!test_non_finite_input())
		return 1;

	return 0;
}

static int
fill_sine(cp_sample_t *buffer, size_t frames, size_t channels,
	cp_sample_t frequency, cp_sample_t level)
{
	cp_sample_t phase;
	cp_sample_t sample;
	size_t channel;
	size_t frame;

	if (buffer == NULL || frames == 0 || channels == 0)
		return 0;

	phase = 0.0f;
	for (frame = 0; frame < frames; frame++) {
		sample = sinf(phase) * level;
		for (channel = 0; channel < channels; channel++)
			buffer[(frame * channels) + channel] = sample;
		phase += (TEST_TWO_PI * frequency) / TEST_RATE;
		if (phase >= TEST_TWO_PI)
			phase -= TEST_TWO_PI;
	}

	return 1;
}

static cp_sample_t
rms_tail(const cp_sample_t *buffer, size_t frames, size_t channels,
	size_t channel)
{
	double sum;
	cp_sample_t sample;
	size_t count;
	size_t frame;
	size_t start;

	if (buffer == NULL || frames == 0 || channels == 0 ||
	    channel >= channels)
		return 0.0f;

	sum   = 0.0;
	count = 0;
	start = frames / 2;
	for (frame = start; frame < frames; frame++) {
		sample = buffer[(frame * channels) + channel];
		sum += (double)sample * (double)sample;
		count++;
	}
	if (count == 0)
		return 0.0f;

	return (cp_sample_t)sqrt(sum / (double)count);
}

static int
test_bass_boost_and_cut(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_bass_eq_config config;
	struct cp_bass_eq bass_eq;
	cp_sample_t input_rms;
	cp_sample_t output_rms;

	fill_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 80.0f, 0.2f);
	cp_bass_eq_default_config(&config);
	config.enabled = 1;
	config.low_gain_db = 3.0f;
	if (cp_bass_eq_init(&bass_eq, &config) != CP_OK)
		return 0;
	if (cp_bass_eq_process(&bass_eq, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	input_rms = rms_tail(input, TEST_FRAMES, CP_CHANNELS_MONO, 0);
	output_rms = rms_tail(output, TEST_FRAMES, CP_CHANNELS_MONO, 0);
	if (output_rms <= input_rms * 1.15f) {
		printf("test_bass_eq: bass boost too weak\n");
		return 0;
	}

	config.low_gain_db = -3.0f;
	if (cp_bass_eq_init(&bass_eq, &config) != CP_OK)
		return 0;
	if (cp_bass_eq_process(&bass_eq, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	output_rms = rms_tail(output, TEST_FRAMES, CP_CHANNELS_MONO, 0);
	if (output_rms >= input_rms * 0.92f) {
		printf("test_bass_eq: bass cut too weak\n");
		return 0;
	}

	return 1;
}

static int
test_disabled_passthrough(void)
{
	cp_sample_t input[512];
	cp_sample_t output[512];
	struct cp_bass_eq_config config;
	struct cp_bass_eq bass_eq;
	size_t index;

	fill_sine(input, 512, CP_CHANNELS_MONO, 1000.0f, 0.2f);
	cp_bass_eq_default_config(&config);
	if (cp_bass_eq_init(&bass_eq, &config) != CP_OK)
		return 0;
	if (cp_bass_eq_process(&bass_eq, input, output, 512) != CP_OK)
		return 0;
	for (index = 0; index < 512; index++) {
		if (fabsf(input[index] - output[index]) > 0.000001f) {
			printf("test_bass_eq: disabled path changed sample\n");
			return 0;
		}
	}

	return 1;
}

static int
test_invalid_config(void)
{
	struct cp_bass_eq_config config;
	struct cp_bass_eq bass_eq;

	cp_bass_eq_default_config(&config);
	config.channel_count = 3;
	if (cp_bass_eq_init(&bass_eq, &config) != CP_ERR_CHANNELS)
		return 0;

	cp_bass_eq_default_config(&config);
	config.low_shelf_hz = 10.0f;
	if (cp_bass_eq_init(&bass_eq, &config) != CP_ERR_RANGE)
		return 0;

	cp_bass_eq_default_config(&config);
	config.high_shelf_hz = 500.0f;
	if (cp_bass_eq_init(&bass_eq, &config) != CP_ERR_RANGE)
		return 0;

	cp_bass_eq_default_config(&config);
	config.low_gain_db = 12.0f;
	if (cp_bass_eq_init(&bass_eq, &config) != CP_ERR_RANGE)
		return 0;

	cp_bass_eq_default_config(&config);
	config.sample_rate = 8000.0f;
	config.high_shelf_hz = 7000.0f;
	if (cp_bass_eq_init(&bass_eq, &config) != CP_ERR_RANGE)
		return 0;

	return 1;
}

static int
test_midrange_conservative(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_bass_eq_config config;
	struct cp_bass_eq bass_eq;
	cp_sample_t input_rms;
	cp_sample_t output_rms;
	cp_sample_t ratio;

	fill_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 1000.0f, 0.2f);
	cp_bass_eq_default_config(&config);
	config.enabled = 1;
	config.low_gain_db = 3.0f;
	if (cp_bass_eq_init(&bass_eq, &config) != CP_OK)
		return 0;
	if (cp_bass_eq_process(&bass_eq, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;

	input_rms = rms_tail(input, TEST_FRAMES, CP_CHANNELS_MONO, 0);
	output_rms = rms_tail(output, TEST_FRAMES, CP_CHANNELS_MONO, 0);
	ratio = output_rms / input_rms;
	if (ratio < 0.80f || ratio > 1.25f) {
		printf("test_bass_eq: midrange ratio unsafe: %f\n", ratio);
		return 0;
	}

	return 1;
}

static int
test_non_finite_input(void)
{
	cp_sample_t input[8];
	cp_sample_t output[8];
	struct cp_bass_eq_config config;
	struct cp_bass_eq bass_eq;
	size_t index;

	input[0] = NAN;
	input[1] = INFINITY;
	input[2] = -INFINITY;
	input[3] = 0.25f;
	input[4] = -0.25f;
	input[5] = 0.0f;
	input[6] = 0.1f;
	input[7] = -0.1f;
	cp_bass_eq_default_config(&config);
	config.enabled = 1;
	if (cp_bass_eq_init(&bass_eq, &config) != CP_OK)
		return 0;
	if (cp_bass_eq_process(&bass_eq, input, output, 8) != CP_OK)
		return 0;
	for (index = 0; index < 8; index++) {
		if (!isfinite(output[index])) {
			printf("test_bass_eq: non-finite output\n");
			return 0;
		}
	}

	return 1;
}

static int
test_preset_parse(void)
{
	struct cp_bass_eq_config config;
	enum cp_bass_eq_preset preset;

	cp_bass_eq_default_config(&config);
	if (cp_bass_eq_apply_preset(&config, "music") != CP_OK ||
	    config.preset != CP_BASS_EQ_PRESET_MUSIC)
		return 0;
	if (cp_bass_eq_preset_from_string("warm", &preset) != CP_OK ||
	    preset != CP_BASS_EQ_PRESET_WARM)
		return 0;
	if (cp_bass_eq_apply_preset(&config, "bad") != CP_ERR_RANGE)
		return 0;

	return 1;
}

static int
test_recommendation_bounds(void)
{
	struct cp_auto_eq_metrics metrics;
	struct cp_bass_eq_recommendation recommendation;

	(void)memset(&metrics, 0, sizeof(metrics));
	metrics.finite = 1;
	metrics.total_rms = 0.1f;
	metrics.source_hint = CP_AUTO_EQ_SOURCE_THIN;
	metrics.low_frequency_weight = 0.0f;
	metrics.presence_weight = 0.2f;
	metrics.high_frequency_weight = 1.0f;
	metrics.spectral_tilt_db = 30.0f;
	if (cp_bass_eq_recommend(&metrics, &recommendation) != CP_OK)
		return 0;
	if (!recommendation.valid)
		return 0;
	if (recommendation.low_gain_db < -3.0f ||
	    recommendation.low_gain_db > 3.0f ||
	    recommendation.high_gain_db < -3.0f ||
	    recommendation.high_gain_db > 3.0f ||
	    recommendation.output_gain_db < -3.0f ||
	    recommendation.output_gain_db > 0.0f ||
	    recommendation.confidence < 0.0f ||
	    recommendation.confidence > 1.0f) {
		printf("test_bass_eq: recommendation exceeded bounds\n");
		return 0;
	}

	return 1;
}

static int
test_recommendation_bright(void)
{
	struct cp_auto_eq_metrics metrics;
	struct cp_bass_eq_recommendation recommendation;

	(void)memset(&metrics, 0, sizeof(metrics));
	metrics.finite = 1;
	metrics.total_rms = 0.1f;
	metrics.source_hint = CP_AUTO_EQ_SOURCE_BRIGHT;
	metrics.low_frequency_weight = 0.25f;
	metrics.presence_weight = 0.25f;
	metrics.high_frequency_weight = 0.55f;
	metrics.spectral_tilt_db = 12.0f;
	if (cp_bass_eq_recommend(&metrics, &recommendation) != CP_OK)
		return 0;
	if (!recommendation.valid || recommendation.high_gain_db >= 0.0f) {
		printf("test_bass_eq: bright recommendation unsafe\n");
		return 0;
	}

	return 1;
}

static int
test_recommendation_dark(void)
{
	struct cp_auto_eq_metrics metrics;
	struct cp_bass_eq_recommendation recommendation;

	(void)memset(&metrics, 0, sizeof(metrics));
	metrics.finite = 1;
	metrics.total_rms = 0.1f;
	metrics.source_hint = CP_AUTO_EQ_SOURCE_DARK;
	metrics.low_frequency_weight = 0.45f;
	metrics.presence_weight = 0.06f;
	metrics.high_frequency_weight = 0.02f;
	metrics.spectral_tilt_db = -20.0f;
	if (cp_bass_eq_recommend(&metrics, &recommendation) != CP_OK)
		return 0;
	if (!recommendation.valid || recommendation.high_gain_db <= 0.0f) {
		printf("test_bass_eq: dark recommendation unsafe\n");
		return 0;
	}

	return 1;
}

static int
test_recommendation_non_finite(void)
{
	struct cp_auto_eq_metrics metrics;
	struct cp_bass_eq_recommendation recommendation;

	(void)memset(&metrics, 0, sizeof(metrics));
	metrics.finite = 0;
	metrics.total_rms = NAN;
	metrics.source_hint = CP_AUTO_EQ_SOURCE_THIN;
	metrics.low_frequency_weight = NAN;
	metrics.presence_weight = 0.0f;
	metrics.high_frequency_weight = 0.0f;
	metrics.spectral_tilt_db = NAN;
	if (cp_bass_eq_recommend(&metrics, &recommendation) != CP_OK)
		return 0;
	if (recommendation.valid || recommendation.confidence != 0.0f) {
		printf("test_bass_eq: non-finite recommendation was valid\n");
		return 0;
	}

	return 1;
}

static int
test_recommendation_silence(void)
{
	struct cp_auto_eq_metrics metrics;
	struct cp_bass_eq_recommendation recommendation;

	(void)memset(&metrics, 0, sizeof(metrics));
	metrics.finite = 1;
	metrics.source_hint = CP_AUTO_EQ_SOURCE_SILENCE;
	if (cp_bass_eq_recommend(&metrics, &recommendation) != CP_OK)
		return 0;
	if (recommendation.valid || recommendation.confidence != 0.0f) {
		printf("test_bass_eq: silence recommendation was valid\n");
		return 0;
	}

	return 1;
}

static int
test_recommendation_source_shapes(void)
{
	struct cp_auto_eq_metrics metrics;
	struct cp_bass_eq_recommendation recommendation;

	(void)memset(&metrics, 0, sizeof(metrics));
	metrics.finite = 1;
	metrics.total_rms = 0.1f;
	metrics.source_hint = CP_AUTO_EQ_SOURCE_BASS_HEAVY;
	metrics.low_frequency_weight = 0.70f;
	metrics.presence_weight = 0.1f;
	metrics.high_frequency_weight = 0.03f;
	metrics.spectral_tilt_db = -18.0f;
	if (cp_bass_eq_recommend(&metrics, &recommendation) != CP_OK)
		return 0;
	if (!recommendation.valid || recommendation.low_gain_db >= 0.0f) {
		printf("test_bass_eq: bass-heavy recommendation unsafe\n");
		return 0;
	}

	metrics.source_hint = CP_AUTO_EQ_SOURCE_THIN;
	metrics.low_frequency_weight = 0.05f;
	metrics.high_frequency_weight = 0.20f;
	metrics.spectral_tilt_db = 8.0f;
	if (cp_bass_eq_recommend(&metrics, &recommendation) != CP_OK)
		return 0;
	if (!recommendation.valid || recommendation.low_gain_db <= 0.0f) {
		printf("test_bass_eq: thin recommendation unsafe\n");
		return 0;
	}

	return 1;
}

static int
test_stereo_stable(void)
{
	cp_sample_t input[TEST_FRAMES * CP_CHANNELS_STEREO];
	cp_sample_t output[TEST_FRAMES * CP_CHANNELS_STEREO];
	struct cp_bass_eq_config config;
	struct cp_bass_eq bass_eq;
	cp_sample_t left_rms;
	cp_sample_t right_rms;
	size_t index;

	fill_sine(input, TEST_FRAMES, CP_CHANNELS_STEREO, 120.0f, 0.2f);
	cp_bass_eq_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_STEREO;
	config.low_gain_db = 2.0f;
	if (cp_bass_eq_init(&bass_eq, &config) != CP_OK)
		return 0;
	if (cp_bass_eq_process(&bass_eq, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	for (index = 0; index < TEST_FRAMES * CP_CHANNELS_STEREO; index++) {
		if (!isfinite(output[index])) {
			printf("test_bass_eq: stereo non-finite output\n");
			return 0;
		}
	}
	left_rms = rms_tail(output, TEST_FRAMES, CP_CHANNELS_STEREO, 0);
	right_rms = rms_tail(output, TEST_FRAMES, CP_CHANNELS_STEREO, 1);
	if (fabsf(left_rms - right_rms) > 0.0001f) {
		printf("test_bass_eq: stereo channels diverged\n");
		return 0;
	}

	return 1;
}

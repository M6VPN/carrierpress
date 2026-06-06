/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_restoration.c */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "cp_restoration.h"

#define TEST_FRAMES		4096u
#define TEST_RATE		48000.0f
#define TEST_TWO_PI		6.28318530717958647692f
#define TEST_CLIP_THRESHOLD	0.95f

static void	fill_clipped_sine(cp_sample_t *, size_t, size_t);
static void	fill_low_ceiling_clipped_sine(cp_sample_t *, size_t, size_t);
static void	fill_sine(cp_sample_t *, size_t, size_t, cp_sample_t,
		    cp_sample_t);
static int	run_case(const char *, int (*)(void));
static int	test_burst_not_high_confidence(void);
static int	test_clean_sine_not_clipped(void);
static int	test_clipped_sine_detected(void);
static int	test_disabled_metrics_stay_clear(void);
static int	test_high_frequency_loss_indicator(void);
static int	test_invalid_config_rejected(void);
static int	test_low_ceiling_clipping_detected(void);
static int	test_nonfinite_input_marked(void);
static int	test_source_profiles(void);
static int	test_stereo_processing_stable(void);

static void
fill_clipped_sine(cp_sample_t *buffer, size_t frames, size_t channels)
{
	cp_sample_t phase;
	cp_sample_t sample;
	cp_sample_t step;
	size_t channel;
	size_t frame;

	phase = 0.0f;
	step = (TEST_TWO_PI * 1000.0f) / TEST_RATE;
	for (frame = 0; frame < frames; frame++) {
		sample = sinf(phase) * 1.4f;
		if (sample > TEST_CLIP_THRESHOLD)
			sample = TEST_CLIP_THRESHOLD;
		if (sample < -TEST_CLIP_THRESHOLD)
			sample = -TEST_CLIP_THRESHOLD;
		for (channel = 0; channel < channels; channel++)
			buffer[(frame * channels) + channel] = sample;
		phase += step;
		if (phase >= TEST_TWO_PI)
			phase -= TEST_TWO_PI;
	}
}

static void
fill_low_ceiling_clipped_sine(cp_sample_t *buffer, size_t frames,
	size_t channels)
{
	cp_sample_t phase;
	cp_sample_t sample;
	cp_sample_t step;
	size_t channel;
	size_t frame;

	phase = 0.0f;
	step = (TEST_TWO_PI * 1000.0f) / TEST_RATE;
	for (frame = 0; frame < frames; frame++) {
		sample = sinf(phase) * 0.90f;
		if (sample > 0.45f)
			sample = 0.45f;
		if (sample < -0.45f)
			sample = -0.45f;
		for (channel = 0; channel < channels; channel++)
			buffer[(frame * channels) + channel] = sample;
		phase += step;
		if (phase >= TEST_TWO_PI)
			phase -= TEST_TWO_PI;
	}
}

static void
fill_sine(cp_sample_t *buffer, size_t frames, size_t channels,
	cp_sample_t frequency, cp_sample_t level)
{
	cp_sample_t phase;
	cp_sample_t sample;
	cp_sample_t step;
	size_t channel;
	size_t frame;

	phase = 0.0f;
	step = (TEST_TWO_PI * frequency) / TEST_RATE;
	for (frame = 0; frame < frames; frame++) {
		sample = sinf(phase) * level;
		for (channel = 0; channel < channels; channel++)
			buffer[(frame * channels) + channel] = sample;
		phase += step;
		if (phase >= TEST_TWO_PI)
			phase -= TEST_TWO_PI;
	}
}

static int
run_case(const char *name, int (*test)(void))
{
	if (!test()) {
		printf("%s failed\n", name);
		return 0;
	}

	return 1;
}

static int
test_burst_not_high_confidence(void)
{
	struct cp_restoration_config config;
	struct cp_restoration restoration;
	cp_sample_t buffer[TEST_FRAMES];
	size_t frame;

	cp_restoration_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	config.sample_rate = TEST_RATE;
	config.clip_threshold = TEST_CLIP_THRESHOLD;
	config.analysis_window_frames = TEST_FRAMES;
	if (cp_restoration_init(&restoration, &config) != CP_OK)
		return 0;

	memset(buffer, 0, sizeof(buffer));
	for (frame = 100; frame < 104; frame++)
		buffer[frame] = TEST_CLIP_THRESHOLD;

	if (cp_restoration_process(&restoration, buffer, TEST_FRAMES) != CP_OK)
		return 0;
	if (restoration.metrics.clipping_confidence >= 0.40f)
		return 0;
	if (restoration.metrics.transient_confidence <= 0.50f)
		return 0;
	if ((restoration.metrics.reason_flags &
	    CP_RESTORATION_REASON_TRANSIENT) == 0)
		return 0;

	return 1;
}

static int
test_clean_sine_not_clipped(void)
{
	struct cp_restoration_config config;
	struct cp_restoration restoration;
	cp_sample_t buffer[TEST_FRAMES];

	cp_restoration_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	config.sample_rate = TEST_RATE;
	config.clip_threshold = TEST_CLIP_THRESHOLD;
	config.analysis_window_frames = TEST_FRAMES;
	if (cp_restoration_init(&restoration, &config) != CP_OK)
		return 0;

	fill_sine(buffer, TEST_FRAMES, CP_CHANNELS_MONO, 1000.0f, 0.40f);
	if (cp_restoration_process(&restoration, buffer, TEST_FRAMES) != CP_OK)
		return 0;
	if (restoration.metrics.clipped_sample_ratio != 0.0f)
		return 0;
	if (restoration.metrics.clipping_confidence > 0.05f)
		return 0;

	return 1;
}

static int
test_clipped_sine_detected(void)
{
	struct cp_restoration_config config;
	struct cp_restoration restoration;
	cp_sample_t buffer[TEST_FRAMES];

	cp_restoration_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	config.sample_rate = TEST_RATE;
	config.clip_threshold = TEST_CLIP_THRESHOLD;
	config.analysis_window_frames = TEST_FRAMES;
	if (cp_restoration_init(&restoration, &config) != CP_OK)
		return 0;

	fill_clipped_sine(buffer, TEST_FRAMES, CP_CHANNELS_MONO);
	if (cp_restoration_process(&restoration, buffer, TEST_FRAMES) != CP_OK)
		return 0;
	if (restoration.metrics.clipped_sample_ratio <= 0.10f)
		return 0;
	if (restoration.metrics.flat_run_count == 0)
		return 0;
	if (restoration.metrics.clipping_confidence <= 0.50f)
		return 0;
	if ((restoration.metrics.reason_flags &
	    CP_RESTORATION_REASON_HARD_CLIP) == 0)
		return 0;

	return 1;
}

static int
test_disabled_metrics_stay_clear(void)
{
	struct cp_restoration_config config;
	struct cp_restoration restoration;
	cp_sample_t buffer[TEST_FRAMES];

	cp_restoration_default_config(&config);
	config.enabled = 0;
	config.channel_count = CP_CHANNELS_MONO;
	config.sample_rate = TEST_RATE;
	if (cp_restoration_init(&restoration, &config) != CP_OK)
		return 0;

	fill_clipped_sine(buffer, TEST_FRAMES, CP_CHANNELS_MONO);
	if (cp_restoration_process(&restoration, buffer, TEST_FRAMES) != CP_OK)
		return 0;
	if (restoration.metrics.total_sample_count != 0)
		return 0;
	if (restoration.metrics.clipping_confidence != 0.0f)
		return 0;

	return 1;
}

static int
test_high_frequency_loss_indicator(void)
{
	struct cp_restoration_config config;
	struct cp_restoration high_loss;
	struct cp_restoration wideband;
	cp_sample_t low_buffer[TEST_FRAMES];
	cp_sample_t high_buffer[TEST_FRAMES];

	cp_restoration_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	config.sample_rate = TEST_RATE;
	config.analysis_window_frames = TEST_FRAMES;
	if (cp_restoration_init(&high_loss, &config) != CP_OK)
		return 0;
	if (cp_restoration_init(&wideband, &config) != CP_OK)
		return 0;

	fill_sine(low_buffer, TEST_FRAMES, CP_CHANNELS_MONO, 300.0f, 0.40f);
	fill_sine(high_buffer, TEST_FRAMES, CP_CHANNELS_MONO, 9000.0f, 0.40f);
	if (cp_restoration_process(&high_loss, low_buffer,
	    TEST_FRAMES) != CP_OK)
		return 0;
	if (cp_restoration_process(&wideband, high_buffer,
	    TEST_FRAMES) != CP_OK)
		return 0;
	if (high_loss.metrics.lossy_confidence <=
	    wideband.metrics.lossy_confidence)
		return 0;
	if (high_loss.metrics.high_frequency_ratio >=
	    wideband.metrics.high_frequency_ratio)
		return 0;

	return 1;
}

static int
test_invalid_config_rejected(void)
{
	struct cp_restoration_config config;
	struct cp_restoration restoration;

	cp_restoration_default_config(&config);
	config.enabled = 1;
	config.channel_count = 3;
	if (cp_restoration_init(&restoration, &config) != CP_ERR_CHANNELS)
		return 0;

	cp_restoration_default_config(&config);
	config.enabled = 1;
	config.sample_rate = 1000.0f;
	if (cp_restoration_init(&restoration, &config) != CP_ERR_RANGE)
		return 0;

	cp_restoration_default_config(&config);
	config.enabled = 1;
	config.clip_threshold = 1.5f;
	if (cp_restoration_init(&restoration, &config) != CP_ERR_RANGE)
		return 0;

	return 1;
}

static int
test_low_ceiling_clipping_detected(void)
{
	struct cp_restoration_config config;
	struct cp_restoration restoration;
	cp_sample_t buffer[TEST_FRAMES];

	cp_restoration_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	config.sample_rate = TEST_RATE;
	config.clip_threshold = TEST_CLIP_THRESHOLD;
	config.analysis_window_frames = TEST_FRAMES;
	if (cp_restoration_init(&restoration, &config) != CP_OK)
		return 0;

	fill_low_ceiling_clipped_sine(buffer, TEST_FRAMES, CP_CHANNELS_MONO);
	if (cp_restoration_process(&restoration, buffer, TEST_FRAMES) != CP_OK)
		return 0;
	if (restoration.metrics.clipping_confidence >= 0.20f)
		return 0;
	if (restoration.metrics.low_ceiling_clipping_confidence <= 0.50f)
		return 0;
	if ((restoration.metrics.reason_flags &
	    CP_RESTORATION_REASON_LOW_CEILING) == 0)
		return 0;

	return 1;
}

static int
test_nonfinite_input_marked(void)
{
	struct cp_restoration_config config;
	struct cp_restoration restoration;
	cp_sample_t buffer[TEST_FRAMES];

	cp_restoration_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	config.sample_rate = TEST_RATE;
	config.analysis_window_frames = TEST_FRAMES;
	if (cp_restoration_init(&restoration, &config) != CP_OK)
		return 0;

	memset(buffer, 0, sizeof(buffer));
	buffer[100] = INFINITY;
	buffer[101] = NAN;
	if (cp_restoration_process(&restoration, buffer, TEST_FRAMES) != CP_OK)
		return 0;
	if (restoration.metrics.finite != 0)
		return 0;
	if (!isfinite(restoration.metrics.clipping_confidence))
		return 0;
	if (!isfinite(restoration.metrics.lossy_confidence))
		return 0;
	if ((restoration.metrics.reason_flags &
	    CP_RESTORATION_REASON_NONFINITE) == 0)
		return 0;

	return 1;
}

static int
test_source_profiles(void)
{
	struct cp_restoration_config config;
	struct cp_restoration am_limited;
	struct cp_restoration hf_loss;
	struct cp_restoration ssb_voice;
	struct cp_restoration wideband;
	cp_sample_t buffer[TEST_FRAMES];

	cp_restoration_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	config.sample_rate = TEST_RATE;
	config.analysis_window_frames = TEST_FRAMES;
	if (cp_restoration_init(&wideband, &config) != CP_OK)
		return 0;
	if (cp_restoration_init(&am_limited, &config) != CP_OK)
		return 0;
	if (cp_restoration_init(&ssb_voice, &config) != CP_OK)
		return 0;
	if (cp_restoration_init(&hf_loss, &config) != CP_OK)
		return 0;

	fill_sine(buffer, TEST_FRAMES, CP_CHANNELS_MONO, 9000.0f, 0.30f);
	if (cp_restoration_process(&wideband, buffer, TEST_FRAMES) != CP_OK)
		return 0;
	fill_sine(buffer, TEST_FRAMES, CP_CHANNELS_MONO, 2500.0f, 0.30f);
	if (cp_restoration_process(&am_limited, buffer, TEST_FRAMES) != CP_OK)
		return 0;
	fill_sine(buffer, TEST_FRAMES, CP_CHANNELS_MONO, 900.0f, 0.30f);
	if (cp_restoration_process(&ssb_voice, buffer, TEST_FRAMES) != CP_OK)
		return 0;
	fill_sine(buffer, TEST_FRAMES, CP_CHANNELS_MONO, 300.0f, 0.30f);
	if (cp_restoration_process(&hf_loss, buffer, TEST_FRAMES) != CP_OK)
		return 0;

	if (wideband.metrics.source_profile != CP_RESTORATION_SOURCE_WIDEBAND)
		return 0;
	if (am_limited.metrics.source_profile !=
	    CP_RESTORATION_SOURCE_AM_LIMITED)
		return 0;
	if (ssb_voice.metrics.source_profile !=
	    CP_RESTORATION_SOURCE_SSB_VOICE)
		return 0;
	if (hf_loss.metrics.source_profile !=
	    CP_RESTORATION_SOURCE_HIGH_FREQUENCY_LOSS)
		return 0;

	return 1;
}

static int
test_stereo_processing_stable(void)
{
	struct cp_restoration_config config;
	struct cp_restoration restoration;
	cp_sample_t buffer[TEST_FRAMES * CP_CHANNELS_STEREO];

	cp_restoration_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_STEREO;
	config.sample_rate = TEST_RATE;
	config.analysis_window_frames = TEST_FRAMES;
	if (cp_restoration_init(&restoration, &config) != CP_OK)
		return 0;

	fill_sine(buffer, TEST_FRAMES, CP_CHANNELS_STEREO, 1000.0f, 0.40f);
	if (cp_restoration_process(&restoration, buffer, TEST_FRAMES) != CP_OK)
		return 0;
	if (!isfinite(restoration.metrics.high_frequency_ratio))
		return 0;
	if (restoration.metrics.total_sample_count !=
	    TEST_FRAMES * CP_CHANNELS_STEREO)
		return 0;

	return 1;
}

int
main(void)
{
	if (!run_case("clean_sine_not_clipped", test_clean_sine_not_clipped))
		return 1;
	if (!run_case("clipped_sine_detected", test_clipped_sine_detected))
		return 1;
	if (!run_case("low_ceiling_clipping_detected",
	    test_low_ceiling_clipping_detected))
		return 1;
	if (!run_case("burst_not_high_confidence",
	    test_burst_not_high_confidence))
		return 1;
	if (!run_case("high_frequency_loss_indicator",
	    test_high_frequency_loss_indicator))
		return 1;
	if (!run_case("source_profiles", test_source_profiles))
		return 1;
	if (!run_case("disabled_metrics_stay_clear",
	    test_disabled_metrics_stay_clear))
		return 1;
	if (!run_case("invalid_config_rejected", test_invalid_config_rejected))
		return 1;
	if (!run_case("nonfinite_input_marked", test_nonfinite_input_marked))
		return 1;
	if (!run_case("stereo_processing_stable", test_stereo_processing_stable))
		return 1;

	return 0;
}

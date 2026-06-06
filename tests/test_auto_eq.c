/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_auto_eq.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_auto_eq.h"

#define TEST_FRAMES	8192
#define TEST_RATE	48000.0f
#define TEST_TWO_PI	6.28318530717958647692f

static void	add_sine(cp_sample_t *, size_t, size_t, cp_sample_t,
		    cp_sample_t);
static void	clear_buffer(cp_sample_t *, size_t);
static int	run_analysis(struct cp_auto_eq *, const cp_sample_t *,
		    size_t);
static int	test_bass_heavy_hint(void);
static int	test_bright_hint(void);
static int	test_disabled(void);
static int	test_invalid_config(void);
static int	test_limited_band_hint(void);
static int	test_non_finite_input(void);
static int	test_silence_hint(void);
static int	test_stereo_stable(void);
static int	test_thin_hint(void);

int
main(void)
{
	if (!test_disabled())
		return 1;
	if (!test_invalid_config())
		return 1;
	if (!test_silence_hint())
		return 1;
	if (!test_bass_heavy_hint())
		return 1;
	if (!test_thin_hint())
		return 1;
	if (!test_bright_hint())
		return 1;
	if (!test_limited_band_hint())
		return 1;
	if (!test_stereo_stable())
		return 1;
	if (!test_non_finite_input())
		return 1;

	return 0;
}

static void
add_sine(cp_sample_t *buffer, size_t frames, size_t channels,
	cp_sample_t frequency, cp_sample_t level)
{
	cp_sample_t phase;
	cp_sample_t sample;
	size_t channel;
	size_t frame;

	phase = 0.0f;
	for (frame = 0; frame < frames; frame++) {
		sample = sinf(phase) * level;
		for (channel = 0; channel < channels; channel++)
			buffer[(frame * channels) + channel] += sample;
		phase += (TEST_TWO_PI * frequency) / TEST_RATE;
		if (phase >= TEST_TWO_PI)
			phase -= TEST_TWO_PI;
	}
}

static void
clear_buffer(cp_sample_t *buffer, size_t samples)
{
	size_t index;

	for (index = 0; index < samples; index++)
		buffer[index] = 0.0f;
}

static int
run_analysis(struct cp_auto_eq *auto_eq, const cp_sample_t *input,
	size_t frames)
{
	size_t offset;
	size_t block;

	for (offset = 0; offset < frames; offset += block) {
		block = frames - offset;
		if (block > 256)
			block = 256;
		if (cp_auto_eq_process(auto_eq,
		    input + (offset * auto_eq->config.channel_count),
		    block) != CP_OK)
			return 0;
	}

	return 1;
}

static int
test_bass_heavy_hint(void)
{
	cp_sample_t input[TEST_FRAMES];
	struct cp_auto_eq_config config;
	struct cp_auto_eq auto_eq;

	clear_buffer(input, TEST_FRAMES);
	add_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 120.0f, 0.40f);
	add_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 1200.0f, 0.05f);
	cp_auto_eq_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	if (cp_auto_eq_init(&auto_eq, &config) != CP_OK)
		return 0;
	if (!run_analysis(&auto_eq, input, TEST_FRAMES))
		return 0;
	if (auto_eq.metrics.source_hint != CP_AUTO_EQ_SOURCE_BASS_HEAVY) {
		printf("test_auto_eq: bass-heavy hint failed: %s\n",
		    cp_auto_eq_source_hint_string(auto_eq.metrics.source_hint));
		return 0;
	}

	return 1;
}

static int
test_bright_hint(void)
{
	cp_sample_t input[TEST_FRAMES];
	struct cp_auto_eq_config config;
	struct cp_auto_eq auto_eq;

	clear_buffer(input, TEST_FRAMES);
	add_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 6000.0f, 0.30f);
	add_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 1500.0f, 0.05f);
	cp_auto_eq_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	if (cp_auto_eq_init(&auto_eq, &config) != CP_OK)
		return 0;
	if (!run_analysis(&auto_eq, input, TEST_FRAMES))
		return 0;
	if (auto_eq.metrics.source_hint != CP_AUTO_EQ_SOURCE_BRIGHT &&
	    auto_eq.metrics.source_hint != CP_AUTO_EQ_SOURCE_THIN) {
		printf("test_auto_eq: bright hint failed: %s\n",
		    cp_auto_eq_source_hint_string(auto_eq.metrics.source_hint));
		return 0;
	}

	return 1;
}

static int
test_disabled(void)
{
	cp_sample_t input[256];
	struct cp_auto_eq_config config;
	struct cp_auto_eq auto_eq;

	clear_buffer(input, 256);
	add_sine(input, 256, CP_CHANNELS_MONO, 1000.0f, 0.20f);
	cp_auto_eq_default_config(&config);
	config.channel_count = CP_CHANNELS_MONO;
	if (cp_auto_eq_init(&auto_eq, &config) != CP_OK)
		return 0;
	if (cp_auto_eq_process(&auto_eq, input, 256) != CP_OK)
		return 0;
	if (auto_eq.metrics.total_rms != 0.0f ||
	    auto_eq.metrics.source_hint != CP_AUTO_EQ_SOURCE_UNKNOWN)
		return 0;

	return 1;
}

static int
test_invalid_config(void)
{
	struct cp_auto_eq_config config;
	struct cp_auto_eq auto_eq;

	cp_auto_eq_default_config(&config);
	config.channel_count = 3;
	if (cp_auto_eq_init(&auto_eq, &config) != CP_ERR_CHANNELS)
		return 0;

	cp_auto_eq_default_config(&config);
	config.sample_rate = 4000.0f;
	if (cp_auto_eq_init(&auto_eq, &config) != CP_ERR_RANGE)
		return 0;

	cp_auto_eq_default_config(&config);
	config.analysis_window_frames = CP_AUTO_EQ_MIN_WINDOW_FRAMES - 1;
	if (cp_auto_eq_init(&auto_eq, &config) != CP_ERR_RANGE)
		return 0;

	return 1;
}

static int
test_limited_band_hint(void)
{
	cp_sample_t input[TEST_FRAMES];
	struct cp_auto_eq_config config;
	struct cp_auto_eq auto_eq;

	clear_buffer(input, TEST_FRAMES);
	add_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 1200.0f, 0.30f);
	cp_auto_eq_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	if (cp_auto_eq_init(&auto_eq, &config) != CP_OK)
		return 0;
	if (!run_analysis(&auto_eq, input, TEST_FRAMES))
		return 0;
	if (auto_eq.metrics.source_hint != CP_AUTO_EQ_SOURCE_LIMITED_BAND &&
	    auto_eq.metrics.source_hint != CP_AUTO_EQ_SOURCE_BALANCED) {
		printf("test_auto_eq: limited hint failed: %s\n",
		    cp_auto_eq_source_hint_string(auto_eq.metrics.source_hint));
		return 0;
	}

	return 1;
}

static int
test_non_finite_input(void)
{
	cp_sample_t input[TEST_FRAMES];
	struct cp_auto_eq_config config;
	struct cp_auto_eq auto_eq;

	clear_buffer(input, TEST_FRAMES);
	add_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 1000.0f, 0.20f);
	input[4] = NAN;
	input[8] = INFINITY;
	cp_auto_eq_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	if (cp_auto_eq_init(&auto_eq, &config) != CP_OK)
		return 0;
	if (!run_analysis(&auto_eq, input, TEST_FRAMES))
		return 0;
	if (auto_eq.metrics.finite) {
		printf("test_auto_eq: non-finite input not flagged\n");
		return 0;
	}
	if (!isfinite(auto_eq.metrics.total_rms) ||
	    !isfinite(auto_eq.metrics.spectral_tilt_db))
		return 0;

	return 1;
}

static int
test_silence_hint(void)
{
	cp_sample_t input[TEST_FRAMES];
	struct cp_auto_eq_config config;
	struct cp_auto_eq auto_eq;

	clear_buffer(input, TEST_FRAMES);
	cp_auto_eq_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	if (cp_auto_eq_init(&auto_eq, &config) != CP_OK)
		return 0;
	if (!run_analysis(&auto_eq, input, TEST_FRAMES))
		return 0;
	if (auto_eq.metrics.source_hint != CP_AUTO_EQ_SOURCE_SILENCE ||
	    auto_eq.metrics.total_rms != 0.0f) {
		printf("test_auto_eq: silence hint failed\n");
		return 0;
	}

	return 1;
}

static int
test_stereo_stable(void)
{
	cp_sample_t input[TEST_FRAMES * CP_CHANNELS_STEREO];
	struct cp_auto_eq_config config;
	struct cp_auto_eq auto_eq;

	clear_buffer(input, TEST_FRAMES * CP_CHANNELS_STEREO);
	add_sine(input, TEST_FRAMES, CP_CHANNELS_STEREO, 1000.0f, 0.20f);
	cp_auto_eq_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_STEREO;
	if (cp_auto_eq_init(&auto_eq, &config) != CP_OK)
		return 0;
	if (!run_analysis(&auto_eq, input, TEST_FRAMES))
		return 0;
	if (!isfinite(auto_eq.metrics.total_rms) ||
	    auto_eq.metrics.total_rms <= 0.0f)
		return 0;

	return 1;
}

static int
test_thin_hint(void)
{
	cp_sample_t input[TEST_FRAMES];
	struct cp_auto_eq_config config;
	struct cp_auto_eq auto_eq;

	clear_buffer(input, TEST_FRAMES);
	add_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 3500.0f, 0.30f);
	add_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 120.0f, 0.01f);
	cp_auto_eq_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	if (cp_auto_eq_init(&auto_eq, &config) != CP_OK)
		return 0;
	if (!run_analysis(&auto_eq, input, TEST_FRAMES))
		return 0;
	if (auto_eq.metrics.source_hint != CP_AUTO_EQ_SOURCE_THIN &&
	    auto_eq.metrics.source_hint != CP_AUTO_EQ_SOURCE_BRIGHT) {
		printf("test_auto_eq: thin hint failed: %s\n",
		    cp_auto_eq_source_hint_string(auto_eq.metrics.source_hint));
		return 0;
	}

	return 1;
}

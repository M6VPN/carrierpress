/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_am.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_am.h"

#define TEST_FRAMES		48000
#define TEST_RATE		48000.0f
#define TEST_TWO_PI		6.28318530717958647692f

static cp_sample_t input[TEST_FRAMES * CP_MAX_CHANNELS];
static cp_sample_t output[TEST_FRAMES * CP_MAX_CHANNELS];

static void		fill_sine(size_t, size_t, cp_sample_t, cp_sample_t);
static cp_sample_t	measure_peak(const cp_sample_t *, size_t, size_t,
			    size_t);
static cp_sample_t	measure_rms(const cp_sample_t *, size_t, size_t,
			    size_t);
static int		run_am(struct cp_am_config *, size_t, size_t);
static void		test_config(struct cp_am_config *, size_t, int);
static int		test_asymmetry_changes_positive_peak(void);
static int		test_disabled_unchanged(void);
static int		test_finite_large_input(void);
static int		test_highpass_reduces_low_frequency(void);
static int		test_invalid_filter_settings(void);
static int		test_lowpass_reduces_high_frequency(void);
static int		test_negative_peak_limit(void);
static int		test_positive_peak_limit(void);
static int		test_stereo_stable(void);

int
main(void)
{
	if (!test_disabled_unchanged())
		return 1;
	if (!test_lowpass_reduces_high_frequency())
		return 1;
	if (!test_highpass_reduces_low_frequency())
		return 1;
	if (!test_invalid_filter_settings())
		return 1;
	if (!test_negative_peak_limit())
		return 1;
	if (!test_positive_peak_limit())
		return 1;
	if (!test_asymmetry_changes_positive_peak())
		return 1;
	if (!test_finite_large_input())
		return 1;
	if (!test_stereo_stable())
		return 1;

	return 0;
}

static void
fill_sine(size_t frames, size_t channels, cp_sample_t frequency,
	cp_sample_t level)
{
	cp_sample_t phase;
	cp_sample_t sample;
	size_t channel;
	size_t frame;

	phase = 0.0f;
	for (frame = 0; frame < frames; frame++) {
		sample = sinf(phase) * level;
		for (channel = 0; channel < channels; channel++)
			input[(frame * channels) + channel] = sample;
		phase += (TEST_TWO_PI * frequency) / TEST_RATE;
		if (phase >= TEST_TWO_PI)
			phase -= TEST_TWO_PI;
	}
}

static cp_sample_t
measure_peak(const cp_sample_t *buffer, size_t frames, size_t channels,
	size_t channel)
{
	cp_sample_t peak;
	cp_sample_t sample;
	size_t frame;

	peak = 0.0f;
	for (frame = frames / 2; frame < frames; frame++) {
		sample = fabsf(buffer[(frame * channels) + channel]);
		if (sample > peak)
			peak = sample;
	}

	return peak;
}

static cp_sample_t
measure_rms(const cp_sample_t *buffer, size_t frames, size_t channels,
	size_t channel)
{
	cp_sample_t sample;
	cp_sample_t sum;
	size_t frame;

	sum = 0.0f;
	for (frame = frames / 2; frame < frames; frame++) {
		sample = buffer[(frame * channels) + channel];
		sum += sample * sample;
	}

	return sqrtf(sum / (cp_sample_t)(frames - (frames / 2)));
}

static int
run_am(struct cp_am_config *config, size_t frames, size_t channels)
{
	struct cp_am am;

	config->channel_count = channels;
	config->sample_rate   = TEST_RATE;
	if (cp_am_init(&am, config) != CP_OK)
		return 0;
	if (cp_am_process(&am, input, output, frames) != CP_OK)
		return 0;

	return 1;
}

static void
test_config(struct cp_am_config *config, size_t channels, int enabled)
{
	cp_am_default_config(config);
	config->enabled       = enabled;
	config->channel_count = channels;
	config->sample_rate   = TEST_RATE;
}

static int
test_asymmetry_changes_positive_peak(void)
{
	struct cp_am_config config;
	cp_sample_t asym_peak;
	cp_sample_t normal_peak;

	fill_sine(TEST_FRAMES, CP_CHANNELS_MONO, 1000.0f, 0.40f);
	test_config(&config, CP_CHANNELS_MONO, 1);
	config.phase_rotator_enabled = 0;
	config.positive_peak_limit   = 1.0f;
	if (!run_am(&config, TEST_FRAMES, CP_CHANNELS_MONO))
		return 0;
	normal_peak = measure_peak(output, TEST_FRAMES, CP_CHANNELS_MONO, 0);

	test_config(&config, CP_CHANNELS_MONO, 1);
	config.phase_rotator_enabled = 0;
	config.positive_peak_limit   = 2.0f;
	config.asymmetry_enabled     = 1;
	config.asymmetry_ratio       = 2.0f;
	if (!run_am(&config, TEST_FRAMES, CP_CHANNELS_MONO))
		return 0;
	asym_peak = measure_peak(output, TEST_FRAMES, CP_CHANNELS_MONO, 0);

	if (asym_peak <= normal_peak * 1.50f) {
		printf("test_am: asymmetry did not raise positive peak\n");
		return 0;
	}

	return 1;
}

static int
test_disabled_unchanged(void)
{
	struct cp_am_config config;
	size_t frame;

	fill_sine(TEST_FRAMES, CP_CHANNELS_MONO, 1000.0f, 0.30f);
	test_config(&config, CP_CHANNELS_MONO, 0);
	if (!run_am(&config, TEST_FRAMES, CP_CHANNELS_MONO))
		return 0;
	for (frame = 0; frame < TEST_FRAMES; frame++) {
		if (fabsf(input[frame] - output[frame]) > 0.000001f) {
			printf("test_am: disabled path changed signal\n");
			return 0;
		}
	}

	return 1;
}

static int
test_finite_large_input(void)
{
	struct cp_am_config config;
	size_t frame;

	fill_sine(TEST_FRAMES, CP_CHANNELS_MONO, 1000.0f, 20.0f);
	input[10] = NAN;
	input[20] = INFINITY;
	input[30] = -INFINITY;
	test_config(&config, CP_CHANNELS_MONO, 1);
	if (!run_am(&config, TEST_FRAMES, CP_CHANNELS_MONO))
		return 0;
	for (frame = 0; frame < TEST_FRAMES; frame++) {
		if (!isfinite(output[frame])) {
			printf("test_am: non-finite output\n");
			return 0;
		}
	}

	return 1;
}

static int
test_highpass_reduces_low_frequency(void)
{
	struct cp_am_config config;
	cp_sample_t ratio;

	fill_sine(TEST_FRAMES, CP_CHANNELS_MONO, 20.0f, 0.50f);
	test_config(&config, CP_CHANNELS_MONO, 1);
	config.highpass_hz = 100.0f;
	if (!run_am(&config, TEST_FRAMES, CP_CHANNELS_MONO))
		return 0;
	ratio = measure_rms(output, TEST_FRAMES, CP_CHANNELS_MONO, 0) /
	    measure_rms(input, TEST_FRAMES, CP_CHANNELS_MONO, 0);
	if (ratio >= 0.30f) {
		printf("test_am: high-pass too weak: %f\n", ratio);
		return 0;
	}

	return 1;
}

static int
test_invalid_filter_settings(void)
{
	struct cp_am am;
	struct cp_am_config config;

	test_config(&config, CP_CHANNELS_MONO, 1);
	config.lowpass_hz = TEST_RATE;
	if (cp_am_init(&am, &config) != CP_ERR_RANGE)
		return 0;

	test_config(&config, CP_CHANNELS_MONO, 1);
	config.highpass_hz = 5000.0f;
	config.lowpass_hz  = 4000.0f;
	if (cp_am_init(&am, &config) != CP_ERR_RANGE)
		return 0;

	test_config(&config, CP_CHANNELS_MONO, 1);
	config.asymmetry_enabled = 1;
	config.asymmetry_ratio   = 2.5f;
	if (cp_am_init(&am, &config) != CP_ERR_RANGE)
		return 0;

	return 1;
}

static int
test_lowpass_reduces_high_frequency(void)
{
	struct cp_am_config config;
	cp_sample_t ratio;

	fill_sine(TEST_FRAMES, CP_CHANNELS_MONO, 10000.0f, 0.50f);
	test_config(&config, CP_CHANNELS_MONO, 1);
	config.lowpass_hz = 4000.0f;
	if (!run_am(&config, TEST_FRAMES, CP_CHANNELS_MONO))
		return 0;
	ratio = measure_rms(output, TEST_FRAMES, CP_CHANNELS_MONO, 0) /
	    measure_rms(input, TEST_FRAMES, CP_CHANNELS_MONO, 0);
	if (ratio >= 0.25f) {
		printf("test_am: low-pass too weak: %f\n", ratio);
		return 0;
	}

	return 1;
}

static int
test_negative_peak_limit(void)
{
	struct cp_am_config config;
	size_t frame;

	fill_sine(TEST_FRAMES, CP_CHANNELS_MONO, 1000.0f, 4.0f);
	test_config(&config, CP_CHANNELS_MONO, 1);
	config.phase_rotator_enabled = 0;
	config.negative_peak_limit   = 0.60f;
	if (!run_am(&config, TEST_FRAMES, CP_CHANNELS_MONO))
		return 0;
	for (frame = TEST_FRAMES / 2; frame < TEST_FRAMES; frame++) {
		if (output[frame] < -0.6001f) {
			printf("test_am: negative peak exceeded\n");
			return 0;
		}
	}

	return 1;
}

static int
test_positive_peak_limit(void)
{
	struct cp_am_config config;
	size_t frame;

	fill_sine(TEST_FRAMES, CP_CHANNELS_MONO, 1000.0f, 4.0f);
	test_config(&config, CP_CHANNELS_MONO, 1);
	config.phase_rotator_enabled = 0;
	config.positive_peak_limit   = 0.70f;
	if (!run_am(&config, TEST_FRAMES, CP_CHANNELS_MONO))
		return 0;
	for (frame = TEST_FRAMES / 2; frame < TEST_FRAMES; frame++) {
		if (output[frame] > 0.7001f) {
			printf("test_am: positive peak exceeded\n");
			return 0;
		}
	}

	return 1;
}

static int
test_stereo_stable(void)
{
	struct cp_am_config config;
	size_t frame;

	fill_sine(TEST_FRAMES, CP_CHANNELS_STEREO, 1000.0f, 0.50f);
	test_config(&config, CP_CHANNELS_STEREO, 1);
	if (!run_am(&config, TEST_FRAMES, CP_CHANNELS_STEREO))
		return 0;
	for (frame = 0; frame < TEST_FRAMES * CP_CHANNELS_STEREO; frame++) {
		if (!isfinite(output[frame]) || fabsf(output[frame]) > 1.0f) {
			printf("test_am: stereo output unstable\n");
			return 0;
		}
	}

	return 1;
}

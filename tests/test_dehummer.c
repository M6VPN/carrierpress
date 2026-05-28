/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_dehummer.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_dehummer.h"

#define TEST_FRAMES		96000
#define TEST_RATE		48000.0f
#define TEST_TWO_PI		6.28318530717958647692f
#define TEST_RATIO_STRONG	0.35f

static cp_sample_t input[TEST_FRAMES * CP_MAX_CHANNELS];
static cp_sample_t output[TEST_FRAMES * CP_MAX_CHANNELS];

static void		fill_sine(cp_sample_t *, size_t, size_t, cp_sample_t);
static cp_sample_t	rms_channel(const cp_sample_t *, size_t, size_t,
		    size_t);
static int		run_dehummer(cp_sample_t, size_t, cp_sample_t,
		    size_t, int);
static int		test_50hz_reduced(void);
static int		test_60hz_reduced(void);
static int		test_disabled_unchanged(void);
static int		test_harmonic_reduced(void);
static int		test_invalid_config(void);
static int		test_nearby_preserved(void);
static int		test_nonfinite_safe(void);
static int		test_stereo_stable(void);

int
main(void)
{
	if (!test_50hz_reduced())
		return 1;
	if (!test_60hz_reduced())
		return 1;
	if (!test_harmonic_reduced())
		return 1;
	if (!test_nearby_preserved())
		return 1;
	if (!test_disabled_unchanged())
		return 1;
	if (!test_invalid_config())
		return 1;
	if (!test_stereo_stable())
		return 1;
	if (!test_nonfinite_safe())
		return 1;

	return 0;
}

static void
fill_sine(cp_sample_t *buffer, size_t frames, size_t channels,
	cp_sample_t frequency)
{
	cp_sample_t phase;
	cp_sample_t sample;
	size_t channel;
	size_t frame;

	phase = 0.0f;
	for (frame = 0; frame < frames; frame++) {
		sample = sinf(phase);
		for (channel = 0; channel < channels; channel++)
			buffer[(frame * channels) + channel] = sample;
		phase += (TEST_TWO_PI * frequency) / TEST_RATE;
		if (phase >= TEST_TWO_PI)
			phase -= TEST_TWO_PI;
	}
}

static cp_sample_t
rms_channel(const cp_sample_t *buffer, size_t frames, size_t channels,
	size_t channel)
{
	cp_sample_t sample;
	cp_sample_t sum;
	size_t frame;
	size_t start;

	start = frames / 2;
	sum   = 0.0f;
	for (frame = start; frame < frames; frame++) {
		sample = buffer[(frame * channels) + channel];
		sum += sample * sample;
	}

	return sqrtf(sum / (cp_sample_t)(frames - start));
}

static int
run_dehummer(cp_sample_t base_frequency, size_t harmonics,
	cp_sample_t q_factor, size_t channels, int enabled)
{
	struct cp_dehummer_config config;
	struct cp_dehummer dehummer;

	cp_dehummer_default_config(&config);
	config.sample_rate    = TEST_RATE;
	config.base_frequency = base_frequency;
	config.harmonic_count = harmonics;
	config.q_factor       = q_factor;
	config.channel_count  = channels;
	config.enabled        = enabled;

	if (cp_dehummer_init(&dehummer, &config) != CP_OK)
		return 0;
	if (cp_dehummer_process(&dehummer, input, output, TEST_FRAMES) != CP_OK)
		return 0;

	return 1;
}

static int
test_50hz_reduced(void)
{
	cp_sample_t ratio;

	fill_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 50.0f);
	if (!run_dehummer(50.0f, 1, CP_DEHUMMER_DEFAULT_Q,
	    CP_CHANNELS_MONO, 1))
		return 0;

	ratio = rms_channel(output, TEST_FRAMES, CP_CHANNELS_MONO, 0) /
	    rms_channel(input, TEST_FRAMES, CP_CHANNELS_MONO, 0);
	if (ratio >= TEST_RATIO_STRONG) {
		printf("test_dehummer: 50 Hz not reduced: %f\n", ratio);
		return 0;
	}

	return 1;
}

static int
test_60hz_reduced(void)
{
	cp_sample_t ratio;

	fill_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 60.0f);
	if (!run_dehummer(60.0f, 1, CP_DEHUMMER_DEFAULT_Q,
	    CP_CHANNELS_MONO, 1))
		return 0;

	ratio = rms_channel(output, TEST_FRAMES, CP_CHANNELS_MONO, 0) /
	    rms_channel(input, TEST_FRAMES, CP_CHANNELS_MONO, 0);
	if (ratio >= TEST_RATIO_STRONG) {
		printf("test_dehummer: 60 Hz not reduced: %f\n", ratio);
		return 0;
	}

	return 1;
}

static int
test_disabled_unchanged(void)
{
	cp_sample_t delta;
	size_t frame;

	fill_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 50.0f);
	if (!run_dehummer(50.0f, 4, CP_DEHUMMER_DEFAULT_Q,
	    CP_CHANNELS_MONO, 0))
		return 0;

	for (frame = 0; frame < TEST_FRAMES; frame++) {
		delta = fabsf(input[frame] - output[frame]);
		if (delta > 0.000001f) {
			printf("test_dehummer: disabled changed signal\n");
			return 0;
		}
	}

	return 1;
}

static int
test_harmonic_reduced(void)
{
	cp_sample_t ratio;

	fill_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 100.0f);
	if (!run_dehummer(50.0f, 2, CP_DEHUMMER_DEFAULT_Q,
	    CP_CHANNELS_MONO, 1))
		return 0;

	ratio = rms_channel(output, TEST_FRAMES, CP_CHANNELS_MONO, 0) /
	    rms_channel(input, TEST_FRAMES, CP_CHANNELS_MONO, 0);
	if (ratio >= TEST_RATIO_STRONG) {
		printf("test_dehummer: harmonic not reduced: %f\n", ratio);
		return 0;
	}

	return 1;
}

static int
test_invalid_config(void)
{
	struct cp_dehummer_config config;
	struct cp_dehummer dehummer;

	cp_dehummer_default_config(&config);
	config.enabled = 1;
	config.base_frequency = 30.0f;
	if (cp_dehummer_init(&dehummer, &config) != CP_ERR_RANGE)
		return 0;

	cp_dehummer_default_config(&config);
	config.enabled = 1;
	config.harmonic_count = CP_DEHUMMER_MAX_HARMONICS + 1;
	if (cp_dehummer_init(&dehummer, &config) != CP_ERR_RANGE)
		return 0;

	cp_dehummer_default_config(&config);
	config.enabled       = 1;
	config.sample_rate   = 200.0f;
	config.harmonic_count = 4;
	if (cp_dehummer_init(&dehummer, &config) != CP_ERR_RANGE)
		return 0;

	return 1;
}

static int
test_nearby_preserved(void)
{
	cp_sample_t ratio;

	fill_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 55.0f);
	if (!run_dehummer(50.0f, 1, CP_DEHUMMER_DEFAULT_Q,
	    CP_CHANNELS_MONO, 1))
		return 0;

	ratio = rms_channel(output, TEST_FRAMES, CP_CHANNELS_MONO, 0) /
	    rms_channel(input, TEST_FRAMES, CP_CHANNELS_MONO, 0);
	if (ratio <= 0.70f) {
		printf("test_dehummer: nearby tone damaged: %f\n", ratio);
		return 0;
	}

	return 1;
}

static int
test_nonfinite_safe(void)
{
	size_t frame;

	fill_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 50.0f);
	input[10] = NAN;
	input[20] = INFINITY;
	input[30] = -INFINITY;

	if (!run_dehummer(50.0f, 4, CP_DEHUMMER_DEFAULT_Q,
	    CP_CHANNELS_MONO, 1))
		return 0;

	for (frame = 0; frame < TEST_FRAMES; frame++) {
		if (!isfinite(output[frame])) {
			printf("test_dehummer: non-finite output\n");
			return 0;
		}
	}

	return 1;
}

static int
test_stereo_stable(void)
{
	cp_sample_t left;
	cp_sample_t right;

	fill_sine(input, TEST_FRAMES, CP_CHANNELS_STEREO, 50.0f);
	if (!run_dehummer(50.0f, 4, CP_DEHUMMER_DEFAULT_Q,
	    CP_CHANNELS_STEREO, 1))
		return 0;

	left  = rms_channel(output, TEST_FRAMES, CP_CHANNELS_STEREO, 0);
	right = rms_channel(output, TEST_FRAMES, CP_CHANNELS_STEREO, 1);
	if (!isfinite(left) || !isfinite(right) || fabsf(left - right) > 0.001f) {
		printf("test_dehummer: stereo unstable\n");
		return 0;
	}

	return 1;
}

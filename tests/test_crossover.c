/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_crossover.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_crossover.h"

#define TEST_FRAMES		4096
#define TEST_RATE		48000.0f
#define TEST_TWO_PI		6.28318530717958647692f

static cp_sample_t input[TEST_FRAMES * CP_MAX_CHANNELS];
static cp_sample_t band0[TEST_FRAMES * CP_MAX_CHANNELS];
static cp_sample_t band1[TEST_FRAMES * CP_MAX_CHANNELS];
static cp_sample_t band2[TEST_FRAMES * CP_MAX_CHANNELS];
static cp_sample_t band3[TEST_FRAMES * CP_MAX_CHANNELS];

static void		fill_mix(cp_sample_t *, size_t, size_t);
static int		run_crossover(size_t, size_t);
static int		test_finite_output(void);
static int		test_invalid_bands(void);
static int		test_recombined_bounded(void);
static int		test_recombined_energy(void);

int
main(void)
{
	if (!test_invalid_bands())
		return 1;
	if (!test_finite_output())
		return 1;
	if (!test_recombined_bounded())
		return 1;
	if (!test_recombined_energy())
		return 1;

	return 0;
}

static void
fill_mix(cp_sample_t *buffer, size_t frames, size_t channels)
{
	cp_sample_t phase_a;
	cp_sample_t phase_b;
	cp_sample_t phase_c;
	cp_sample_t sample;
	size_t channel;
	size_t frame;

	phase_a = 0.0f;
	phase_b = 0.0f;
	phase_c = 0.0f;
	for (frame = 0; frame < frames; frame++) {
		sample = (sinf(phase_a) * 0.30f) + (sinf(phase_b) * 0.20f) +
		    (sinf(phase_c) * 0.10f);
		for (channel = 0; channel < channels; channel++)
			buffer[(frame * channels) + channel] = sample;
		phase_a += (TEST_TWO_PI * 100.0f) / TEST_RATE;
		phase_b += (TEST_TWO_PI * 1000.0f) / TEST_RATE;
		phase_c += (TEST_TWO_PI * 6000.0f) / TEST_RATE;
		if (phase_a >= TEST_TWO_PI)
			phase_a -= TEST_TWO_PI;
		if (phase_b >= TEST_TWO_PI)
			phase_b -= TEST_TWO_PI;
		if (phase_c >= TEST_TWO_PI)
			phase_c -= TEST_TWO_PI;
	}
}

static int
run_crossover(size_t bands, size_t channels)
{
	cp_sample_t *outputs[CP_CROSSOVER_M5_MAX_BANDS];
	struct cp_crossover_config config;
	struct cp_crossover crossover;

	outputs[0] = band0;
	outputs[1] = band1;
	outputs[2] = band2;
	outputs[3] = band3;

	fill_mix(input, TEST_FRAMES, channels);
	cp_crossover_default_config(&config, channels, bands);
	config.sample_rate = TEST_RATE;
	if (cp_crossover_init(&crossover, &config) != CP_OK)
		return 0;
	if (cp_crossover_process(&crossover, input, outputs,
	    TEST_FRAMES) != CP_OK)
		return 0;

	return 1;
}

static int
test_finite_output(void)
{
	cp_sample_t sample;
	size_t frame;

	if (!run_crossover(4, CP_CHANNELS_STEREO))
		return 0;

	for (frame = 0; frame < TEST_FRAMES * CP_CHANNELS_STEREO; frame++) {
		sample = band0[frame] + band1[frame] + band2[frame] +
		    band3[frame];
		if (!isfinite(sample)) {
			printf("test_crossover: non-finite output\n");
			return 0;
		}
	}

	return 1;
}

static int
test_invalid_bands(void)
{
	struct cp_crossover_config config;
	struct cp_crossover crossover;

	cp_crossover_default_config(&config, CP_CHANNELS_MONO, 1);
	if (cp_crossover_init(&crossover, &config) != CP_ERR_RANGE)
		return 0;

	cp_crossover_default_config(&config, CP_CHANNELS_MONO,
	    CP_CROSSOVER_M5_MAX_BANDS + 1);
	if (cp_crossover_init(&crossover, &config) != CP_ERR_RANGE)
		return 0;

	return 1;
}

static int
test_recombined_bounded(void)
{
	cp_sample_t recombined;
	size_t frame;

	if (!run_crossover(3, CP_CHANNELS_MONO))
		return 0;

	for (frame = 0; frame < TEST_FRAMES; frame++) {
		recombined = band0[frame] + band1[frame] + band2[frame];
		if (fabsf(recombined) > 1.20f) {
			printf("test_crossover: recombined output too high\n");
			return 0;
		}
	}

	return 1;
}

static int
test_recombined_energy(void)
{
	cp_sample_t input_sum;
	cp_sample_t output_sum;
	cp_sample_t ratio;
	cp_sample_t recombined;
	size_t frame;

	if (!run_crossover(4, CP_CHANNELS_MONO))
		return 0;

	input_sum  = 0.0f;
	output_sum = 0.0f;
	for (frame = 0; frame < TEST_FRAMES; frame++) {
		recombined = band0[frame] + band1[frame] + band2[frame] +
		    band3[frame];
		input_sum += input[frame] * input[frame];
		output_sum += recombined * recombined;
	}
	ratio = sqrtf(output_sum / input_sum);
	if (ratio < 0.98f || ratio > 1.02f) {
		printf("test_crossover: recombined energy changed: %f\n",
		    ratio);
		return 0;
	}

	return 1;
}

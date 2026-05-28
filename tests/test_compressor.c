/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_compressor.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_compressor.h"

#define TEST_FRAMES	480
#define TEST_RATE	48000.0f
#define TEST_SMOOTH_FRAMES	48000

static void	fill(cp_sample_t *, size_t, cp_sample_t);
static void	test_config(struct cp_compressor_config *, size_t);
static int	test_above_threshold_reduces(void);
static int	test_attack_smoothing(void);
static int	test_below_threshold_unchanged(void);
static int	test_nonfinite_safe(void);
static int	test_ratio_affects_reduction(void);
static int	test_stereo_linked(void);

int
main(void)
{
	if (!test_above_threshold_reduces())
		return 1;
	if (!test_below_threshold_unchanged())
		return 1;
	if (!test_ratio_affects_reduction())
		return 1;
	if (!test_attack_smoothing())
		return 1;
	if (!test_stereo_linked())
		return 1;
	if (!test_nonfinite_safe())
		return 1;

	return 0;
}

static void
fill(cp_sample_t *buffer, size_t samples, cp_sample_t value)
{
	size_t index;

	for (index = 0; index < samples; index++)
		buffer[index] = value;
}

static void
test_config(struct cp_compressor_config *config, size_t channels)
{
	cp_compressor_default_config(config);
	config->threshold_db   = -20.0f;
	config->ratio          = 4.0f;
	config->attack_ms      = 0.0f;
	config->release_ms     = 0.0f;
	config->makeup_gain_db = 0.0f;
	config->knee_db        = 0.0f;
	config->sample_rate    = TEST_RATE;
	config->channels       = channels;
	config->enabled        = 1;
}

static int
test_above_threshold_reduces(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_compressor_config config;
	struct cp_compressor compressor;

	fill(input, TEST_FRAMES, 0.80f);
	test_config(&config, CP_CHANNELS_MONO);
	if (cp_compressor_init(&compressor, &config) != CP_OK)
		return 0;
	if (cp_compressor_process(&compressor, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	if (compressor.gain_reduction_db <= 1.0f || output[TEST_FRAMES - 1] >=
	    input[TEST_FRAMES - 1]) {
		printf("test_compressor: loud input not reduced\n");
		return 0;
	}

	return 1;
}

static int
test_attack_smoothing(void)
{
	cp_sample_t frame[CP_CHANNELS_MONO];
	cp_sample_t output[CP_CHANNELS_MONO];
	struct cp_compressor_config config;
	struct cp_compressor compressor;
	cp_sample_t first_gr;
	size_t index;

	frame[0] = 0.80f;
	test_config(&config, CP_CHANNELS_MONO);
	config.attack_ms = 100.0f;
	if (cp_compressor_init(&compressor, &config) != CP_OK)
		return 0;
	if (cp_compressor_process_frame(&compressor, frame, output) != CP_OK)
		return 0;
	first_gr = compressor.gain_reduction_db;
	for (index = 0; index < TEST_SMOOTH_FRAMES; index++) {
		if (cp_compressor_process_frame(&compressor, frame,
		    output) != CP_OK)
			return 0;
	}
	if (first_gr >= compressor.gain_reduction_db) {
		printf("test_compressor: attack smoothing did not move\n");
		return 0;
	}

	return 1;
}

static int
test_below_threshold_unchanged(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_compressor_config config;
	struct cp_compressor compressor;

	fill(input, TEST_FRAMES, 0.02f);
	test_config(&config, CP_CHANNELS_MONO);
	if (cp_compressor_init(&compressor, &config) != CP_OK)
		return 0;
	if (cp_compressor_process(&compressor, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	if (compressor.gain_reduction_db > 0.10f ||
	    fabsf(output[TEST_FRAMES - 1] - input[TEST_FRAMES - 1]) >
	    0.0001f) {
		printf("test_compressor: quiet input changed\n");
		return 0;
	}

	return 1;
}

static int
test_nonfinite_safe(void)
{
	cp_sample_t input[4] = { NAN, INFINITY, -INFINITY, 0.25f };
	cp_sample_t output[4];
	struct cp_compressor_config config;
	struct cp_compressor compressor;
	size_t index;

	test_config(&config, CP_CHANNELS_MONO);
	if (cp_compressor_init(&compressor, &config) != CP_OK)
		return 0;
	if (cp_compressor_process(&compressor, input, output, 4) != CP_OK)
		return 0;
	for (index = 0; index < 4; index++) {
		if (!isfinite(output[index])) {
			printf("test_compressor: non-finite output\n");
			return 0;
		}
	}

	return 1;
}

static int
test_ratio_affects_reduction(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_compressor_config config;
	struct cp_compressor high_ratio;
	struct cp_compressor low_ratio;

	fill(input, TEST_FRAMES, 0.80f);
	test_config(&config, CP_CHANNELS_MONO);
	config.ratio = 2.0f;
	if (cp_compressor_init(&low_ratio, &config) != CP_OK)
		return 0;
	config.ratio = 8.0f;
	if (cp_compressor_init(&high_ratio, &config) != CP_OK)
		return 0;
	if (cp_compressor_process(&low_ratio, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	if (cp_compressor_process(&high_ratio, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	if (high_ratio.gain_reduction_db <= low_ratio.gain_reduction_db) {
		printf("test_compressor: ratio did not affect reduction\n");
		return 0;
	}

	return 1;
}

static int
test_stereo_linked(void)
{
	cp_sample_t input[TEST_FRAMES * CP_CHANNELS_STEREO];
	cp_sample_t output[TEST_FRAMES * CP_CHANNELS_STEREO];
	struct cp_compressor_config config;
	struct cp_compressor compressor;
	cp_sample_t left_ratio;
	cp_sample_t right_ratio;
	size_t frame;

	for (frame = 0; frame < TEST_FRAMES; frame++) {
		input[(frame * CP_CHANNELS_STEREO) + 0] = 0.80f;
		input[(frame * CP_CHANNELS_STEREO) + 1] = 0.20f;
	}
	test_config(&config, CP_CHANNELS_STEREO);
	if (cp_compressor_init(&compressor, &config) != CP_OK)
		return 0;
	if (cp_compressor_process(&compressor, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;

	left_ratio = output[(TEST_FRAMES - 1) * CP_CHANNELS_STEREO] /
	    input[(TEST_FRAMES - 1) * CP_CHANNELS_STEREO];
	right_ratio = output[((TEST_FRAMES - 1) * CP_CHANNELS_STEREO) + 1] /
	    input[((TEST_FRAMES - 1) * CP_CHANNELS_STEREO) + 1];
	if (fabsf(left_ratio - right_ratio) > 0.0001f) {
		printf("test_compressor: stereo gain not linked\n");
		return 0;
	}

	return 1;
}

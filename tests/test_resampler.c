/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_resampler.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_resampler.h"

#define TEST_FRAMES	32
#define TEST_CHANNELS	2

static int	test_capacity(void);
static int	test_disabled_copy(void);
static int	test_invalid_config(void);
static int	test_nonfinite_rejected(void);
static int	test_stereo_stability(void);
static int	test_upsample_downsample(void);

int
main(void)
{
	if (!test_capacity())
		return 1;
	if (!test_disabled_copy())
		return 1;
	if (!test_invalid_config())
		return 1;
	if (!test_nonfinite_rejected())
		return 1;
	if (!test_stereo_stability())
		return 1;
	if (!test_upsample_downsample())
		return 1;

	return 0;
}

static int
test_capacity(void)
{
	size_t capacity;

	capacity = cp_resampler_output_capacity(441, 44100.0, 48000.0);
	if (capacity < 480) {
		printf("test_resampler: upsample capacity too small\n");
		return 0;
	}
	if (cp_resampler_output_capacity(0, 44100.0, 48000.0) != 0) {
		printf("test_resampler: zero capacity accepted\n");
		return 0;
	}
	if (cp_resampler_output_capacity(128, 0.0, 48000.0) != 0) {
		printf("test_resampler: bad rate capacity accepted\n");
		return 0;
	}

	return 1;
}

static int
test_disabled_copy(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_resampler_config config;
	struct cp_resampler state;
	size_t frame;
	size_t output_frames;

	cp_resampler_default_config(&config);
	config.channel_count = CP_CHANNELS_MONO;
	config.input_rate = 48000.0;
	config.output_rate = 48000.0;
	config.enabled = 0;
	if (cp_resampler_init(&state, &config) != CP_OK) {
		printf("test_resampler: disabled init failed\n");
		return 0;
	}

	for (frame = 0; frame < TEST_FRAMES; frame++)
		input[frame] = (cp_sample_t)frame / 100.0f;
	if (cp_resampler_process(&state, input, TEST_FRAMES, output,
	    TEST_FRAMES, &output_frames) != CP_OK) {
		printf("test_resampler: disabled process failed\n");
		return 0;
	}
	if (output_frames != TEST_FRAMES) {
		printf("test_resampler: disabled frame count mismatch\n");
		return 0;
	}
	for (frame = 0; frame < TEST_FRAMES; frame++) {
		if (output[frame] != input[frame]) {
			printf("test_resampler: disabled copy mismatch\n");
			return 0;
		}
	}

	return 1;
}

static int
test_invalid_config(void)
{
	struct cp_resampler_config config;
	struct cp_resampler state;

	cp_resampler_default_config(&config);
	config.channel_count = 3;
	if (cp_resampler_init(&state, &config) != CP_ERR_RANGE) {
		printf("test_resampler: bad channel count accepted\n");
		return 0;
	}

	cp_resampler_default_config(&config);
	config.input_rate = 1000.0;
	if (cp_resampler_init(&state, &config) != CP_ERR_RANGE) {
		printf("test_resampler: bad input rate accepted\n");
		return 0;
	}

	cp_resampler_default_config(&config);
	config.output_rate = 1000000.0;
	if (cp_resampler_init(&state, &config) != CP_ERR_RANGE) {
		printf("test_resampler: bad output rate accepted\n");
		return 0;
	}

	return 1;
}

static int
test_nonfinite_rejected(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES * 2];
	struct cp_resampler_config config;
	struct cp_resampler state;
	size_t frame;
	size_t output_frames;

	cp_resampler_default_config(&config);
	config.channel_count = CP_CHANNELS_MONO;
	config.enabled = 1;
	if (cp_resampler_init(&state, &config) != CP_OK)
		return 0;
	for (frame = 0; frame < TEST_FRAMES; frame++)
		input[frame] = 0.0f;
	input[4] = NAN;
	if (cp_resampler_process(&state, input, TEST_FRAMES, output,
	    TEST_FRAMES * 2, &output_frames) != CP_ERR_RANGE) {
		printf("test_resampler: nonfinite accepted\n");
		return 0;
	}

	return 1;
}

static int
test_stereo_stability(void)
{
	cp_sample_t input[TEST_FRAMES * TEST_CHANNELS];
	cp_sample_t output[(TEST_FRAMES * TEST_CHANNELS) * 2];
	struct cp_resampler_config config;
	struct cp_resampler state;
	size_t frame;
	size_t output_frames;

	cp_resampler_default_config(&config);
	config.channel_count = CP_CHANNELS_STEREO;
	config.input_rate = 44100.0;
	config.output_rate = 48000.0;
	config.enabled = 1;
	if (cp_resampler_init(&state, &config) != CP_OK) {
		printf("test_resampler: stereo init failed\n");
		return 0;
	}
	for (frame = 0; frame < TEST_FRAMES; frame++) {
		input[(frame * TEST_CHANNELS) + 0] =
		    (cp_sample_t)frame / 100.0f;
		input[(frame * TEST_CHANNELS) + 1] =
		    -((cp_sample_t)frame / 100.0f);
	}
	if (cp_resampler_process(&state, input, TEST_FRAMES, output,
	    TEST_FRAMES * 2, &output_frames) != CP_OK) {
		printf("test_resampler: stereo process failed\n");
		return 0;
	}
	if (output_frames == 0) {
		printf("test_resampler: stereo produced no frames\n");
		return 0;
	}
	for (frame = 0; frame < output_frames; frame++) {
		if (!isfinite(output[(frame * TEST_CHANNELS) + 0]) ||
		    !isfinite(output[(frame * TEST_CHANNELS) + 1])) {
			printf("test_resampler: stereo nonfinite output\n");
			return 0;
		}
	}

	return 1;
}

static int
test_upsample_downsample(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES * 2];
	struct cp_resampler_config config;
	struct cp_resampler state;
	size_t frame;
	size_t output_frames;

	cp_resampler_default_config(&config);
	config.channel_count = CP_CHANNELS_MONO;
	config.input_rate = 44100.0;
	config.output_rate = 48000.0;
	config.enabled = 1;
	if (cp_resampler_init(&state, &config) != CP_OK)
		return 0;
	for (frame = 0; frame < TEST_FRAMES; frame++)
		input[frame] = sinf((float)frame * 0.1f) * 0.5f;
	if (cp_resampler_process(&state, input, TEST_FRAMES, output,
	    TEST_FRAMES * 2, &output_frames) != CP_OK) {
		printf("test_resampler: upsample failed\n");
		return 0;
	}
	if (output_frames <= TEST_FRAMES) {
		printf("test_resampler: upsample frame count too small\n");
		return 0;
	}

	config.input_rate = 48000.0;
	config.output_rate = 44100.0;
	if (cp_resampler_init(&state, &config) != CP_OK)
		return 0;
	if (cp_resampler_process(&state, input, TEST_FRAMES, output,
	    TEST_FRAMES * 2, &output_frames) != CP_OK) {
		printf("test_resampler: downsample failed\n");
		return 0;
	}
	if (output_frames >= TEST_FRAMES) {
		printf("test_resampler: downsample frame count too large\n");
		return 0;
	}

	return 1;
}

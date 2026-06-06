/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_declipper.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "cp_declipper.h"

#define TEST_FRAMES		4096u
#define TEST_RATE		48000.0f
#define TEST_TWO_PI		6.28318530717958647692f
#define TEST_CLIP		0.95f
#define TEST_EPSILON		0.000001f

static void	fill_burst(cp_sample_t *, size_t, size_t);
static void	fill_clipped_sine(cp_sample_t *, size_t, size_t);
static void	fill_sine(cp_sample_t *, size_t, size_t, cp_sample_t);
static int	make_analysis(const cp_sample_t *, size_t, size_t,
		    struct cp_restoration_metrics *);
static int	process_with_analysis(struct cp_declipper *, const cp_sample_t *,
		    cp_sample_t *, const cp_sample_t *, size_t, size_t);
static int	run_case(const char *, int (*)(void));
static int	test_analysis_gate_required(void);
static int	test_clipped_sine_repaired(void);
static int	test_disabled_passthrough(void);
static int	test_invalid_config_rejected(void);
static int	test_nonfinite_bypassed(void);
static int	test_stereo_stable(void);
static int	test_transient_bypassed(void);

int
main(void)
{
	if (!run_case("disabled_passthrough", test_disabled_passthrough))
		return 1;
	if (!run_case("analysis_gate_required", test_analysis_gate_required))
		return 1;
	if (!run_case("clipped_sine_repaired", test_clipped_sine_repaired))
		return 1;
	if (!run_case("transient_bypassed", test_transient_bypassed))
		return 1;
	if (!run_case("stereo_stable", test_stereo_stable))
		return 1;
	if (!run_case("nonfinite_bypassed", test_nonfinite_bypassed))
		return 1;
	if (!run_case("invalid_config_rejected", test_invalid_config_rejected))
		return 1;

	return 0;
}

static void
fill_burst(cp_sample_t *buffer, size_t frames, size_t channels)
{
	size_t channel;
	size_t frame;

	memset(buffer, 0, frames * channels * sizeof(*buffer));
	for (frame = 100; frame < 104 && frame < frames; frame++) {
		for (channel = 0; channel < channels; channel++)
			buffer[(frame * channels) + channel] = TEST_CLIP;
	}
}

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
		if (sample > TEST_CLIP)
			sample = TEST_CLIP;
		if (sample < -TEST_CLIP)
			sample = -TEST_CLIP;
		for (channel = 0; channel < channels; channel++)
			buffer[(frame * channels) + channel] = sample;
		phase += step;
		if (phase >= TEST_TWO_PI)
			phase -= TEST_TWO_PI;
	}
}

static void
fill_sine(cp_sample_t *buffer, size_t frames, size_t channels,
	cp_sample_t level)
{
	cp_sample_t phase;
	cp_sample_t sample;
	cp_sample_t step;
	size_t channel;
	size_t frame;

	phase = 0.0f;
	step = (TEST_TWO_PI * 1000.0f) / TEST_RATE;
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
make_analysis(const cp_sample_t *input, size_t frames, size_t channels,
	struct cp_restoration_metrics *metrics)
{
	struct cp_restoration_config config;
	struct cp_restoration restoration;
	cp_sample_t scratch[TEST_FRAMES * CP_CHANNELS_STEREO];

	if (input == NULL || metrics == NULL || channels > CP_CHANNELS_STEREO)
		return 0;
	memcpy(scratch, input, frames * channels * sizeof(*scratch));
	cp_restoration_default_config(&config);
	config.enabled = 1;
	config.channel_count = channels;
	config.sample_rate = TEST_RATE;
	config.clip_threshold = TEST_CLIP;
	config.analysis_window_frames = frames;
	if (cp_restoration_init(&restoration, &config) != CP_OK)
		return 0;
	if (cp_restoration_process(&restoration, scratch, frames) != CP_OK)
		return 0;
	*metrics = restoration.metrics;

	return 1;
}

static int
process_with_analysis(struct cp_declipper *declipper, const cp_sample_t *input,
	cp_sample_t *output, const cp_sample_t *analysis_input, size_t frames,
	size_t channels)
{
	struct cp_restoration_metrics metrics;

	if (!make_analysis(analysis_input, frames, channels, &metrics))
		return 0;
	if (cp_declipper_process(declipper, &metrics, input, output,
	    frames) != CP_OK)
		return 0;

	return 1;
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
test_analysis_gate_required(void)
{
	struct cp_declipper_config config;
	struct cp_declipper declipper;
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	size_t frame;

	cp_declipper_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	config.sample_rate = TEST_RATE;
	if (cp_declipper_init(&declipper, &config) != CP_OK)
		return 0;
	fill_clipped_sine(input, TEST_FRAMES, CP_CHANNELS_MONO);
	if (cp_declipper_process(&declipper, NULL, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	for (frame = 0; frame < TEST_FRAMES; frame++) {
		if (fabsf(input[frame] - output[frame]) > TEST_EPSILON)
			return 0;
	}
	if (declipper.metrics.bypass_reason !=
	    CP_DECLIPPER_BYPASS_ANALYSIS_REQUIRED)
		return 0;

	return 1;
}

static int
test_clipped_sine_repaired(void)
{
	struct cp_declipper_config config;
	struct cp_declipper declipper;
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	size_t frame;
	size_t changed;

	cp_declipper_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	config.sample_rate = TEST_RATE;
	if (cp_declipper_init(&declipper, &config) != CP_OK)
		return 0;
	fill_clipped_sine(input, TEST_FRAMES, CP_CHANNELS_MONO);
	if (!process_with_analysis(&declipper, input, output, input,
	    TEST_FRAMES, CP_CHANNELS_MONO))
		return 0;

	changed = 0;
	for (frame = 0; frame < TEST_FRAMES; frame++) {
		if (!isfinite(output[frame]))
			return 0;
		if (fabsf(input[frame] - output[frame]) > TEST_EPSILON)
			changed++;
		if (fabsf(output[frame]) > CP_DEFAULT_CEILING + TEST_EPSILON)
			return 0;
	}
	if (changed == 0 || declipper.metrics.repaired_sample_count == 0)
		return 0;
	if (declipper.metrics.bypass_reason != CP_DECLIPPER_BYPASS_NONE &&
	    declipper.metrics.bypass_reason != CP_DECLIPPER_BYPASS_LONG_RUN)
		return 0;

	return 1;
}

static int
test_disabled_passthrough(void)
{
	struct cp_declipper_config config;
	struct cp_declipper declipper;
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	size_t frame;

	cp_declipper_default_config(&config);
	config.enabled = 0;
	config.channel_count = CP_CHANNELS_MONO;
	config.sample_rate = TEST_RATE;
	if (cp_declipper_init(&declipper, &config) != CP_OK)
		return 0;
	fill_clipped_sine(input, TEST_FRAMES, CP_CHANNELS_MONO);
	if (cp_declipper_process(&declipper, NULL, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	for (frame = 0; frame < TEST_FRAMES; frame++) {
		if (fabsf(input[frame] - output[frame]) > TEST_EPSILON)
			return 0;
	}
	if (declipper.metrics.repaired_sample_count != 0)
		return 0;

	return 1;
}

static int
test_invalid_config_rejected(void)
{
	struct cp_declipper_config config;
	struct cp_declipper declipper;

	cp_declipper_default_config(&config);
	config.enabled = 1;
	config.channel_count = 3;
	if (cp_declipper_init(&declipper, &config) != CP_ERR_CHANNELS)
		return 0;
	cp_declipper_default_config(&config);
	config.enabled = 1;
	config.repair_strength = 2.0f;
	if (cp_declipper_init(&declipper, &config) != CP_ERR_RANGE)
		return 0;
	cp_declipper_default_config(&config);
	config.enabled = 1;
	config.max_repair_samples = CP_DECLIPPER_MAX_REPAIR_SAMPLES + 1;
	if (cp_declipper_init(&declipper, &config) != CP_ERR_RANGE)
		return 0;

	return 1;
}

static int
test_nonfinite_bypassed(void)
{
	struct cp_declipper_config config;
	struct cp_declipper declipper;
	struct cp_restoration_metrics metrics;
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	size_t frame;

	cp_declipper_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	config.sample_rate = TEST_RATE;
	if (cp_declipper_init(&declipper, &config) != CP_OK)
		return 0;
	fill_sine(input, TEST_FRAMES, CP_CHANNELS_MONO, 0.25f);
	if (!make_analysis(input, TEST_FRAMES, CP_CHANNELS_MONO, &metrics))
		return 0;
	metrics.finite = 0;
	if (cp_declipper_process(&declipper, &metrics, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	for (frame = 0; frame < TEST_FRAMES; frame++) {
		if (fabsf(input[frame] - output[frame]) > TEST_EPSILON)
			return 0;
	}
	if (declipper.metrics.bypass_reason != CP_DECLIPPER_BYPASS_NONFINITE)
		return 0;

	return 1;
}

static int
test_stereo_stable(void)
{
	struct cp_declipper_config config;
	struct cp_declipper declipper;
	cp_sample_t input[TEST_FRAMES * CP_CHANNELS_STEREO];
	cp_sample_t output[TEST_FRAMES * CP_CHANNELS_STEREO];
	size_t sample;

	cp_declipper_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_STEREO;
	config.sample_rate = TEST_RATE;
	if (cp_declipper_init(&declipper, &config) != CP_OK)
		return 0;
	fill_clipped_sine(input, TEST_FRAMES, CP_CHANNELS_STEREO);
	if (!process_with_analysis(&declipper, input, output, input,
	    TEST_FRAMES, CP_CHANNELS_STEREO))
		return 0;
	for (sample = 0; sample < TEST_FRAMES * CP_CHANNELS_STEREO; sample++) {
		if (!isfinite(output[sample]))
			return 0;
	}
	if (declipper.metrics.repaired_sample_count == 0)
		return 0;

	return 1;
}

static int
test_transient_bypassed(void)
{
	struct cp_declipper_config config;
	struct cp_declipper declipper;
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	size_t frame;

	cp_declipper_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_MONO;
	config.sample_rate = TEST_RATE;
	if (cp_declipper_init(&declipper, &config) != CP_OK)
		return 0;
	fill_burst(input, TEST_FRAMES, CP_CHANNELS_MONO);
	if (!process_with_analysis(&declipper, input, output, input,
	    TEST_FRAMES, CP_CHANNELS_MONO))
		return 0;
	for (frame = 0; frame < TEST_FRAMES; frame++) {
		if (fabsf(input[frame] - output[frame]) > TEST_EPSILON)
			return 0;
	}
	if (declipper.metrics.repaired_sample_count != 0)
		return 0;
	if (declipper.metrics.bypass_reason != CP_DECLIPPER_BYPASS_TRANSIENT &&
	    declipper.metrics.bypass_reason != CP_DECLIPPER_BYPASS_LOW_CONFIDENCE)
		return 0;

	return 1;
}

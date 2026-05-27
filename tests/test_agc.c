/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_agc.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_agc.h"

#define TEST_EPSILON	0.0001f
#define TEST_FRAMES	480
#define TEST_RATE	48000.0f

static int	close_enough(cp_sample_t, cp_sample_t);
static void	fill_buffer(cp_sample_t *, size_t, cp_sample_t);
static void	test_config(struct cp_agc_config *);
static int	test_fast_attack(void);
static int	test_gain_bounds(void);
static int	test_hold_delays_release(void);
static int	test_loud_reduces_gain(void);
static int	test_nonfinite_input(void);
static int	test_quiet_raises_gain(void);
static int	test_silence_stays_put(void);
static int	test_stereo_linked_gain(void);

int
main(void)
{
	if (!test_loud_reduces_gain())
		return 1;
	if (!test_quiet_raises_gain())
		return 1;
	if (!test_silence_stays_put())
		return 1;
	if (!test_stereo_linked_gain())
		return 1;
	if (!test_gain_bounds())
		return 1;
	if (!test_nonfinite_input())
		return 1;
	if (!test_fast_attack())
		return 1;
	if (!test_hold_delays_release())
		return 1;

	return 0;
}

static int
close_enough(cp_sample_t a, cp_sample_t b)
{
	return fabsf(a - b) <= TEST_EPSILON;
}

static void
fill_buffer(cp_sample_t *buffer, size_t samples, cp_sample_t value)
{
	size_t index;

	for (index = 0; index < samples; index++)
		buffer[index] = value;
}

static void
test_config(struct cp_agc_config *config)
{
	cp_agc_default_config(config);
	config->target_rms           = 0.20f;
	config->min_gain             = 0.25f;
	config->max_gain             = 4.0f;
	config->attack_ms            = 50.0f;
	config->release_ms           = 100.0f;
	config->fast_attack_ms       = 5.0f;
	config->hold_ms              = 0.0f;
	config->gate_threshold_db    = -50.0f;
	config->silence_threshold_db = -80.0f;
	config->max_gain_step_db     = 24.0f;
	config->sample_rate          = TEST_RATE;
}

static int
test_fast_attack(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_agc_config config;
	struct cp_agc fast_agc;
	struct cp_agc normal_agc;

	fill_buffer(input, TEST_FRAMES, 1.0f);
	test_config(&config);
	config.attack_ms      = 500.0f;
	config.fast_attack_ms = 0.0f;
	if (cp_agc_init_config(&fast_agc, CP_CHANNELS_MONO, &config) != CP_OK)
		return 0;

	config.fast_attack_ms = 500.0f;
	if (cp_agc_init_config(&normal_agc, CP_CHANNELS_MONO, &config) != CP_OK)
		return 0;

	if (cp_agc_process(&fast_agc, input, output, TEST_FRAMES) != CP_OK)
		return 0;
	if (cp_agc_process(&normal_agc, input, output, TEST_FRAMES) != CP_OK)
		return 0;

	if (fast_agc.gain >= normal_agc.gain) {
		printf("test_agc: fast attack did not reduce faster\n");
		return 0;
	}

	return 1;
}

static int
test_gain_bounds(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_agc_config config;
	struct cp_agc agc;

	test_config(&config);
	config.min_gain       = 0.50f;
	config.max_gain       = 2.0f;
	config.attack_ms      = 0.0f;
	config.release_ms     = 0.0f;
	config.fast_attack_ms = 0.0f;

	fill_buffer(input, TEST_FRAMES, 1.0f);
	if (cp_agc_init_config(&agc, CP_CHANNELS_MONO, &config) != CP_OK)
		return 0;
	if (cp_agc_process(&agc, input, output, TEST_FRAMES) != CP_OK)
		return 0;
	if (agc.gain < config.min_gain || agc.gain > config.max_gain) {
		printf("test_agc: loud gain outside bounds\n");
		return 0;
	}

	fill_buffer(input, TEST_FRAMES, 0.01f);
	if (cp_agc_reset(&agc) != CP_OK)
		return 0;
	if (cp_agc_process(&agc, input, output, TEST_FRAMES) != CP_OK)
		return 0;
	if (agc.gain < config.min_gain || agc.gain > config.max_gain) {
		printf("test_agc: quiet gain outside bounds\n");
		return 0;
	}

	return 1;
}

static int
test_hold_delays_release(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_agc_config config;
	struct cp_agc agc;
	cp_sample_t gain_after_loud;

	test_config(&config);
	config.attack_ms      = 0.0f;
	config.fast_attack_ms = 0.0f;
	config.release_ms     = 0.0f;
	config.hold_ms        = 1000.0f;

	fill_buffer(input, TEST_FRAMES, 1.0f);
	if (cp_agc_init_config(&agc, CP_CHANNELS_MONO, &config) != CP_OK)
		return 0;
	if (cp_agc_process(&agc, input, output, TEST_FRAMES) != CP_OK)
		return 0;

	gain_after_loud = agc.gain;
	fill_buffer(input, TEST_FRAMES, 0.10f);
	if (cp_agc_process(&agc, input, output, TEST_FRAMES) != CP_OK)
		return 0;

	if (!close_enough(agc.gain, gain_after_loud) ||
	    agc.gate_state != CP_AGC_STATE_HELD) {
		printf("test_agc: hold did not delay release\n");
		return 0;
	}

	return 1;
}

static int
test_loud_reduces_gain(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_agc_config config;
	struct cp_agc agc;

	fill_buffer(input, TEST_FRAMES, 1.0f);
	test_config(&config);
	config.attack_ms      = 0.0f;
	config.fast_attack_ms = 0.0f;

	if (cp_agc_init_config(&agc, CP_CHANNELS_MONO, &config) != CP_OK)
		return 0;
	if (cp_agc_process(&agc, input, output, TEST_FRAMES) != CP_OK)
		return 0;

	if (agc.gain >= 1.0f || agc.gate_state != CP_AGC_STATE_OPEN) {
		printf("test_agc: loud input did not reduce gain\n");
		return 0;
	}

	return 1;
}

static int
test_nonfinite_input(void)
{
	cp_sample_t input[4] = { NAN, INFINITY, -INFINITY, 0.25f };
	cp_sample_t output[4];
	struct cp_agc_config config;
	struct cp_agc agc;
	size_t index;

	test_config(&config);
	if (cp_agc_init_config(&agc, CP_CHANNELS_MONO, &config) != CP_OK)
		return 0;
	if (cp_agc_process(&agc, input, output, 4) != CP_OK)
		return 0;

	for (index = 0; index < 4; index++) {
		if (!isfinite(output[index])) {
			printf("test_agc: non-finite output\n");
			return 0;
		}
	}
	if (!isfinite(agc.last_rms) || !isfinite(agc.gain)) {
		printf("test_agc: non-finite meter state\n");
		return 0;
	}

	return 1;
}

static int
test_quiet_raises_gain(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_agc_config config;
	struct cp_agc agc;

	fill_buffer(input, TEST_FRAMES, 0.05f);
	test_config(&config);
	config.release_ms = 100.0f;

	if (cp_agc_init_config(&agc, CP_CHANNELS_MONO, &config) != CP_OK)
		return 0;
	if (cp_agc_process(&agc, input, output, TEST_FRAMES) != CP_OK)
		return 0;

	if (agc.gain <= 1.0f || agc.gain >= config.max_gain) {
		printf("test_agc: quiet input did not raise gain slowly\n");
		return 0;
	}

	return 1;
}

static int
test_silence_stays_put(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_agc_config config;
	struct cp_agc agc;
	size_t pass;

	fill_buffer(input, TEST_FRAMES, 0.0f);
	test_config(&config);

	if (cp_agc_init_config(&agc, CP_CHANNELS_MONO, &config) != CP_OK)
		return 0;

	for (pass = 0; pass < 100; pass++) {
		if (cp_agc_process(&agc, input, output, TEST_FRAMES) != CP_OK)
			return 0;
	}

	if (!close_enough(agc.gain, 1.0f) ||
	    agc.gate_state != CP_AGC_STATE_SILENT) {
		printf("test_agc: silence changed gain\n");
		return 0;
	}

	return 1;
}

static int
test_stereo_linked_gain(void)
{
	cp_sample_t input[TEST_FRAMES * 2];
	cp_sample_t output[TEST_FRAMES * 2];
	struct cp_agc_config config;
	struct cp_agc agc;
	cp_sample_t left_ratio;
	cp_sample_t right_ratio;
	size_t frame;

	for (frame = 0; frame < TEST_FRAMES; frame++) {
		input[(frame * 2)]     = 0.10f;
		input[(frame * 2) + 1] = 0.50f;
	}

	test_config(&config);
	if (cp_agc_init_config(&agc, CP_CHANNELS_STEREO, &config) != CP_OK)
		return 0;
	if (cp_agc_process(&agc, input, output, TEST_FRAMES) != CP_OK)
		return 0;

	left_ratio  = output[0] / input[0];
	right_ratio = output[1] / input[1];

	if (!close_enough(left_ratio, right_ratio)) {
		printf("test_agc: stereo gain is not linked\n");
		return 0;
	}

	return 1;
}

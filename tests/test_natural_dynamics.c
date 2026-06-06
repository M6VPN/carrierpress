/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_natural_dynamics.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_natural_dynamics.h"

#define TEST_FRAMES	480
#define TEST_RATE	48000.0f
#define TEST_EPSILON	0.0001f

static void	fill(cp_sample_t *, size_t, cp_sample_t);
static int	test_disabled_passthrough(void);
static int	test_invalid_config(void);
static int	test_loud_reduces(void);
static int	test_nonfinite_safe(void);
static int	test_quiet_little_change(void);
static int	test_stereo_linked(void);
static void	valid_config(struct cp_natural_dynamics_config *, size_t);

int
main(void)
{
	if (!test_disabled_passthrough())
		return 1;
	if (!test_invalid_config())
		return 1;
	if (!test_loud_reduces())
		return 1;
	if (!test_quiet_little_change())
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

static int
test_disabled_passthrough(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_natural_dynamics_config config;
	struct cp_natural_dynamics dynamics;
	size_t index;

	valid_config(&config, CP_CHANNELS_MONO);
	config.enabled = 0;
	fill(input, TEST_FRAMES, 0.25f);
	if (cp_natural_dynamics_init(&dynamics, &config) != CP_OK)
		return 0;
	if (cp_natural_dynamics_process(&dynamics, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	for (index = 0; index < TEST_FRAMES; index++) {
		if (fabsf(input[index] - output[index]) > TEST_EPSILON) {
			printf("test_natural_dynamics: disabled changed audio\n");
			return 0;
		}
	}

	return 1;
}

static int
test_invalid_config(void)
{
	struct cp_natural_dynamics_config config;
	struct cp_natural_dynamics dynamics;

	valid_config(&config, CP_CHANNELS_MONO);
	config.ratio = 0.5f;
	if (cp_natural_dynamics_init(&dynamics, &config) != CP_ERR_RANGE) {
		printf("test_natural_dynamics: invalid ratio accepted\n");
		return 0;
	}

	valid_config(&config, CP_CHANNELS_MONO);
	config.channel_count = 3;
	if (cp_natural_dynamics_init(&dynamics, &config) != CP_ERR_RANGE) {
		printf("test_natural_dynamics: invalid channels accepted\n");
		return 0;
	}

	return 1;
}

static int
test_loud_reduces(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_natural_dynamics_config config;
	struct cp_natural_dynamics dynamics;

	valid_config(&config, CP_CHANNELS_MONO);
	config.attack_ms = 0.0f;
	fill(input, TEST_FRAMES, 0.80f);
	if (cp_natural_dynamics_init(&dynamics, &config) != CP_OK)
		return 0;
	if (cp_natural_dynamics_process(&dynamics, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	if (dynamics.gain >= 1.0f ||
	    dynamics.gain_reduction_db <= 0.1f ||
	    output[TEST_FRAMES - 1] >= input[TEST_FRAMES - 1]) {
		printf("test_natural_dynamics: loud input not reduced\n");
		return 0;
	}

	return 1;
}

static int
test_nonfinite_safe(void)
{
	cp_sample_t input[4] = { NAN, INFINITY, -INFINITY, 0.25f };
	cp_sample_t output[4];
	struct cp_natural_dynamics_config config;
	struct cp_natural_dynamics dynamics;
	size_t index;

	valid_config(&config, CP_CHANNELS_MONO);
	if (cp_natural_dynamics_init(&dynamics, &config) != CP_OK)
		return 0;
	if (cp_natural_dynamics_process(&dynamics, input, output, 4) !=
	    CP_OK)
		return 0;
	for (index = 0; index < 4; index++) {
		if (!isfinite(output[index])) {
			printf("test_natural_dynamics: non-finite output\n");
			return 0;
		}
	}
	if (!isfinite(dynamics.last_rms) || !isfinite(dynamics.gain)) {
		printf("test_natural_dynamics: non-finite state\n");
		return 0;
	}

	return 1;
}

static int
test_quiet_little_change(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_natural_dynamics_config config;
	struct cp_natural_dynamics dynamics;

	valid_config(&config, CP_CHANNELS_MONO);
	config.attack_ms = 0.0f;
	fill(input, TEST_FRAMES, 0.04f);
	if (cp_natural_dynamics_init(&dynamics, &config) != CP_OK)
		return 0;
	if (cp_natural_dynamics_process(&dynamics, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	if (dynamics.gain_reduction_db > 0.1f ||
	    fabsf(output[TEST_FRAMES - 1] - input[TEST_FRAMES - 1]) >
	    TEST_EPSILON) {
		printf("test_natural_dynamics: quiet input changed too much\n");
		return 0;
	}

	return 1;
}

static int
test_stereo_linked(void)
{
	cp_sample_t input[TEST_FRAMES * CP_CHANNELS_STEREO];
	cp_sample_t output[TEST_FRAMES * CP_CHANNELS_STEREO];
	struct cp_natural_dynamics_config config;
	struct cp_natural_dynamics dynamics;
	cp_sample_t left_ratio;
	cp_sample_t right_ratio;
	size_t frame;

	for (frame = 0; frame < TEST_FRAMES; frame++) {
		input[(frame * CP_CHANNELS_STEREO) + 0] = 0.80f;
		input[(frame * CP_CHANNELS_STEREO) + 1] = 0.20f;
	}
	valid_config(&config, CP_CHANNELS_STEREO);
	config.attack_ms = 0.0f;
	if (cp_natural_dynamics_init(&dynamics, &config) != CP_OK)
		return 0;
	if (cp_natural_dynamics_process(&dynamics, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	left_ratio = output[(TEST_FRAMES - 1) * CP_CHANNELS_STEREO] /
	    input[(TEST_FRAMES - 1) * CP_CHANNELS_STEREO];
	right_ratio = output[((TEST_FRAMES - 1) * CP_CHANNELS_STEREO) + 1] /
	    input[((TEST_FRAMES - 1) * CP_CHANNELS_STEREO) + 1];
	if (fabsf(left_ratio - right_ratio) > TEST_EPSILON) {
		printf("test_natural_dynamics: stereo gain not linked\n");
		return 0;
	}

	return 1;
}

static void
valid_config(struct cp_natural_dynamics_config *config, size_t channels)
{
	cp_natural_dynamics_default_config(config);
	config->enabled       = 1;
	config->sample_rate   = TEST_RATE;
	config->channel_count = channels;
	config->threshold_db  = -18.0f;
	config->ratio         = 2.0f;
	config->attack_ms     = 10.0f;
	config->release_ms    = 100.0f;
	config->max_gain_reduction_db = 6.0f;
}

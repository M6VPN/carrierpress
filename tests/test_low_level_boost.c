/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_low_level_boost.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_low_level_boost.h"

#define TEST_FRAMES	480
#define TEST_RATE	48000.0f
#define TEST_EPSILON	0.0001f

static void	fill(cp_sample_t *, size_t, cp_sample_t);
static int	test_disabled_passthrough(void);
static int	test_invalid_config(void);
static int	test_loud_not_boosted(void);
static int	test_nonfinite_safe(void);
static int	test_quiet_boosts(void);
static int	test_silence_not_boosted(void);
static int	test_stereo_linked(void);
static void	valid_config(struct cp_low_level_boost_config *, size_t);

int
main(void)
{
	if (!test_disabled_passthrough())
		return 1;
	if (!test_invalid_config())
		return 1;
	if (!test_quiet_boosts())
		return 1;
	if (!test_loud_not_boosted())
		return 1;
	if (!test_silence_not_boosted())
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
	struct cp_low_level_boost_config config;
	struct cp_low_level_boost boost;
	size_t index;

	valid_config(&config, CP_CHANNELS_MONO);
	config.enabled = 0;
	fill(input, TEST_FRAMES, 0.05f);
	if (cp_low_level_boost_init(&boost, &config) != CP_OK)
		return 0;
	if (cp_low_level_boost_process(&boost, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	for (index = 0; index < TEST_FRAMES; index++) {
		if (fabsf(input[index] - output[index]) > TEST_EPSILON) {
			printf("test_low_level_boost: disabled changed audio\n");
			return 0;
		}
	}

	return 1;
}

static int
test_invalid_config(void)
{
	struct cp_low_level_boost_config config;
	struct cp_low_level_boost boost;

	valid_config(&config, CP_CHANNELS_MONO);
	config.max_boost_db = -1.0f;
	if (cp_low_level_boost_init(&boost, &config) != CP_ERR_RANGE) {
		printf("test_low_level_boost: invalid boost accepted\n");
		return 0;
	}

	valid_config(&config, CP_CHANNELS_MONO);
	config.channel_count = 3;
	if (cp_low_level_boost_init(&boost, &config) != CP_ERR_RANGE) {
		printf("test_low_level_boost: invalid channels accepted\n");
		return 0;
	}

	return 1;
}

static int
test_loud_not_boosted(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_low_level_boost_config config;
	struct cp_low_level_boost boost;

	valid_config(&config, CP_CHANNELS_MONO);
	config.attack_ms = 0.0f;
	fill(input, TEST_FRAMES, 0.40f);
	if (cp_low_level_boost_init(&boost, &config) != CP_OK)
		return 0;
	if (cp_low_level_boost_process(&boost, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	if (boost.gain_db > 0.1f ||
	    fabsf(output[TEST_FRAMES - 1] - input[TEST_FRAMES - 1]) >
	    TEST_EPSILON) {
		printf("test_low_level_boost: loud input boosted\n");
		return 0;
	}

	return 1;
}

static int
test_nonfinite_safe(void)
{
	cp_sample_t input[4] = { NAN, INFINITY, -INFINITY, 0.04f };
	cp_sample_t output[4];
	struct cp_low_level_boost_config config;
	struct cp_low_level_boost boost;
	size_t index;

	valid_config(&config, CP_CHANNELS_MONO);
	if (cp_low_level_boost_init(&boost, &config) != CP_OK)
		return 0;
	if (cp_low_level_boost_process(&boost, input, output, 4) != CP_OK)
		return 0;
	for (index = 0; index < 4; index++) {
		if (!isfinite(output[index])) {
			printf("test_low_level_boost: non-finite output\n");
			return 0;
		}
	}
	if (!isfinite(boost.last_rms) || !isfinite(boost.gain)) {
		printf("test_low_level_boost: non-finite state\n");
		return 0;
	}

	return 1;
}

static int
test_quiet_boosts(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_low_level_boost_config config;
	struct cp_low_level_boost boost;

	valid_config(&config, CP_CHANNELS_MONO);
	config.attack_ms = 0.0f;
	fill(input, TEST_FRAMES, 0.04f);
	if (cp_low_level_boost_init(&boost, &config) != CP_OK)
		return 0;
	if (cp_low_level_boost_process(&boost, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	if (boost.gain <= 1.0f || output[TEST_FRAMES - 1] <=
	    input[TEST_FRAMES - 1] ||
	    boost.gate_state != CP_AGC_STATE_OPEN) {
		printf("test_low_level_boost: quiet input not boosted\n");
		return 0;
	}

	return 1;
}

static int
test_silence_not_boosted(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_low_level_boost_config config;
	struct cp_low_level_boost boost;

	valid_config(&config, CP_CHANNELS_MONO);
	config.attack_ms = 0.0f;
	fill(input, TEST_FRAMES, 0.0f);
	if (cp_low_level_boost_init(&boost, &config) != CP_OK)
		return 0;
	if (cp_low_level_boost_process(&boost, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	if (boost.gain_db > 0.1f ||
	    boost.gate_state != CP_AGC_STATE_SILENT ||
	    fabsf(output[TEST_FRAMES - 1]) > TEST_EPSILON) {
		printf("test_low_level_boost: silence boosted\n");
		return 0;
	}

	return 1;
}

static int
test_stereo_linked(void)
{
	cp_sample_t input[TEST_FRAMES * CP_CHANNELS_STEREO];
	cp_sample_t output[TEST_FRAMES * CP_CHANNELS_STEREO];
	struct cp_low_level_boost_config config;
	struct cp_low_level_boost boost;
	cp_sample_t left_ratio;
	cp_sample_t right_ratio;
	size_t frame;

	for (frame = 0; frame < TEST_FRAMES; frame++) {
		input[(frame * CP_CHANNELS_STEREO) + 0] = 0.03f;
		input[(frame * CP_CHANNELS_STEREO) + 1] = 0.05f;
	}
	valid_config(&config, CP_CHANNELS_STEREO);
	config.attack_ms = 0.0f;
	if (cp_low_level_boost_init(&boost, &config) != CP_OK)
		return 0;
	if (cp_low_level_boost_process(&boost, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	left_ratio = output[(TEST_FRAMES - 1) * CP_CHANNELS_STEREO] /
	    input[(TEST_FRAMES - 1) * CP_CHANNELS_STEREO];
	right_ratio = output[((TEST_FRAMES - 1) * CP_CHANNELS_STEREO) + 1] /
	    input[((TEST_FRAMES - 1) * CP_CHANNELS_STEREO) + 1];
	if (fabsf(left_ratio - right_ratio) > TEST_EPSILON) {
		printf("test_low_level_boost: stereo gain not linked\n");
		return 0;
	}

	return 1;
}

static void
valid_config(struct cp_low_level_boost_config *config, size_t channels)
{
	cp_low_level_boost_default_config(config);
	config->enabled       = 1;
	config->sample_rate   = TEST_RATE;
	config->channel_count = channels;
	config->target_rms    = 0.12f;
	config->max_boost_db  = 6.0f;
	config->attack_ms     = 20.0f;
	config->release_ms    = 100.0f;
	config->gate_threshold_db = -50.0f;
	config->silence_threshold_db = -75.0f;
}

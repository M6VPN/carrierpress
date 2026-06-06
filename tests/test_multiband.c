/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_multiband.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_multiband.h"

#define TEST_FRAMES	4096
#define TEST_RATE	48000.0f
#define TEST_TWO_PI	6.28318530717958647692f

static cp_sample_t input[TEST_FRAMES * CP_MAX_CHANNELS];
static cp_sample_t output[TEST_FRAMES * CP_MAX_CHANNELS];

static void	fill_mix(size_t, size_t, cp_sample_t);
static void	test_config(struct cp_multiband_config *, size_t, size_t, int);
static int	test_disabled_unchanged(void);
static int	test_enabled_bounded(void);
static int	test_gain_reduction_meter(void);
static int	test_invalid_bands(void);
static int	test_nonfinite_safe(void);
static int	test_polish_stage_gentler(void);

int
main(void)
{
	if (!test_disabled_unchanged())
		return 1;
	if (!test_invalid_bands())
		return 1;
	if (!test_enabled_bounded())
		return 1;
	if (!test_gain_reduction_meter())
		return 1;
	if (!test_nonfinite_safe())
		return 1;
	if (!test_polish_stage_gentler())
		return 1;

	return 0;
}

static void
fill_mix(size_t frames, size_t channels, cp_sample_t level)
{
	cp_sample_t phase_a;
	cp_sample_t phase_b;
	cp_sample_t sample;
	size_t channel;
	size_t frame;

	phase_a = 0.0f;
	phase_b = 0.0f;
	for (frame = 0; frame < frames; frame++) {
		sample = ((sinf(phase_a) * 0.60f) +
		    (sinf(phase_b) * 0.40f)) * level;
		for (channel = 0; channel < channels; channel++)
			input[(frame * channels) + channel] = sample;
		phase_a += (TEST_TWO_PI * 180.0f) / TEST_RATE;
		phase_b += (TEST_TWO_PI * 3000.0f) / TEST_RATE;
		if (phase_a >= TEST_TWO_PI)
			phase_a -= TEST_TWO_PI;
		if (phase_b >= TEST_TWO_PI)
			phase_b -= TEST_TWO_PI;
	}
}

static void
test_config(struct cp_multiband_config *config, size_t channels,
	size_t bands, int enabled)
{
	cp_multiband_default_config(config);
	config->sample_rate = TEST_RATE;
	config->channels    = channels;
	config->band_count  = bands;
	config->preset      = CP_MULTIBAND_PRESET_SPEECH;
	config->enabled     = enabled;
}

static int
test_disabled_unchanged(void)
{
	struct cp_multiband_config config;
	struct cp_multiband multiband;
	size_t index;

	fill_mix(TEST_FRAMES, CP_CHANNELS_MONO, 0.50f);
	test_config(&config, CP_CHANNELS_MONO, 3, 0);
	if (cp_multiband_init(&multiband, &config) != CP_OK)
		return 0;
	if (cp_multiband_process(&multiband, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;

	for (index = 0; index < TEST_FRAMES; index++) {
		if (fabsf(input[index] - output[index]) > 0.000001f) {
			printf("test_multiband: disabled changed signal\n");
			return 0;
		}
	}

	return 1;
}

static int
test_enabled_bounded(void)
{
	struct cp_multiband_config config;
	struct cp_multiband multiband;
	size_t index;

	fill_mix(TEST_FRAMES, CP_CHANNELS_STEREO, 0.50f);
	test_config(&config, CP_CHANNELS_STEREO, 4, 1);
	config.preset = CP_MULTIBAND_PRESET_MUSIC;
	if (cp_multiband_init(&multiband, &config) != CP_OK)
		return 0;
	if (cp_multiband_process(&multiband, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;

	for (index = 0; index < TEST_FRAMES * CP_CHANNELS_STEREO; index++) {
		if (!isfinite(output[index]) || fabsf(output[index]) > 2.0f) {
			printf("test_multiband: enabled output invalid\n");
			return 0;
		}
	}

	return 1;
}

static int
test_gain_reduction_meter(void)
{
	struct cp_multiband_config config;
	struct cp_multiband multiband;
	size_t band;

	fill_mix(TEST_FRAMES, CP_CHANNELS_MONO, 1.0f);
	test_config(&config, CP_CHANNELS_MONO, 3, 1);
	if (cp_multiband_init(&multiband, &config) != CP_OK)
		return 0;
	if (cp_multiband_process(&multiband, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;

	for (band = 0; band < multiband.band_count; band++) {
		if (multiband.band_gain_reduction_db[band] > 0.1f)
			return 1;
	}
	printf("test_multiband: no gain reduction meter activity\n");
	return 0;
}

static int
test_invalid_bands(void)
{
	struct cp_multiband_config config;
	struct cp_multiband multiband;

	test_config(&config, CP_CHANNELS_MONO, 1, 1);
	if (cp_multiband_init(&multiband, &config) != CP_ERR_RANGE)
		return 0;

	test_config(&config, CP_CHANNELS_MONO, CP_MULTIBAND_M5_MAX_BANDS + 1,
	    1);
	if (cp_multiband_init(&multiband, &config) != CP_ERR_RANGE)
		return 0;

	return 1;
}

static int
test_polish_stage_gentler(void)
{
	struct cp_multiband_config config;
	struct cp_multiband polish;
	struct cp_multiband primary;
	cp_sample_t polish_gr;
	cp_sample_t primary_gr;
	size_t band;

	fill_mix(TEST_FRAMES, CP_CHANNELS_MONO, 1.0f);
	test_config(&config, CP_CHANNELS_MONO, 3, 1);
	config.stage = CP_MULTIBAND_STAGE_PRIMARY;
	if (cp_multiband_init(&primary, &config) != CP_OK)
		return 0;
	if (cp_multiband_process(&primary, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;

	config.stage = CP_MULTIBAND_STAGE_POLISH;
	if (cp_multiband_init(&polish, &config) != CP_OK)
		return 0;
	if (cp_multiband_process(&polish, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;

	primary_gr = 0.0f;
	polish_gr = 0.0f;
	for (band = 0; band < primary.band_count; band++) {
		primary_gr += primary.band_gain_reduction_db[band];
		polish_gr += polish.band_gain_reduction_db[band];
	}
	if (polish_gr <= 0.0f || polish_gr >= primary_gr) {
		printf("test_multiband: polish stage not gentler\n");
		return 0;
	}

	return 1;
}

static int
test_nonfinite_safe(void)
{
	struct cp_multiband_config config;
	struct cp_multiband multiband;
	size_t index;

	fill_mix(TEST_FRAMES, CP_CHANNELS_MONO, 0.50f);
	input[10] = NAN;
	input[20] = INFINITY;
	input[30] = -INFINITY;
	test_config(&config, CP_CHANNELS_MONO, 2, 1);
	if (cp_multiband_init(&multiband, &config) != CP_OK)
		return 0;
	if (cp_multiband_process(&multiband, input, output,
	    TEST_FRAMES) != CP_OK)
		return 0;
	for (index = 0; index < TEST_FRAMES; index++) {
		if (!isfinite(output[index])) {
			printf("test_multiband: non-finite output\n");
			return 0;
		}
	}

	return 1;
}

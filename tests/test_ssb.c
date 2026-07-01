/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_ssb.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "cp_ssb.h"

#define TEST_FRAMES		4096
#define TEST_RATE		48000.0f
#define TEST_TWO_PI		6.28318530717958647692f

static cp_sample_t	measure_rms(const cp_sample_t *, size_t);
static void		make_sine(cp_sample_t *, size_t, cp_sample_t,
			    cp_sample_t, size_t);
static int		test_disabled_passthrough(void);
static int		test_highpass_reduces_low_tone(void);
static int		test_invalid_config(void);
static int		test_lowpass_reduces_high_tone(void);
static int		test_peak_limit(void);
static int		test_phase_finite(void);
static int		test_presets(void);
static int		test_stereo_stable(void);

int
main(void)
{
	if (!test_disabled_passthrough())
		return 1;
	if (!test_lowpass_reduces_high_tone())
		return 1;
	if (!test_highpass_reduces_low_tone())
		return 1;
	if (!test_invalid_config())
		return 1;
	if (!test_peak_limit())
		return 1;
	if (!test_phase_finite())
		return 1;
	if (!test_presets())
		return 1;
	if (!test_stereo_stable())
		return 1;

	return 0;
}

static cp_sample_t
measure_rms(const cp_sample_t *samples, size_t count)
{
	double sum;
	size_t index;

	sum = 0.0;
	for (index = 0; index < count; index++)
		sum += (double)samples[index] * (double)samples[index];

	return (cp_sample_t)sqrt(sum / (double)count);
}

static void
make_sine(cp_sample_t *samples, size_t frames, cp_sample_t frequency,
	cp_sample_t amplitude, size_t channels)
{
	cp_sample_t value;
	size_t channel;
	size_t frame;

	for (frame = 0; frame < frames; frame++) {
		value = amplitude * sinf((TEST_TWO_PI * frequency *
		    (cp_sample_t)frame) / TEST_RATE);
		for (channel = 0; channel < channels; channel++)
			samples[(frame * channels) + channel] = value;
	}
}

static int
test_disabled_passthrough(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_ssb_config config;
	struct cp_ssb ssb;
	size_t frame;

	cp_ssb_default_config(&config);
	config.enabled = 0;
	if (cp_ssb_init(&ssb, &config) != CP_OK)
		return 0;
	make_sine(input, TEST_FRAMES, 1000.0f, 0.25f, CP_CHANNELS_MONO);
	if (cp_ssb_process(&ssb, input, output, TEST_FRAMES) != CP_OK)
		return 0;
	for (frame = 0; frame < TEST_FRAMES; frame++) {
		if (fabsf(input[frame] - output[frame]) > 0.000001f) {
			printf("test_ssb: disabled path changed samples\n");
			return 0;
		}
	}

	return 1;
}

static int
test_highpass_reduces_low_tone(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_ssb_config config;
	struct cp_ssb ssb;

	cp_ssb_default_config(&config);
	config.enabled = 1;
	config.highpass_hz = 250.0f;
	config.lowpass_hz = 3000.0f;
	if (cp_ssb_init(&ssb, &config) != CP_OK)
		return 0;
	make_sine(input, TEST_FRAMES, 50.0f, 0.50f, CP_CHANNELS_MONO);
	if (cp_ssb_process(&ssb, input, output, TEST_FRAMES) != CP_OK)
		return 0;
	if (measure_rms(output + (TEST_FRAMES / 2), TEST_FRAMES / 2) >=
	    measure_rms(input + (TEST_FRAMES / 2), TEST_FRAMES / 2) * 0.60f) {
		printf("test_ssb: highpass did not reduce low tone\n");
		return 0;
	}

	return 1;
}

static int
test_invalid_config(void)
{
	struct cp_ssb_config config;
	struct cp_ssb ssb;

	cp_ssb_default_config(&config);
	config.lowpass_hz = TEST_RATE;
	if (cp_ssb_init(&ssb, &config) != CP_ERR_RANGE) {
		printf("test_ssb: invalid lowpass accepted\n");
		return 0;
	}

	cp_ssb_default_config(&config);
	config.highpass_hz = 3000.0f;
	config.lowpass_hz = 2500.0f;
	if (cp_ssb_init(&ssb, &config) != CP_ERR_RANGE) {
		printf("test_ssb: highpass above lowpass accepted\n");
		return 0;
	}

	cp_ssb_default_config(&config);
	config.peak_limit = 2.0f;
	if (cp_ssb_init(&ssb, &config) != CP_ERR_RANGE) {
		printf("test_ssb: invalid peak accepted\n");
		return 0;
	}

	cp_ssb_default_config(&config);
	config.channel_count = 3;
	if (cp_ssb_init(&ssb, &config) != CP_ERR_RANGE) {
		printf("test_ssb: invalid channel count accepted\n");
		return 0;
	}

	return 1;
}

static int
test_lowpass_reduces_high_tone(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_ssb_config config;
	struct cp_ssb ssb;

	cp_ssb_default_config(&config);
	config.enabled = 1;
	config.highpass_hz = 100.0f;
	config.lowpass_hz = 2500.0f;
	if (cp_ssb_init(&ssb, &config) != CP_OK)
		return 0;
	make_sine(input, TEST_FRAMES, 6000.0f, 0.50f, CP_CHANNELS_MONO);
	if (cp_ssb_process(&ssb, input, output, TEST_FRAMES) != CP_OK)
		return 0;
	if (measure_rms(output + (TEST_FRAMES / 2), TEST_FRAMES / 2) >=
	    measure_rms(input + (TEST_FRAMES / 2), TEST_FRAMES / 2) * 0.35f) {
		printf("test_ssb: lowpass did not reduce high tone\n");
		return 0;
	}

	return 1;
}

static int
test_peak_limit(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_ssb_config config;
	struct cp_ssb ssb;
	size_t frame;

	cp_ssb_default_config(&config);
	config.enabled = 1;
	config.highpass_hz = 80.0f;
	config.lowpass_hz = 6000.0f;
	config.phase_rotator_enabled = 0;
	config.phase_rotator_stages = 0;
	config.peak_limit = 0.25f;
	if (cp_ssb_init(&ssb, &config) != CP_OK)
		return 0;
	for (frame = 0; frame < TEST_FRAMES; frame++)
		input[frame] = frame % 2 == 0 ? 2.0f : -2.0f;
	if (cp_ssb_process(&ssb, input, output, TEST_FRAMES) != CP_OK)
		return 0;
	for (frame = TEST_FRAMES / 2; frame < TEST_FRAMES; frame++) {
		if (output[frame] > 0.2501f || output[frame] < -0.2501f) {
			printf("test_ssb: peak limit exceeded\n");
			return 0;
		}
	}

	return 1;
}

static int
test_phase_finite(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_ssb_config config;
	struct cp_ssb ssb;
	size_t frame;

	cp_ssb_default_config(&config);
	config.enabled = 1;
	config.phase_rotator_enabled = 1;
	config.phase_rotator_stages = CP_SSB_MAX_PHASE_STAGES;
	if (cp_ssb_init(&ssb, &config) != CP_OK)
		return 0;
	make_sine(input, TEST_FRAMES, 1000.0f, 0.75f, CP_CHANNELS_MONO);
	if (cp_ssb_process(&ssb, input, output, TEST_FRAMES) != CP_OK)
		return 0;
	for (frame = 0; frame < TEST_FRAMES; frame++) {
		if (!isfinite(output[frame])) {
			printf("test_ssb: phase output not finite\n");
			return 0;
		}
	}

	return 1;
}

static int
test_presets(void)
{
	struct cp_ssb_config config;

	cp_ssb_default_config(&config);
	if (cp_ssb_apply_preset(&config, "ssb-narrow") != CP_OK)
		return 0;
	if (strcmp(config.preset_name, "ssb-narrow") != 0 ||
	    config.lowpass_hz > 2600.0f) {
		printf("test_ssb: narrow preset mismatch\n");
		return 0;
	}
	if (cp_ssb_apply_preset(&config, "bad") != CP_ERR_RANGE) {
		printf("test_ssb: bad preset accepted\n");
		return 0;
	}
	if (cp_ssb_apply_preset(&config, "hf-ssb-voice") != CP_OK ||
	    strcmp(config.preset_name, "hf-ssb-voice") != 0 ||
	    config.highpass_hz != 150.0f ||
	    config.lowpass_hz != 2700.0f ||
	    config.peak_limit > 0.90f) {
		printf("test_ssb: hf voice preset mismatch\n");
		return 0;
	}
	if (cp_ssb_apply_preset(&config, "hf-ssb-narrow") != CP_OK ||
	    config.highpass_hz != 200.0f ||
	    config.lowpass_hz != 2400.0f) {
		printf("test_ssb: hf narrow preset mismatch\n");
		return 0;
	}
	if (cp_ssb_apply_preset(&config, "vhf-fm-voice") != CP_OK ||
	    config.highpass_hz < 100.0f ||
	    config.lowpass_hz != 3000.0f) {
		printf("test_ssb: vhf fm preset mismatch\n");
		return 0;
	}

	return 1;
}

static int
test_stereo_stable(void)
{
	cp_sample_t input[TEST_FRAMES * CP_CHANNELS_STEREO];
	cp_sample_t output[TEST_FRAMES * CP_CHANNELS_STEREO];
	struct cp_ssb_config config;
	struct cp_ssb ssb;
	size_t index;

	cp_ssb_default_config(&config);
	config.enabled = 1;
	config.channel_count = CP_CHANNELS_STEREO;
	if (cp_ssb_init(&ssb, &config) != CP_OK)
		return 0;
	make_sine(input, TEST_FRAMES, 1000.0f, 0.25f, CP_CHANNELS_STEREO);
	if (cp_ssb_process(&ssb, input, output, TEST_FRAMES) != CP_OK)
		return 0;
	for (index = 0; index < TEST_FRAMES * CP_CHANNELS_STEREO; index++) {
		if (!isfinite(output[index]) || fabsf(output[index]) > 1.0f) {
			printf("test_ssb: stereo output unstable\n");
			return 0;
		}
	}

	return 1;
}

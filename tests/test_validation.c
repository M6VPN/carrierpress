/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_validation.c */

#include <sys/types.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "cp_block.h"

#define CP_VALIDATION_BLOCK_FRAMES	256
#define CP_VALIDATION_TOTAL_FRAMES	8192
#define CP_VALIDATION_RATE		(48000.0f)
#define CP_VALIDATION_PI		(3.14159265358979323846f)
#define CP_VALIDATION_LIMIT_EPSILON	(0.002f)

enum cp_validation_fixture {
	CP_VALIDATION_SILENCE = 0,
	CP_VALIDATION_TONE,
	CP_VALIDATION_SWEEP,
	CP_VALIDATION_NOISE,
	CP_VALIDATION_CLIPPED,
	CP_VALIDATION_DC_OFFSET,
	CP_VALIDATION_STEREO_IMBALANCE,
	CP_VALIDATION_HUM
};

enum cp_validation_mode {
	CP_VALIDATION_MODE_DEFAULT = 0,
	CP_VALIDATION_MODE_AM,
	CP_VALIDATION_MODE_SSB
};

struct cp_validation_metrics {
	double input_square;
	double output_square;
	cp_sample_t input_peak;
	cp_sample_t output_peak;
	cp_sample_t output_min;
	cp_sample_t output_max;
	int finite;
};

static cp_sample_t	cp_validation_abs(cp_sample_t);
static void		cp_validation_config(struct cp_block_config *,
			    enum cp_validation_mode);
static int		cp_validation_generate(enum cp_validation_fixture,
			    cp_sample_t *, size_t, size_t);
static cp_sample_t	cp_validation_noise(size_t, size_t);
static const char	*cp_validation_fixture_name(
			    enum cp_validation_fixture);
static const char	*cp_validation_mode_name(enum cp_validation_mode);
static int		cp_validation_run(enum cp_validation_fixture,
			    enum cp_validation_mode);
static void		cp_validation_update(struct cp_validation_metrics *,
			    const cp_sample_t *, const cp_sample_t *, size_t);

int
main(void)
{
	size_t fixture;
	size_t mode;

	for (mode = CP_VALIDATION_MODE_DEFAULT;
	    mode <= CP_VALIDATION_MODE_SSB; mode++) {
		for (fixture = CP_VALIDATION_SILENCE;
		    fixture <= CP_VALIDATION_HUM; fixture++) {
			if (!cp_validation_run(
			    (enum cp_validation_fixture)fixture,
			    (enum cp_validation_mode)mode))
				return 1;
		}
	}

	return 0;
}

static cp_sample_t
cp_validation_abs(cp_sample_t sample)
{
	return sample < 0.0f ? -sample : sample;
}

static void
cp_validation_config(struct cp_block_config *config,
	enum cp_validation_mode mode)
{
	cp_block_default_config(config, CP_CHANNELS_STEREO);
	config->sample_rate = CP_VALIDATION_RATE;
	config->limiter_ceiling = CP_DEFAULT_CEILING;

	if (mode == CP_VALIDATION_MODE_AM) {
		config->dehummer_enabled = 1;
		config->hum_base_frequency = 50.0f;
		config->hum_harmonic_count = 4;
		config->multiband_enabled = 1;
		config->multiband_band_count = 3;
		config->multiband_preset = CP_MULTIBAND_PRESET_SPEECH;
		(void)cp_am_apply_preset(&config->am_config, "am-shortwave");
		config->am_config.enabled = 1;
	}
	if (mode == CP_VALIDATION_MODE_SSB) {
		config->dehummer_enabled = 1;
		config->hum_base_frequency = 50.0f;
		config->hum_harmonic_count = 4;
		config->multiband_enabled = 1;
		config->multiband_band_count = 3;
		config->multiband_preset = CP_MULTIBAND_PRESET_SPEECH;
		(void)cp_ssb_apply_preset(&config->ssb_config, "ssb-narrow");
		config->ssb_config.enabled = 1;
	}
}

static int
cp_validation_generate(enum cp_validation_fixture fixture, cp_sample_t *buffer,
	size_t offset, size_t frames)
{
	cp_sample_t left;
	cp_sample_t right;
	cp_sample_t value;
	float phase;
	float position;
	float frequency;
	size_t channel;
	size_t frame;
	size_t index;

	if (buffer == NULL)
		return 0;

	for (frame = 0; frame < frames; frame++) {
		index = offset + frame;
		position = (float)index / (float)CP_VALIDATION_TOTAL_FRAMES;
		frequency = 100.0f + (4200.0f * position);
		phase = 2.0f * CP_VALIDATION_PI * (float)index /
		    CP_VALIDATION_RATE;

		switch (fixture) {
		case CP_VALIDATION_SILENCE:
			left = 0.0f;
			right = 0.0f;
			break;
		case CP_VALIDATION_TONE:
			left = 0.25f * sinf(phase * 1000.0f);
			right = left;
			break;
		case CP_VALIDATION_SWEEP:
			left = 0.22f * sinf(phase * frequency);
			right = left;
			break;
		case CP_VALIDATION_NOISE:
			left = 0.15f * cp_validation_noise(index, 0);
			right = 0.15f * cp_validation_noise(index, 1);
			break;
		case CP_VALIDATION_CLIPPED:
			value = 1.4f * sinf(phase * 700.0f);
			if (value > 0.45f)
				value = 0.45f;
			if (value < -0.45f)
				value = -0.45f;
			left = value;
			right = value;
			break;
		case CP_VALIDATION_DC_OFFSET:
			left = 0.20f + (0.08f * sinf(phase * 400.0f));
			right = left;
			break;
		case CP_VALIDATION_STEREO_IMBALANCE:
			left = 0.35f * sinf(phase * 1000.0f);
			right = 0.04f * sinf(phase * 1000.0f);
			break;
		case CP_VALIDATION_HUM:
			left = (0.16f * sinf(phase * 50.0f)) +
			    (0.12f * sinf(phase * 1000.0f));
			right = left;
			break;
		default:
			return 0;
		}

		for (channel = 0; channel < CP_CHANNELS_STEREO; channel++)
			buffer[(frame * CP_CHANNELS_STEREO) + channel] =
			    channel == 0 ? left : right;
	}

	return 1;
}

static cp_sample_t
cp_validation_noise(size_t frame, size_t channel)
{
	uint32_t value;

	value = (uint32_t)(frame + (channel * 257u));
	value ^= value >> 16;
	value *= 2246822519u;
	value ^= value >> 13;
	value *= 3266489917u;
	value ^= value >> 16;

	return ((cp_sample_t)(value & 0xffffu) / 32768.0f) - 1.0f;
}

static const char *
cp_validation_fixture_name(enum cp_validation_fixture fixture)
{
	switch (fixture) {
	case CP_VALIDATION_SILENCE:
		return "silence";
	case CP_VALIDATION_TONE:
		return "tone";
	case CP_VALIDATION_SWEEP:
		return "sweep";
	case CP_VALIDATION_NOISE:
		return "noise";
	case CP_VALIDATION_CLIPPED:
		return "clipped";
	case CP_VALIDATION_DC_OFFSET:
		return "dc-offset";
	case CP_VALIDATION_STEREO_IMBALANCE:
		return "stereo-imbalance";
	case CP_VALIDATION_HUM:
		return "hum";
	default:
		return "unknown";
	}
}

static const char *
cp_validation_mode_name(enum cp_validation_mode mode)
{
	switch (mode) {
	case CP_VALIDATION_MODE_DEFAULT:
		return "default";
	case CP_VALIDATION_MODE_AM:
		return "am-shortwave";
	case CP_VALIDATION_MODE_SSB:
		return "ssb-narrow";
	default:
		return "unknown";
	}
}

static int
cp_validation_run(enum cp_validation_fixture fixture,
	enum cp_validation_mode mode)
{
	cp_sample_t input[CP_VALIDATION_BLOCK_FRAMES * CP_CHANNELS_STEREO];
	cp_sample_t output[CP_VALIDATION_BLOCK_FRAMES * CP_CHANNELS_STEREO];
	cp_sample_t scratch[CP_VALIDATION_BLOCK_FRAMES * CP_CHANNELS_STEREO];
	struct cp_block_config config;
	struct cp_block_processor processor;
	struct cp_validation_metrics metrics;
	const char *reason;
	size_t offset;
	size_t frames;
	int status;

	cp_validation_config(&config, mode);
	status = cp_block_init(&processor, &config);
	if (status != CP_OK) {
		printf("validation case=%s mode=%s status=fail reason=init\n",
		    cp_validation_fixture_name(fixture),
		    cp_validation_mode_name(mode));
		return 0;
	}

	metrics.input_square = 0.0;
	metrics.output_square = 0.0;
	metrics.input_peak = 0.0f;
	metrics.output_peak = 0.0f;
	metrics.output_min = 0.0f;
	metrics.output_max = 0.0f;
	metrics.finite = 1;

	for (offset = 0; offset < CP_VALIDATION_TOTAL_FRAMES;
	    offset += CP_VALIDATION_BLOCK_FRAMES) {
		frames = CP_VALIDATION_BLOCK_FRAMES;
		if (offset + frames > CP_VALIDATION_TOTAL_FRAMES)
			frames = CP_VALIDATION_TOTAL_FRAMES - offset;
		if (!cp_validation_generate(fixture, input, offset, frames))
			return 0;
		status = cp_block_process(&processor, input, output, scratch,
		    frames * CP_CHANNELS_STEREO, frames);
		if (status != CP_OK) {
			printf("validation case=%s mode=%s status=fail "
			    "reason=process\n",
			    cp_validation_fixture_name(fixture),
			    cp_validation_mode_name(mode));
			return 0;
		}
		cp_validation_update(&metrics, input, output,
		    frames * CP_CHANNELS_STEREO);
	}

	metrics.input_square = sqrt(metrics.input_square /
	    (double)(CP_VALIDATION_TOTAL_FRAMES * CP_CHANNELS_STEREO));
	metrics.output_square = sqrt(metrics.output_square /
	    (double)(CP_VALIDATION_TOTAL_FRAMES * CP_CHANNELS_STEREO));

	reason = NULL;
	if (!metrics.finite)
		reason = "non-finite";
	if (reason == NULL && metrics.output_peak > CP_DEFAULT_CEILING +
	    CP_VALIDATION_LIMIT_EPSILON)
		reason = "limiter";
	if (reason == NULL && mode == CP_VALIDATION_MODE_AM &&
	    metrics.output_min < -config.am_config.negative_peak_limit -
	    CP_VALIDATION_LIMIT_EPSILON)
		reason = "am-negative-peak";
	if (reason == NULL && mode == CP_VALIDATION_MODE_SSB &&
	    metrics.output_peak > config.ssb_config.peak_limit +
	    CP_VALIDATION_LIMIT_EPSILON)
		reason = "ssb-peak";
	if (reason != NULL) {
		printf("validation case=%s mode=%s input_peak=%0.6f "
		    "input_rms=%0.6f output_peak=%0.6f output_rms=%0.6f "
		    "output_min=%0.6f output_max=%0.6f status=fail "
		    "reason=%s\n",
		    cp_validation_fixture_name(fixture),
		    cp_validation_mode_name(mode), metrics.input_peak,
		    metrics.input_square, metrics.output_peak,
		    metrics.output_square, metrics.output_min,
		    metrics.output_max, reason);
		return 0;
	}

	printf("validation case=%s mode=%s input_peak=%0.6f "
	    "input_rms=%0.6f output_peak=%0.6f output_rms=%0.6f "
	    "output_min=%0.6f output_max=%0.6f status=pass\n",
	    cp_validation_fixture_name(fixture),
	    cp_validation_mode_name(mode), metrics.input_peak,
	    metrics.input_square, metrics.output_peak, metrics.output_square,
	    metrics.output_min, metrics.output_max);

	return 1;
}

static void
cp_validation_update(struct cp_validation_metrics *metrics,
	const cp_sample_t *input, const cp_sample_t *output, size_t samples)
{
	cp_sample_t input_abs;
	cp_sample_t output_abs;
	size_t sample;

	for (sample = 0; sample < samples; sample++) {
		if (!isfinite(input[sample]) || !isfinite(output[sample])) {
			metrics->finite = 0;
			continue;
		}
		input_abs = cp_validation_abs(input[sample]);
		output_abs = cp_validation_abs(output[sample]);
		if (input_abs > metrics->input_peak)
			metrics->input_peak = input_abs;
		if (output_abs > metrics->output_peak)
			metrics->output_peak = output_abs;
		if (output[sample] < metrics->output_min)
			metrics->output_min = output[sample];
		if (output[sample] > metrics->output_max)
			metrics->output_max = output[sample];
		metrics->input_square +=
		    (double)input[sample] * (double)input[sample];
		metrics->output_square +=
		    (double)output[sample] * (double)output[sample];
	}
}

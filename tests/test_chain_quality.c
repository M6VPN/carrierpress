/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_chain_quality.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_block.h"

#define CQ_BLOCK_FRAMES		256
#define CQ_TOTAL_FRAMES		8192
#define CQ_RATE			(48000.0f)
#define CQ_PI			(3.14159265358979323846f)
#define CQ_LIMIT_EPSILON	(0.002f)
#define CQ_SILENCE_EPSILON	(0.0001f)

enum cq_fixture {
	CQ_FIXTURE_SILENCE = 0,
	CQ_FIXTURE_LOW_PROGRAM,
	CQ_FIXTURE_STEPPED_SPEECH,
	CQ_FIXTURE_MUSIC_MIX,
	CQ_FIXTURE_BURST,
	CQ_FIXTURE_CLIPPED,
	CQ_FIXTURE_DC_OFFSET,
	CQ_FIXTURE_HUM_50,
	CQ_FIXTURE_HUM_60,
	CQ_FIXTURE_HIGH_TONE,
	CQ_FIXTURE_STEREO_IMBALANCE
};

enum cq_profile {
	CQ_PROFILE_DEFAULT = 0,
	CQ_PROFILE_DEHUM_50,
	CQ_PROFILE_DEHUM_60,
	CQ_PROFILE_MB_BASS,
	CQ_PROFILE_AM_VOICE,
	CQ_PROFILE_AM_SHORTWAVE,
	CQ_PROFILE_AM_WIDE,
	CQ_PROFILE_SSB_SPEECH,
	CQ_PROFILE_SSB_NARROW,
	CQ_PROFILE_SSB_WIDE
};

struct cq_metrics {
	double input_square;
	double output_square;
	double input_sum;
	double output_sum;
	double output_left_square;
	double output_right_square;
	cp_sample_t input_peak;
	cp_sample_t output_peak;
	cp_sample_t output_min;
	cp_sample_t output_max;
	cp_sample_t final_gain;
	enum cp_agc_gate_state final_gate_state;
	int finite;
};

static cp_sample_t	cq_abs(cp_sample_t);
static void		cq_config(struct cp_block_config *, enum cq_profile);
static int		cq_generate(enum cq_fixture, cp_sample_t *, size_t,
			    size_t);
static int		cq_process(enum cq_profile, enum cq_fixture,
			    struct cq_metrics *, struct cp_block_config *);
static const char	*cq_fixture_name(enum cq_fixture);
static int		cq_profile_is_am(enum cq_profile);
static int		cq_profile_is_ssb(enum cq_profile);
static const char	*cq_profile_name(enum cq_profile);
static int		cq_run_matrix(void);
static int		cq_test_agc_limits(void);
static int		cq_test_dc_offset_reduction(void);
static int		cq_test_dehummer_reduction(void);
static int		cq_test_lowpass_profiles(void);
static int		cq_test_silence_stability(void);
static int		cq_test_stereo_stability(void);
static void		cq_update(struct cq_metrics *, const cp_sample_t *,
			    const cp_sample_t *, size_t);

int
main(void)
{
	if (!cq_run_matrix())
		return 1;
	if (!cq_test_silence_stability())
		return 1;
	if (!cq_test_dc_offset_reduction())
		return 1;
	if (!cq_test_dehummer_reduction())
		return 1;
	if (!cq_test_lowpass_profiles())
		return 1;
	if (!cq_test_stereo_stability())
		return 1;
	if (!cq_test_agc_limits())
		return 1;

	printf("chain_quality profiles=%d fixtures=%d status=pass\n",
	    CQ_PROFILE_SSB_WIDE + 1, CQ_FIXTURE_STEREO_IMBALANCE + 1);
	return 0;
}

static cp_sample_t
cq_abs(cp_sample_t sample)
{
	return sample < 0.0f ? -sample : sample;
}

static void
cq_config(struct cp_block_config *config, enum cq_profile profile)
{
	cp_block_default_config(config, CP_CHANNELS_STEREO);
	config->sample_rate = CQ_RATE;
	config->limiter_ceiling = CP_DEFAULT_CEILING;

	switch (profile) {
	case CQ_PROFILE_DEFAULT:
		break;
	case CQ_PROFILE_DEHUM_50:
		config->dehummer_enabled = 1;
		config->hum_base_frequency = 50.0f;
		config->hum_harmonic_count = 4;
		break;
	case CQ_PROFILE_DEHUM_60:
		config->dehummer_enabled = 1;
		config->hum_base_frequency = 60.0f;
		config->hum_harmonic_count = 4;
		break;
	case CQ_PROFILE_MB_BASS:
		config->multiband_enabled = 1;
		config->multiband_band_count = 3;
		config->multiband_preset = CP_MULTIBAND_PRESET_MUSIC;
		config->bass_eq_config.enabled = 1;
		(void)cp_bass_eq_apply_preset(&config->bass_eq_config,
		    "music");
		break;
	case CQ_PROFILE_AM_VOICE:
	case CQ_PROFILE_AM_SHORTWAVE:
	case CQ_PROFILE_AM_WIDE:
		config->dehummer_enabled = 1;
		config->hum_base_frequency = 50.0f;
		config->hum_harmonic_count = 4;
		config->multiband_enabled = 1;
		config->multiband_band_count = 3;
		config->multiband_preset = CP_MULTIBAND_PRESET_SPEECH;
		config->bass_eq_config.enabled = 1;
		(void)cp_bass_eq_apply_preset(&config->bass_eq_config,
		    "speech");
		if (profile == CQ_PROFILE_AM_VOICE)
			(void)cp_am_apply_preset(&config->am_config,
			    "am-voice");
		else if (profile == CQ_PROFILE_AM_WIDE)
			(void)cp_am_apply_preset(&config->am_config,
			    "am-wide");
		else
			(void)cp_am_apply_preset(&config->am_config,
			    "am-shortwave");
		config->am_config.enabled = 1;
		break;
	case CQ_PROFILE_SSB_SPEECH:
	case CQ_PROFILE_SSB_NARROW:
	case CQ_PROFILE_SSB_WIDE:
		config->dehummer_enabled = 1;
		config->hum_base_frequency = 50.0f;
		config->hum_harmonic_count = 4;
		config->multiband_enabled = 1;
		config->multiband_band_count = 3;
		config->multiband_preset = CP_MULTIBAND_PRESET_SPEECH;
		config->bass_eq_config.enabled = 1;
		(void)cp_bass_eq_apply_preset(&config->bass_eq_config,
		    "speech");
		if (profile == CQ_PROFILE_SSB_NARROW)
			(void)cp_ssb_apply_preset(&config->ssb_config,
			    "ssb-narrow");
		else if (profile == CQ_PROFILE_SSB_WIDE)
			(void)cp_ssb_apply_preset(&config->ssb_config,
			    "ssb-wide");
		else
			(void)cp_ssb_apply_preset(&config->ssb_config,
			    "ssb-speech");
		config->ssb_config.enabled = 1;
		break;
	default:
		break;
	}
}

static int
cq_generate(enum cq_fixture fixture, cp_sample_t *buffer, size_t offset,
	size_t frames)
{
	cp_sample_t left;
	cp_sample_t phase;
	cp_sample_t right;
	cp_sample_t value;
	size_t channel;
	size_t frame;
	size_t index;

	if (buffer == NULL)
		return 0;

	for (frame = 0; frame < frames; frame++) {
		index = offset + frame;
		phase = 2.0f * CQ_PI * (cp_sample_t)index / CQ_RATE;
		switch (fixture) {
		case CQ_FIXTURE_SILENCE:
			left = 0.0f;
			right = 0.0f;
			break;
		case CQ_FIXTURE_LOW_PROGRAM:
			left = (0.025f * sinf(phase * 700.0f)) +
			    (0.015f * sinf(phase * 1400.0f));
			right = left;
			break;
		case CQ_FIXTURE_STEPPED_SPEECH:
			if (index < CQ_TOTAL_FRAMES / 4)
				value = 0.03f;
			else if (index < CQ_TOTAL_FRAMES / 2)
				value = 0.20f;
			else if (index < (CQ_TOTAL_FRAMES * 3) / 4)
				value = 0.45f;
			else
				value = 0.08f;
			left = value * sinf(phase * 650.0f);
			right = left;
			break;
		case CQ_FIXTURE_MUSIC_MIX:
			left = (0.12f * sinf(phase * 110.0f)) +
			    (0.08f * sinf(phase * 220.0f)) +
			    (0.06f * sinf(phase * 440.0f)) +
			    (0.04f * sinf(phase * 880.0f)) +
			    (0.02f * sinf(phase * 1760.0f));
			right = left;
			break;
		case CQ_FIXTURE_BURST:
			if ((index % 1024u) < 40u)
				left = 0.85f * sinf(phase * 1000.0f);
			else
				left = 0.03f * sinf(phase * 1000.0f);
			right = left;
			break;
		case CQ_FIXTURE_CLIPPED:
			value = 1.3f * sinf(phase * 700.0f);
			if (value > 0.45f)
				value = 0.45f;
			if (value < -0.45f)
				value = -0.45f;
			left = value;
			right = value;
			break;
		case CQ_FIXTURE_DC_OFFSET:
			left = 0.22f + (0.04f * sinf(phase * 400.0f));
			right = left;
			break;
		case CQ_FIXTURE_HUM_50:
			left = 0.20f * sinf(phase * 50.0f);
			right = left;
			break;
		case CQ_FIXTURE_HUM_60:
			left = 0.20f * sinf(phase * 60.0f);
			right = left;
			break;
		case CQ_FIXTURE_HIGH_TONE:
			left = 0.20f * sinf(phase * 10000.0f);
			right = left;
			break;
		case CQ_FIXTURE_STEREO_IMBALANCE:
			left = 0.30f * sinf(phase * 1000.0f);
			right = 0.03f * sinf(phase * 1000.0f);
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

static int
cq_process(enum cq_profile profile, enum cq_fixture fixture,
	struct cq_metrics *metrics, struct cp_block_config *used_config)
{
	cp_sample_t input[CQ_BLOCK_FRAMES * CP_CHANNELS_STEREO];
	cp_sample_t output[CQ_BLOCK_FRAMES * CP_CHANNELS_STEREO];
	cp_sample_t scratch[CQ_BLOCK_FRAMES * CP_CHANNELS_STEREO];
	struct cp_block_config config;
	struct cp_block_processor processor;
	size_t frames;
	size_t offset;
	int status;

	if (metrics == NULL)
		return 0;

	cq_config(&config, profile);
	status = cp_block_init(&processor, &config);
	if (status != CP_OK) {
		printf("chain_quality profile=%s fixture=%s status=fail "
		    "reason=init code=%d\n", cq_profile_name(profile),
		    cq_fixture_name(fixture), status);
		return 0;
	}

	metrics->input_square = 0.0;
	metrics->output_square = 0.0;
	metrics->input_sum = 0.0;
	metrics->output_sum = 0.0;
	metrics->output_left_square = 0.0;
	metrics->output_right_square = 0.0;
	metrics->input_peak = 0.0f;
	metrics->output_peak = 0.0f;
	metrics->output_min = 0.0f;
	metrics->output_max = 0.0f;
	metrics->final_gain = 1.0f;
	metrics->final_gate_state = CP_AGC_STATE_OPEN;
	metrics->finite = 1;

	for (offset = 0; offset < CQ_TOTAL_FRAMES; offset += CQ_BLOCK_FRAMES) {
		frames = CQ_BLOCK_FRAMES;
		if (offset + frames > CQ_TOTAL_FRAMES)
			frames = CQ_TOTAL_FRAMES - offset;
		if (!cq_generate(fixture, input, offset, frames))
			return 0;
		status = cp_block_process(&processor, input, output, scratch,
		    frames * CP_CHANNELS_STEREO, frames);
		if (status != CP_OK) {
			printf("chain_quality profile=%s fixture=%s "
			    "status=fail reason=process code=%d\n",
			    cq_profile_name(profile), cq_fixture_name(fixture),
			    status);
			return 0;
		}
		cq_update(metrics, input, output, frames);
	}

	metrics->input_square = sqrt(metrics->input_square /
	    (double)(CQ_TOTAL_FRAMES * CP_CHANNELS_STEREO));
	metrics->output_square = sqrt(metrics->output_square /
	    (double)(CQ_TOTAL_FRAMES * CP_CHANNELS_STEREO));
	metrics->input_sum = metrics->input_sum /
	    (double)(CQ_TOTAL_FRAMES * CP_CHANNELS_STEREO);
	metrics->output_sum = metrics->output_sum /
	    (double)(CQ_TOTAL_FRAMES * CP_CHANNELS_STEREO);
	metrics->output_left_square = sqrt(metrics->output_left_square /
	    (double)CQ_TOTAL_FRAMES);
	metrics->output_right_square = sqrt(metrics->output_right_square /
	    (double)CQ_TOTAL_FRAMES);
	metrics->final_gain = processor.agc.gain;
	metrics->final_gate_state = processor.agc.gate_state;
	if (used_config != NULL)
		*used_config = config;

	return 1;
}

static const char *
cq_fixture_name(enum cq_fixture fixture)
{
	switch (fixture) {
	case CQ_FIXTURE_SILENCE:
		return "silence";
	case CQ_FIXTURE_LOW_PROGRAM:
		return "low-program";
	case CQ_FIXTURE_STEPPED_SPEECH:
		return "stepped-speech";
	case CQ_FIXTURE_MUSIC_MIX:
		return "music-mix";
	case CQ_FIXTURE_BURST:
		return "burst";
	case CQ_FIXTURE_CLIPPED:
		return "clipped";
	case CQ_FIXTURE_DC_OFFSET:
		return "dc-offset";
	case CQ_FIXTURE_HUM_50:
		return "hum-50";
	case CQ_FIXTURE_HUM_60:
		return "hum-60";
	case CQ_FIXTURE_HIGH_TONE:
		return "high-tone";
	case CQ_FIXTURE_STEREO_IMBALANCE:
		return "stereo-imbalance";
	default:
		return "unknown";
	}
}

static int
cq_profile_is_am(enum cq_profile profile)
{
	return profile == CQ_PROFILE_AM_VOICE ||
	    profile == CQ_PROFILE_AM_SHORTWAVE ||
	    profile == CQ_PROFILE_AM_WIDE;
}

static int
cq_profile_is_ssb(enum cq_profile profile)
{
	return profile == CQ_PROFILE_SSB_SPEECH ||
	    profile == CQ_PROFILE_SSB_NARROW ||
	    profile == CQ_PROFILE_SSB_WIDE;
}

static const char *
cq_profile_name(enum cq_profile profile)
{
	switch (profile) {
	case CQ_PROFILE_DEFAULT:
		return "default";
	case CQ_PROFILE_DEHUM_50:
		return "dehum-50";
	case CQ_PROFILE_DEHUM_60:
		return "dehum-60";
	case CQ_PROFILE_MB_BASS:
		return "multiband-bass";
	case CQ_PROFILE_AM_VOICE:
		return "am-voice";
	case CQ_PROFILE_AM_SHORTWAVE:
		return "am-shortwave";
	case CQ_PROFILE_AM_WIDE:
		return "am-wide";
	case CQ_PROFILE_SSB_SPEECH:
		return "ssb-speech";
	case CQ_PROFILE_SSB_NARROW:
		return "ssb-narrow";
	case CQ_PROFILE_SSB_WIDE:
		return "ssb-wide";
	default:
		return "unknown";
	}
}

static int
cq_run_matrix(void)
{
	struct cq_metrics metrics;
	struct cp_block_config config;
	enum cq_fixture fixture;
	enum cq_profile profile;
	const char *reason;

	for (profile = CQ_PROFILE_DEFAULT; profile <= CQ_PROFILE_SSB_WIDE;
	    profile++) {
		for (fixture = CQ_FIXTURE_SILENCE;
		    fixture <= CQ_FIXTURE_STEREO_IMBALANCE; fixture++) {
			if (!cq_process(profile, fixture, &metrics, &config))
				return 0;
			reason = NULL;
			if (!metrics.finite)
				reason = "non-finite";
			if (reason == NULL &&
			    metrics.output_peak > CP_DEFAULT_CEILING +
			    CQ_LIMIT_EPSILON)
				reason = "limiter";
			if (reason == NULL && cq_profile_is_am(profile) &&
			    metrics.output_min <
			    -config.am_config.negative_peak_limit -
			    CQ_LIMIT_EPSILON)
				reason = "am-negative-peak";
			if (reason == NULL && cq_profile_is_ssb(profile) &&
			    metrics.output_peak > config.ssb_config.peak_limit +
			    CQ_LIMIT_EPSILON)
				reason = "ssb-peak";
			if (reason != NULL) {
				printf("chain_quality profile=%s fixture=%s "
				    "status=fail reason=%s input_rms=%0.6f "
				    "output_rms=%0.6f output_peak=%0.6f "
				    "output_min=%0.6f\n",
				    cq_profile_name(profile),
				    cq_fixture_name(fixture), reason,
				    metrics.input_square,
				    metrics.output_square,
				    metrics.output_peak, metrics.output_min);
				return 0;
			}
		}
	}

	return 1;
}

static int
cq_test_agc_limits(void)
{
	struct cq_metrics metrics;
	struct cp_block_config config;

	if (!cq_process(CQ_PROFILE_DEFAULT, CQ_FIXTURE_STEPPED_SPEECH,
	    &metrics, &config))
		return 0;
	if (metrics.final_gain < config.min_gain ||
	    metrics.final_gain > config.max_gain) {
		printf("chain_quality check=agc-limits status=fail "
		    "gain=%0.6f min=%0.6f max=%0.6f\n",
		    metrics.final_gain, config.min_gain, config.max_gain);
		return 0;
	}

	return 1;
}

static int
cq_test_dc_offset_reduction(void)
{
	struct cq_metrics metrics;
	struct cp_block_config config;
	cp_sample_t input_mean;
	cp_sample_t output_mean;

	if (!cq_process(CQ_PROFILE_DEFAULT, CQ_FIXTURE_DC_OFFSET, &metrics,
	    &config))
		return 0;
	(void)config;

	input_mean = cq_abs((cp_sample_t)metrics.input_sum);
	output_mean = cq_abs((cp_sample_t)metrics.output_sum);
	if (output_mean >= input_mean * 0.40f) {
		printf("chain_quality check=dc-offset status=fail "
		    "input_mean=%0.6f output_mean=%0.6f\n",
		    input_mean, output_mean);
		return 0;
	}

	return 1;
}

static int
cq_test_dehummer_reduction(void)
{
	struct cq_metrics metrics;
	struct cp_block_config config;

	if (!cq_process(CQ_PROFILE_DEHUM_50, CQ_FIXTURE_HUM_50, &metrics,
	    &config))
		return 0;
	(void)config;
	if (metrics.output_square >= metrics.input_square * 0.75f) {
		printf("chain_quality check=dehum-50 status=fail "
		    "input_rms=%0.6f output_rms=%0.6f\n",
		    metrics.input_square, metrics.output_square);
		return 0;
	}
	if (!cq_process(CQ_PROFILE_DEHUM_60, CQ_FIXTURE_HUM_60, &metrics,
	    &config))
		return 0;
	if (metrics.output_square >= metrics.input_square * 0.75f) {
		printf("chain_quality check=dehum-60 status=fail "
		    "input_rms=%0.6f output_rms=%0.6f\n",
		    metrics.input_square, metrics.output_square);
		return 0;
	}

	return 1;
}

static int
cq_test_lowpass_profiles(void)
{
	struct cq_metrics metrics;
	struct cp_block_config config;

	if (!cq_process(CQ_PROFILE_AM_VOICE, CQ_FIXTURE_HIGH_TONE, &metrics,
	    &config))
		return 0;
	if (metrics.output_square >= metrics.input_square * 0.35f) {
		printf("chain_quality check=am-lowpass status=fail "
		    "input_rms=%0.6f output_rms=%0.6f\n",
		    metrics.input_square, metrics.output_square);
		return 0;
	}
	if (!cq_process(CQ_PROFILE_SSB_NARROW, CQ_FIXTURE_HIGH_TONE,
	    &metrics, &config))
		return 0;
	if (metrics.output_square >= metrics.input_square * 0.30f) {
		printf("chain_quality check=ssb-lowpass status=fail "
		    "input_rms=%0.6f output_rms=%0.6f\n",
		    metrics.input_square, metrics.output_square);
		return 0;
	}

	return 1;
}

static int
cq_test_silence_stability(void)
{
	struct cq_metrics metrics;
	struct cp_block_config config;

	if (!cq_process(CQ_PROFILE_DEFAULT, CQ_FIXTURE_SILENCE, &metrics,
	    &config))
		return 0;
	if (metrics.output_peak > CQ_SILENCE_EPSILON ||
	    metrics.final_gain > 1.01f ||
	    metrics.final_gate_state != CP_AGC_STATE_SILENT) {
		printf("chain_quality check=silence status=fail "
		    "output_peak=%0.6f gain=%0.6f state=%d\n",
		    metrics.output_peak, metrics.final_gain,
		    metrics.final_gate_state);
		return 0;
	}

	return 1;
}

static int
cq_test_stereo_stability(void)
{
	struct cq_metrics metrics;
	struct cp_block_config config;

	if (!cq_process(CQ_PROFILE_MB_BASS, CQ_FIXTURE_STEREO_IMBALANCE,
	    &metrics, &config))
		return 0;
	(void)config;
	if (metrics.output_left_square <= metrics.output_right_square * 2.0) {
		printf("chain_quality check=stereo status=fail "
		    "left_rms=%0.6f right_rms=%0.6f\n",
		    metrics.output_left_square, metrics.output_right_square);
		return 0;
	}

	return 1;
}

static void
cq_update(struct cq_metrics *metrics, const cp_sample_t *input,
	const cp_sample_t *output, size_t frames)
{
	cp_sample_t input_abs;
	cp_sample_t output_abs;
	size_t frame;
	size_t index;
	size_t sample;
	size_t samples;

	samples = frames * CP_CHANNELS_STEREO;
	for (sample = 0; sample < samples; sample++) {
		if (!isfinite(input[sample]) || !isfinite(output[sample])) {
			metrics->finite = 0;
			continue;
		}
		input_abs = cq_abs(input[sample]);
		output_abs = cq_abs(output[sample]);
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
		metrics->input_sum += (double)input[sample];
		metrics->output_sum += (double)output[sample];
	}

	for (frame = 0; frame < frames; frame++) {
		index = frame * CP_CHANNELS_STEREO;
		metrics->output_left_square +=
		    (double)output[index] * (double)output[index];
		metrics->output_right_square +=
		    (double)output[index + 1] * (double)output[index + 1];
	}
}

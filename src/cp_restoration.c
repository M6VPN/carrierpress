/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_restoration.c */

#include <sys/types.h>

#include <math.h>
#include <string.h>

#include "cp_restoration.h"

#define CP_RESTORATION_FLAT_EPSILON	0.0005f
#define CP_RESTORATION_ENERGY_FLOOR	1.0e-12
#define CP_RESTORATION_HF_SCALE		4.0
#define CP_RESTORATION_LOSS_HF_RATIO	0.08f
#define CP_RESTORATION_SSB_HF_RATIO	0.04f
#define CP_RESTORATION_AM_HF_RATIO	0.12f
#define CP_RESTORATION_WIDEBAND_HF_RATIO	0.20f
#define CP_RESTORATION_CLIP_RATIO_HIGH	0.01f
#define CP_RESTORATION_CLIP_RATIO_LOW	0.0002f
#define CP_RESTORATION_LOW_CEILING_FLOOR	0.20f
#define CP_RESTORATION_LOW_CEILING_HIGH	0.0008f
#define CP_RESTORATION_DYNAMIC_FLAT_EPSILON	0.0001f
#define CP_RESTORATION_TRANSIENT_RATIO_MAX	0.004f
#define CP_RESTORATION_CONFIDENCE_FLAG_LEVEL	0.50f
#define CP_RESTORATION_LOW_CREST_FACTOR	1.60f
#define CP_RESTORATION_LOW_CREST_RANGE	0.60f

static cp_sample_t	cp_restoration_clamp(cp_sample_t, cp_sample_t,
			    cp_sample_t);
static void		cp_restoration_finish_window(struct cp_restoration *);
static enum cp_restoration_source_profile cp_restoration_source_profile(
			    const struct cp_restoration_metrics *);
static int		cp_restoration_validate_config(
			    const struct cp_restoration_config *);

static cp_sample_t
cp_restoration_clamp(cp_sample_t value, cp_sample_t low, cp_sample_t high)
{
	if (!isfinite(value))
		return low;
	if (value < low)
		return low;
	if (value > high)
		return high;

	return value;
}

static void
cp_restoration_finish_window(struct cp_restoration *restoration)
{
	cp_sample_t clip_score;
	cp_sample_t crest_score;
	cp_sample_t flat_score;
	cp_sample_t low_ceiling_score;
	cp_sample_t repeat_score;
	double rms;
	cp_sample_t transient_score;
	double hf_ratio;

	if (restoration == NULL)
		return;

	if (restoration->metrics.total_sample_count == 0 ||
	    restoration->sample_energy <= CP_RESTORATION_ENERGY_FLOOR) {
		restoration->metrics.clipped_sample_ratio = 0.0f;
		restoration->metrics.high_frequency_ratio = 0.0f;
		restoration->metrics.clipping_confidence = 0.0f;
		restoration->metrics.lossy_confidence = 0.0f;
		restoration->metrics.low_ceiling_clipping_confidence = 0.0f;
		restoration->metrics.transient_confidence = 0.0f;
		restoration->metrics.flat_run_ratio = 0.0f;
		restoration->metrics.peak_repeat_ratio = 0.0f;
		restoration->metrics.low_ceiling_flat_run_ratio = 0.0f;
		restoration->metrics.low_ceiling_peak_repeat_ratio = 0.0f;
		restoration->metrics.observed_peak = 0.0f;
		restoration->metrics.crest_factor = 0.0f;
		restoration->metrics.source_profile =
		    CP_RESTORATION_SOURCE_SILENCE;
		restoration->metrics.reason_flags = restoration->metrics.finite ?
		    0u : CP_RESTORATION_REASON_NONFINITE;
		return;
	}

	restoration->metrics.clipped_sample_ratio =
	    (cp_sample_t)restoration->metrics.clipped_sample_count /
	    (cp_sample_t)restoration->metrics.total_sample_count;
	rms = sqrt(restoration->sample_energy /
	    (double)restoration->metrics.total_sample_count);
	if (rms > CP_RESTORATION_ENERGY_FLOOR)
		restoration->metrics.crest_factor =
		    restoration->metrics.observed_peak / (cp_sample_t)rms;
	else
		restoration->metrics.crest_factor = 0.0f;

	hf_ratio = sqrt(restoration->difference_energy /
	    (restoration->sample_energy * CP_RESTORATION_HF_SCALE));
	restoration->metrics.high_frequency_ratio =
	    cp_restoration_clamp((cp_sample_t)hf_ratio, 0.0f, 1.0f);

	clip_score = (restoration->metrics.clipped_sample_ratio -
	    CP_RESTORATION_CLIP_RATIO_LOW) /
	    (CP_RESTORATION_CLIP_RATIO_HIGH - CP_RESTORATION_CLIP_RATIO_LOW);
	restoration->metrics.flat_run_ratio =
	    (cp_sample_t)restoration->metrics.flat_run_count /
	    (cp_sample_t)restoration->config.analysis_window_frames;
	restoration->metrics.peak_repeat_ratio =
	    (cp_sample_t)restoration->metrics.peak_repeat_count /
	    (cp_sample_t)restoration->metrics.total_sample_count;
	restoration->metrics.low_ceiling_flat_run_ratio =
	    (cp_sample_t)restoration->metrics.low_ceiling_flat_run_count /
	    (cp_sample_t)restoration->config.analysis_window_frames;
	restoration->metrics.low_ceiling_peak_repeat_ratio =
	    (cp_sample_t)restoration->metrics.low_ceiling_peak_repeat_count /
	    (cp_sample_t)restoration->metrics.total_sample_count;

	flat_score = restoration->metrics.flat_run_ratio;
	repeat_score = restoration->metrics.peak_repeat_ratio;
	low_ceiling_score =
	    restoration->metrics.low_ceiling_flat_run_ratio * 160.0f;
	crest_score = 0.0f;
	if (restoration->metrics.observed_peak >=
	    CP_RESTORATION_LOW_CEILING_FLOOR &&
	    restoration->metrics.crest_factor > 0.0f &&
	    restoration->metrics.crest_factor < CP_RESTORATION_LOW_CREST_FACTOR)
		crest_score = (CP_RESTORATION_LOW_CREST_FACTOR -
		    restoration->metrics.crest_factor) /
		    CP_RESTORATION_LOW_CREST_RANGE;

	restoration->metrics.clipping_confidence =
	    cp_restoration_clamp((clip_score * 0.70f) +
	    (flat_score * 0.20f) + (repeat_score * 0.10f), 0.0f, 1.0f);
	restoration->metrics.low_ceiling_clipping_confidence =
	    cp_restoration_clamp((low_ceiling_score * 0.80f) +
	    (restoration->metrics.low_ceiling_peak_repeat_ratio * 0.20f) +
	    crest_score, 0.0f, 1.0f);
	if (restoration->metrics.clipping_confidence >=
	    CP_RESTORATION_CONFIDENCE_FLAG_LEVEL)
		restoration->metrics.low_ceiling_clipping_confidence = 0.0f;
	transient_score = 0.0f;
	if (restoration->metrics.clipped_sample_ratio > 0.0f &&
	    restoration->metrics.clipped_sample_ratio <
	    CP_RESTORATION_TRANSIENT_RATIO_MAX &&
	    restoration->metrics.low_ceiling_flat_run_ratio <
	    CP_RESTORATION_LOW_CEILING_HIGH)
		transient_score = 1.0f -
		    (restoration->metrics.clipped_sample_ratio /
		    CP_RESTORATION_TRANSIENT_RATIO_MAX);
	restoration->metrics.transient_confidence =
	    cp_restoration_clamp(transient_score, 0.0f, 1.0f);
	restoration->metrics.lossy_confidence = cp_restoration_clamp(
	    (CP_RESTORATION_LOSS_HF_RATIO -
	    restoration->metrics.high_frequency_ratio) /
	    CP_RESTORATION_LOSS_HF_RATIO, 0.0f, 1.0f);
	restoration->metrics.source_profile =
	    cp_restoration_source_profile(&restoration->metrics);

	restoration->metrics.reason_flags = 0u;
	if (restoration->metrics.clipping_confidence >=
	    CP_RESTORATION_CONFIDENCE_FLAG_LEVEL)
		restoration->metrics.reason_flags |=
		    CP_RESTORATION_REASON_HARD_CLIP;
	if (restoration->metrics.low_ceiling_clipping_confidence >=
	    CP_RESTORATION_CONFIDENCE_FLAG_LEVEL)
		restoration->metrics.reason_flags |=
		    CP_RESTORATION_REASON_LOW_CEILING;
	if (restoration->metrics.transient_confidence >=
	    CP_RESTORATION_CONFIDENCE_FLAG_LEVEL)
		restoration->metrics.reason_flags |=
		    CP_RESTORATION_REASON_TRANSIENT;
	if (restoration->metrics.lossy_confidence >=
	    CP_RESTORATION_CONFIDENCE_FLAG_LEVEL)
		restoration->metrics.reason_flags |=
		    CP_RESTORATION_REASON_LOW_HF;
	if (!restoration->metrics.finite)
		restoration->metrics.reason_flags |=
		    CP_RESTORATION_REASON_NONFINITE;
}

static enum cp_restoration_source_profile
cp_restoration_source_profile(const struct cp_restoration_metrics *metrics)
{
	if (metrics == NULL)
		return CP_RESTORATION_SOURCE_UNKNOWN;
	if (metrics->total_sample_count == 0)
		return CP_RESTORATION_SOURCE_SILENCE;
	if (metrics->high_frequency_ratio >= CP_RESTORATION_WIDEBAND_HF_RATIO)
		return CP_RESTORATION_SOURCE_WIDEBAND;
	if (metrics->high_frequency_ratio >= CP_RESTORATION_AM_HF_RATIO)
		return CP_RESTORATION_SOURCE_AM_LIMITED;
	if (metrics->high_frequency_ratio >= CP_RESTORATION_SSB_HF_RATIO)
		return CP_RESTORATION_SOURCE_SSB_VOICE;

	return CP_RESTORATION_SOURCE_HIGH_FREQUENCY_LOSS;
}

static int
cp_restoration_validate_config(const struct cp_restoration_config *config)
{
	if (config == NULL)
		return CP_ERR_NULL;
	if (config->channel_count != CP_CHANNELS_MONO &&
	    config->channel_count != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (!isfinite(config->sample_rate) ||
	    config->sample_rate < CP_RESTORATION_MIN_SAMPLE_RATE ||
	    config->sample_rate > CP_RESTORATION_MAX_SAMPLE_RATE)
		return CP_ERR_RANGE;
	if (!isfinite(config->clip_threshold) ||
	    config->clip_threshold < CP_RESTORATION_MIN_CLIP_THRESHOLD ||
	    config->clip_threshold > CP_RESTORATION_MAX_CLIP_THRESHOLD)
		return CP_ERR_RANGE;
	if (config->flat_run_min < CP_RESTORATION_MIN_FLAT_RUN_MIN ||
	    config->flat_run_min > CP_RESTORATION_MAX_FLAT_RUN_MIN)
		return CP_ERR_RANGE;
	if (config->analysis_window_frames <
	    CP_RESTORATION_MIN_WINDOW_FRAMES ||
	    config->analysis_window_frames >
	    CP_RESTORATION_MAX_WINDOW_FRAMES)
		return CP_ERR_RANGE;

	return CP_OK;
}

void
cp_restoration_default_config(struct cp_restoration_config *config)
{
	if (config == NULL)
		return;

	config->enabled = CP_RESTORATION_DEFAULT_ENABLED;
	config->sample_rate = CP_RESTORATION_DEFAULT_SAMPLE_RATE;
	config->channel_count = CP_RESTORATION_DEFAULT_CHANNELS;
	config->clip_threshold = CP_RESTORATION_DEFAULT_CLIP_THRESHOLD;
	config->flat_run_min = CP_RESTORATION_DEFAULT_FLAT_RUN_MIN;
	config->analysis_window_frames =
	    CP_RESTORATION_DEFAULT_WINDOW_FRAMES;
}

int
cp_restoration_init(struct cp_restoration *restoration,
	const struct cp_restoration_config *config)
{
	int status;

	if (restoration == NULL || config == NULL)
		return CP_ERR_NULL;

	status = cp_restoration_validate_config(config);
	if (status != CP_OK)
		return status;

	(void)memset(restoration, 0, sizeof(*restoration));
	restoration->config = *config;
	restoration->metrics.finite = 1;

	return CP_OK;
}

const struct cp_restoration_metrics *
cp_restoration_get_metrics(const struct cp_restoration *restoration)
{
	if (restoration == NULL)
		return NULL;

	return &restoration->metrics;
}

const char *
cp_restoration_source_profile_string(
	enum cp_restoration_source_profile profile)
{
	switch (profile) {
	case CP_RESTORATION_SOURCE_SILENCE:
		return "silence";
	case CP_RESTORATION_SOURCE_WIDEBAND:
		return "wideband";
	case CP_RESTORATION_SOURCE_AM_LIMITED:
		return "am-limited";
	case CP_RESTORATION_SOURCE_SSB_VOICE:
		return "ssb-voice";
	case CP_RESTORATION_SOURCE_HIGH_FREQUENCY_LOSS:
		return "high-frequency-loss";
	case CP_RESTORATION_SOURCE_UNKNOWN:
	default:
		return "unknown";
	}
}

int
cp_restoration_process(struct cp_restoration *restoration,
	const cp_sample_t *input, size_t frames)
{
	cp_sample_t abs_sample;
	cp_sample_t diff;
	cp_sample_t sample;
	size_t channel;
	size_t frame;
	size_t index;

	if (restoration == NULL || input == NULL)
		return CP_ERR_NULL;
	if (restoration->config.channel_count != CP_CHANNELS_MONO &&
	    restoration->config.channel_count != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (frames == 0)
		return CP_ERR_RANGE;
	if (!restoration->config.enabled)
		return CP_OK;

	for (frame = 0; frame < frames; frame++) {
		if (restoration->window_frames_seen >=
		    restoration->config.analysis_window_frames) {
			cp_restoration_finish_window(restoration);
			restoration->metrics.clipped_sample_count = 0;
			restoration->metrics.total_sample_count = 0;
			restoration->metrics.flat_run_count = 0;
			restoration->metrics.peak_repeat_count = 0;
			restoration->metrics.low_ceiling_flat_run_count = 0;
			restoration->metrics.low_ceiling_peak_repeat_count = 0;
			restoration->metrics.observed_peak = 0.0f;
			restoration->metrics.crest_factor = 0.0f;
			restoration->sample_energy = 0.0;
			restoration->difference_energy = 0.0;
			restoration->window_frames_seen = 0;
		}

		for (channel = 0; channel < restoration->config.channel_count;
		    channel++) {
			index = (frame * restoration->config.channel_count) +
			    channel;
			sample = input[index];
			if (!isfinite(sample)) {
				sample = 0.0f;
				restoration->metrics.finite = 0;
			}

			abs_sample = fabsf(sample);
			if (abs_sample > restoration->metrics.observed_peak)
				restoration->metrics.observed_peak =
				    abs_sample;
			if (abs_sample >= restoration->config.clip_threshold) {
				restoration->metrics.clipped_sample_count++;
				if (restoration->have_previous[channel] &&
				    fabsf(abs_sample -
				    restoration->previous_abs[channel]) <=
				    CP_RESTORATION_FLAT_EPSILON) {
					restoration->flat_run_length[channel]++;
					if (restoration->flat_run_length[channel] ==
					    restoration->config.flat_run_min)
						restoration->metrics.flat_run_count++;
					restoration->metrics.peak_repeat_count++;
				} else {
					restoration->flat_run_length[channel] = 1;
				}
			} else {
				restoration->flat_run_length[channel] = 0;
			}

			if (abs_sample >= CP_RESTORATION_LOW_CEILING_FLOOR) {
				if (restoration->have_previous[channel] &&
				    fabsf(abs_sample -
				    restoration->previous_abs[channel]) <=
				    CP_RESTORATION_DYNAMIC_FLAT_EPSILON) {
					restoration->low_ceiling_flat_run_length[channel]++;
					if (restoration->
					    low_ceiling_flat_run_length[channel] ==
					    restoration->config.flat_run_min)
						restoration->metrics.
						    low_ceiling_flat_run_count++;
					restoration->metrics.
					    low_ceiling_peak_repeat_count++;
				} else {
					restoration->low_ceiling_flat_run_length[channel] = 1;
				}
			} else {
				restoration->low_ceiling_flat_run_length[channel] = 0;
			}

			if (restoration->have_previous[channel]) {
				diff = sample -
				    restoration->previous_sample[channel];
				restoration->difference_energy +=
				    (double)diff * (double)diff;
			}

			restoration->sample_energy +=
			    (double)sample * (double)sample;
			restoration->previous_sample[channel] = sample;
			restoration->previous_abs[channel] = abs_sample;
			restoration->have_previous[channel] = 1;
			restoration->metrics.total_sample_count++;
		}

		restoration->window_frames_seen++;
	}

	cp_restoration_finish_window(restoration);

	return CP_OK;
}

int
cp_restoration_reset(struct cp_restoration *restoration)
{
	struct cp_restoration_config config;

	if (restoration == NULL)
		return CP_ERR_NULL;

	config = restoration->config;
	(void)memset(restoration, 0, sizeof(*restoration));
	restoration->config = config;
	restoration->metrics.finite = 1;

	return CP_OK;
}

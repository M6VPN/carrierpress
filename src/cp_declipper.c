/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_declipper.c */

#include <sys/types.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "cp_declipper.h"

#define CP_DECLIPPER_HARD_CONFIDENCE		0.50f
#define CP_DECLIPPER_LOW_CEILING_CONFIDENCE	0.50f
#define CP_DECLIPPER_TRANSIENT_CONFIDENCE	0.50f
#define CP_DECLIPPER_FLAT_EPSILON		0.0015f

static cp_sample_t	cp_declipper_abs(cp_sample_t);
static cp_sample_t	cp_declipper_clamp(cp_sample_t, cp_sample_t,
			    cp_sample_t);
static int		cp_declipper_confident(
			    const struct cp_declipper *,
			    const struct cp_restoration_metrics *);
static cp_sample_t	cp_declipper_target(const struct cp_declipper *,
			    const struct cp_declipper_channel *,
			    cp_sample_t, cp_sample_t);
static int		cp_declipper_validate_config(
			    const struct cp_declipper_config *);

static cp_sample_t
cp_declipper_abs(cp_sample_t sample)
{
	return sample < 0.0f ? -sample : sample;
}

static cp_sample_t
cp_declipper_clamp(cp_sample_t value, cp_sample_t low, cp_sample_t high)
{
	if (!isfinite(value))
		return 0.0f;
	if (value < low)
		return low;
	if (value > high)
		return high;

	return value;
}

static int
cp_declipper_confident(const struct cp_declipper *declipper,
	const struct cp_restoration_metrics *metrics)
{
	if (declipper == NULL || metrics == NULL)
		return 0;
	if (!metrics->finite)
		return 0;
	if (metrics->transient_confidence >=
	    CP_DECLIPPER_TRANSIENT_CONFIDENCE)
		return 0;
	if (metrics->clipping_confidence >= CP_DECLIPPER_HARD_CONFIDENCE)
		return 1;
	if (metrics->low_ceiling_clipping_confidence >=
	    CP_DECLIPPER_LOW_CEILING_CONFIDENCE)
		return 1;

	(void)declipper;
	return 0;
}

static cp_sample_t
cp_declipper_target(const struct cp_declipper *declipper,
	const struct cp_declipper_channel *channel, cp_sample_t sample,
	cp_sample_t previous)
{
	cp_sample_t bridged;
	cp_sample_t delta;
	cp_sample_t sign;

	if (declipper == NULL || channel == NULL)
		return sample;

	sign = sample < 0.0f ? -1.0f : 1.0f;
	bridged = previous;
	if (!channel->previous_valid ||
	    cp_declipper_abs(bridged) >= cp_declipper_abs(sample))
		bridged = sign * (cp_declipper_abs(sample) +
		    (CP_DEFAULT_CEILING - cp_declipper_abs(sample)) * 0.25f);

	delta = (bridged - sample) * declipper->config.repair_strength;
	return cp_declipper_clamp(sample + delta, -CP_DEFAULT_CEILING,
	    CP_DEFAULT_CEILING);
}

static int
cp_declipper_validate_config(const struct cp_declipper_config *config)
{
	if (config == NULL)
		return CP_ERR_NULL;
	if (config->channel_count != CP_CHANNELS_MONO &&
	    config->channel_count != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (!isfinite(config->sample_rate) ||
	    config->sample_rate < CP_DECLIPPER_MIN_SAMPLE_RATE ||
	    config->sample_rate > CP_DECLIPPER_MAX_SAMPLE_RATE)
		return CP_ERR_RANGE;
	if (config->mode != CP_DECLIPPER_MODE_CONSERVATIVE)
		return CP_ERR_RANGE;
	if (!isfinite(config->clip_threshold) ||
	    config->clip_threshold < CP_DECLIPPER_MIN_CLIP_THRESHOLD ||
	    config->clip_threshold > CP_DECLIPPER_MAX_CLIP_THRESHOLD)
		return CP_ERR_RANGE;
	if (!isfinite(config->low_ceiling_threshold) ||
	    config->low_ceiling_threshold < CP_DECLIPPER_MIN_LOW_CEILING ||
	    config->low_ceiling_threshold > CP_DECLIPPER_MAX_LOW_CEILING)
		return CP_ERR_RANGE;
	if (config->low_ceiling_threshold >= config->clip_threshold)
		return CP_ERR_RANGE;
	if (!isfinite(config->repair_strength) ||
	    config->repair_strength < CP_DECLIPPER_MIN_STRENGTH ||
	    config->repair_strength > CP_DECLIPPER_MAX_STRENGTH)
		return CP_ERR_RANGE;
	if (config->max_repair_samples < CP_DECLIPPER_MIN_REPAIR_SAMPLES ||
	    config->max_repair_samples > CP_DECLIPPER_MAX_REPAIR_SAMPLES)
		return CP_ERR_RANGE;

	return CP_OK;
}

void
cp_declipper_default_config(struct cp_declipper_config *config)
{
	if (config == NULL)
		return;

	config->enabled = CP_DECLIPPER_DEFAULT_ENABLED;
	config->sample_rate = CP_DECLIPPER_DEFAULT_SAMPLE_RATE;
	config->channel_count = CP_DECLIPPER_DEFAULT_CHANNELS;
	config->mode = CP_DECLIPPER_MODE_CONSERVATIVE;
	config->clip_threshold = CP_DECLIPPER_DEFAULT_CLIP_THRESHOLD;
	config->low_ceiling_threshold = CP_DECLIPPER_DEFAULT_LOW_CEILING;
	config->repair_strength = CP_DECLIPPER_DEFAULT_STRENGTH;
	config->max_repair_samples = CP_DECLIPPER_DEFAULT_MAX_REPAIR;
	config->analysis_required = CP_DECLIPPER_DEFAULT_ANALYSIS_REQUIRED;
}

const char *
cp_declipper_bypass_reason_string(enum cp_declipper_bypass_reason reason)
{
	switch (reason) {
	case CP_DECLIPPER_BYPASS_NONE:
		return "none";
	case CP_DECLIPPER_BYPASS_DISABLED:
		return "disabled";
	case CP_DECLIPPER_BYPASS_ANALYSIS_REQUIRED:
		return "analysis-required";
	case CP_DECLIPPER_BYPASS_LOW_CONFIDENCE:
		return "low-confidence";
	case CP_DECLIPPER_BYPASS_TRANSIENT:
		return "transient";
	case CP_DECLIPPER_BYPASS_LONG_RUN:
		return "long-run";
	case CP_DECLIPPER_BYPASS_NONFINITE:
		return "nonfinite";
	default:
		return "unknown";
	}
}

int
cp_declipper_init(struct cp_declipper *declipper,
	const struct cp_declipper_config *config)
{
	int status;

	if (declipper == NULL || config == NULL)
		return CP_ERR_NULL;

	status = cp_declipper_validate_config(config);
	if (status != CP_OK)
		return status;

	(void)memset(declipper, 0, sizeof(*declipper));
	declipper->config = *config;
	declipper->metrics.finite = 1;
	declipper->metrics.bypass_reason = config->enabled ?
	    CP_DECLIPPER_BYPASS_LOW_CONFIDENCE : CP_DECLIPPER_BYPASS_DISABLED;

	return CP_OK;
}

int
cp_declipper_process(struct cp_declipper *declipper,
	const struct cp_restoration_metrics *metrics, const cp_sample_t *input,
	cp_sample_t *output, size_t frames)
{
	struct cp_declipper_channel *channel_state;
	cp_sample_t abs_sample;
	cp_sample_t delta;
	cp_sample_t previous;
	cp_sample_t repaired;
	cp_sample_t sample;
	size_t channel;
	size_t frame;
	size_t index;

	if (declipper == NULL || input == NULL || output == NULL)
		return CP_ERR_NULL;
	if (declipper->config.channel_count != CP_CHANNELS_MONO &&
	    declipper->config.channel_count != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (frames == 0)
		return CP_ERR_RANGE;
	if (frames > SIZE_MAX / declipper->config.channel_count)
		return CP_ERR_RANGE;

	declipper->metrics.repaired_sample_count = 0;
	declipper->metrics.repaired_run_count = 0;
	declipper->metrics.max_repair_delta = 0.0f;
	declipper->metrics.finite = 1;
	declipper->metrics.bypass_reason = CP_DECLIPPER_BYPASS_NONE;

	if (!declipper->config.enabled) {
		if (input != output)
			(void)memcpy(output, input, frames *
			    declipper->config.channel_count * sizeof(*output));
		declipper->metrics.bypass_reason =
		    CP_DECLIPPER_BYPASS_DISABLED;
		return CP_OK;
	}
	if (declipper->config.analysis_required && metrics == NULL) {
		if (input != output)
			(void)memcpy(output, input, frames *
			    declipper->config.channel_count * sizeof(*output));
		declipper->metrics.bypass_reason =
		    CP_DECLIPPER_BYPASS_ANALYSIS_REQUIRED;
		return CP_OK;
	}
	if (metrics != NULL && !metrics->finite) {
		if (input != output)
			(void)memcpy(output, input, frames *
			    declipper->config.channel_count * sizeof(*output));
		declipper->metrics.bypass_reason =
		    CP_DECLIPPER_BYPASS_NONFINITE;
		return CP_OK;
	}
	if (!cp_declipper_confident(declipper, metrics)) {
		if (input != output)
			(void)memcpy(output, input, frames *
			    declipper->config.channel_count * sizeof(*output));
		if (metrics != NULL && metrics->transient_confidence >=
		    CP_DECLIPPER_TRANSIENT_CONFIDENCE)
			declipper->metrics.bypass_reason =
			    CP_DECLIPPER_BYPASS_TRANSIENT;
		else
			declipper->metrics.bypass_reason =
			    CP_DECLIPPER_BYPASS_LOW_CONFIDENCE;
		return CP_OK;
	}

	for (frame = 0; frame < frames; frame++) {
		for (channel = 0; channel < declipper->config.channel_count;
		    channel++) {
			index = (frame * declipper->config.channel_count) +
			    channel;
			channel_state = &declipper->channel[channel];
			sample = input[index];
			if (!isfinite(sample)) {
				sample = 0.0f;
				declipper->metrics.finite = 0;
				declipper->metrics.bypass_reason =
				    CP_DECLIPPER_BYPASS_NONFINITE;
			}

			previous = channel_state->previous_sample;
			abs_sample = cp_declipper_abs(sample);
			repaired = sample;
			if (abs_sample >= declipper->config.clip_threshold ||
			    abs_sample >=
			    declipper->config.low_ceiling_threshold) {
				if (!channel_state->active) {
					channel_state->active = 1;
					channel_state->run_length = 0;
					channel_state->held_sample = sample;
					declipper->metrics.repaired_run_count++;
				}
				channel_state->run_length++;
				if (channel_state->run_length <=
				    declipper->config.max_repair_samples) {
					repaired = cp_declipper_target(
					    declipper, channel_state, sample,
					    previous);
					delta = cp_declipper_abs(repaired -
					    sample);
					if (delta >
					    declipper->metrics.max_repair_delta)
						declipper->metrics.
						    max_repair_delta = delta;
					if (delta > 0.0f)
						declipper->metrics.
						    repaired_sample_count++;
				} else {
					declipper->metrics.bypass_reason =
					    CP_DECLIPPER_BYPASS_LONG_RUN;
				}
			} else {
				channel_state->active = 0;
				channel_state->run_length = 0;
			}

			output[index] = repaired;
			channel_state->previous_sample = repaired;
			channel_state->previous_valid = 1;
		}
	}

	return CP_OK;
}

int
cp_declipper_reset(struct cp_declipper *declipper)
{
	struct cp_declipper_config config;

	if (declipper == NULL)
		return CP_ERR_NULL;

	config = declipper->config;
	(void)memset(declipper, 0, sizeof(*declipper));
	declipper->config = config;
	declipper->metrics.finite = 1;
	declipper->metrics.bypass_reason = config.enabled ?
	    CP_DECLIPPER_BYPASS_LOW_CONFIDENCE : CP_DECLIPPER_BYPASS_DISABLED;

	return CP_OK;
}

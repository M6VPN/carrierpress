/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_resampler.c */

#include <sys/types.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "cp_resampler.h"

#define CP_RESAMPLER_EPSILON	(0.000001)
#define CP_RESAMPLER_GUARD	4

static int		cp_resampler_config_valid(
			    const struct cp_resampler_config *);
static cp_sample_t	cp_resampler_frame_sample(
			    const struct cp_resampler *, const cp_sample_t *,
			    size_t, size_t, size_t);
static int		cp_resampler_rates_equal(double, double);

void
cp_resampler_default_config(struct cp_resampler_config *config)
{
	if (config == NULL)
		return;

	config->input_rate    = CP_RESAMPLER_DEFAULT_RATE;
	config->output_rate   = CP_RESAMPLER_DEFAULT_RATE;
	config->channel_count = CP_CHANNELS_STEREO;
	config->enabled       = 0;
}

int
cp_resampler_init(struct cp_resampler *state,
	const struct cp_resampler_config *config)
{
	if (state == NULL || config == NULL)
		return CP_ERR_NULL;
	if (!cp_resampler_config_valid(config))
		return CP_ERR_RANGE;

	memset(state, 0, sizeof(*state));
	state->config = *config;
	state->channels = config->channel_count;
	state->enabled = config->enabled &&
	    !cp_resampler_rates_equal(config->input_rate, config->output_rate);
	state->step = config->input_rate / config->output_rate;
	state->position = 1.0;

	return cp_resampler_reset(state);
}

size_t
cp_resampler_output_capacity(size_t input_frames, double input_rate,
	double output_rate)
{
	double frames;
	double ratio;

	if (input_frames == 0)
		return 0;
	if (!isfinite(input_rate) || !isfinite(output_rate) ||
	    input_rate < CP_RESAMPLER_MIN_RATE ||
	    input_rate > CP_RESAMPLER_MAX_RATE ||
	    output_rate < CP_RESAMPLER_MIN_RATE ||
	    output_rate > CP_RESAMPLER_MAX_RATE)
		return 0;

	ratio = output_rate / input_rate;
	frames = ceil((double)input_frames * ratio) +
	    (double)CP_RESAMPLER_GUARD;
	if (!isfinite(frames) || frames <= 0.0 || frames > (double)SIZE_MAX)
		return 0;

	return (size_t)frames;
}

int
cp_resampler_process(struct cp_resampler *state, const cp_sample_t *input,
	size_t input_frames, cp_sample_t *output, size_t output_capacity,
	size_t *output_frames)
{
	cp_sample_t current;
	cp_sample_t next;
	cp_sample_t sample;
	double fraction;
	size_t channel;
	size_t index;
	size_t input_index;
	size_t out;
	size_t samples;

	if (state == NULL || input == NULL || output == NULL ||
	    output_frames == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (input_frames == 0)
		return CP_ERR_RANGE;
	if (input_frames > (SIZE_MAX / state->channels))
		return CP_ERR_RANGE;

	*output_frames = 0;
	samples = input_frames * state->channels;
	for (index = 0; index < samples; index++) {
		if (!isfinite(input[index]))
			return CP_ERR_RANGE;
	}

	if (!state->enabled) {
		if (output_capacity < input_frames)
			return CP_ERR_BUFFER;
		memmove(output, input, samples * sizeof(*output));
		*output_frames = input_frames;
		for (channel = 0; channel < state->channels; channel++)
			state->previous[channel] =
			    input[((input_frames - 1) * state->channels) +
			    channel];
		state->has_previous = 1;
		return CP_OK;
	}

	if (!state->has_previous) {
		for (channel = 0; channel < state->channels; channel++)
			state->previous[channel] = input[channel];
		state->position = 1.0;
		state->has_previous = 1;
	}

	out = 0;
	while (state->position <= (double)input_frames) {
		if (out >= output_capacity)
			return CP_ERR_BUFFER;
		input_index = (size_t)floor(state->position);
		fraction = state->position - (double)input_index;
		for (channel = 0; channel < state->channels; channel++) {
			current = cp_resampler_frame_sample(state, input,
			    input_frames, input_index, channel);
			next = cp_resampler_frame_sample(state, input,
			    input_frames, input_index + 1, channel);
			sample = current +
			    (cp_sample_t)fraction * (next - current);
			if (!isfinite(sample))
				return CP_ERR_RANGE;
			output[(out * state->channels) + channel] = sample;
		}
		out++;
		state->position += state->step;
	}

	state->position -= (double)input_frames;
	for (channel = 0; channel < state->channels; channel++)
		state->previous[channel] =
		    input[((input_frames - 1) * state->channels) + channel];
	*output_frames = out;

	return CP_OK;
}

int
cp_resampler_reset(struct cp_resampler *state)
{
	if (state == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	memset(state->previous, 0, sizeof(state->previous));
	state->position = 1.0;
	state->has_previous = 0;

	return CP_OK;
}

static int
cp_resampler_config_valid(const struct cp_resampler_config *config)
{
	if (config == NULL)
		return 0;
	if (config->channel_count != CP_CHANNELS_MONO &&
	    config->channel_count != CP_CHANNELS_STEREO)
		return 0;
	if (!isfinite(config->input_rate) ||
	    !isfinite(config->output_rate))
		return 0;
	if (config->input_rate < CP_RESAMPLER_MIN_RATE ||
	    config->input_rate > CP_RESAMPLER_MAX_RATE)
		return 0;
	if (config->output_rate < CP_RESAMPLER_MIN_RATE ||
	    config->output_rate > CP_RESAMPLER_MAX_RATE)
		return 0;

	return 1;
}

static cp_sample_t
cp_resampler_frame_sample(const struct cp_resampler *state,
	const cp_sample_t *input, size_t input_frames, size_t frame,
	size_t channel)
{
	if (frame == 0)
		return state->previous[channel];
	if (frame > input_frames)
		return input[((input_frames - 1) * state->channels) + channel];

	return input[((frame - 1) * state->channels) + channel];
}

static int
cp_resampler_rates_equal(double left, double right)
{
	double difference;

	difference = left - right;
	if (difference < 0.0)
		difference = -difference;

	return difference <= CP_RESAMPLER_EPSILON;
}

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_dc_blocker.c */

#include <sys/types.h>

#include <stdint.h>

#include "cp_dc_blocker.h"

int
cp_dc_blocker_init(struct cp_dc_blocker *state, size_t channels,
	cp_sample_t coefficient)
{
	if (state == NULL)
		return CP_ERR_NULL;
	if (channels != CP_CHANNELS_MONO && channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (coefficient <= 0.0f || coefficient >= 1.0f)
		return CP_ERR_RANGE;

	state->channels    = channels;
	state->coefficient = coefficient;

	return cp_dc_blocker_reset(state);
}

int
cp_dc_blocker_process(struct cp_dc_blocker *state, const cp_sample_t *input,
	cp_sample_t *output, size_t frames)
{
	cp_sample_t x;
	cp_sample_t y;
	size_t channel;
	size_t frame;
	size_t index;

	if (state == NULL || input == NULL || output == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (frames > (SIZE_MAX / state->channels))
		return CP_ERR_RANGE;

	for (frame = 0; frame < frames; frame++) {
		for (channel = 0; channel < state->channels; channel++) {
			index = (frame * state->channels) + channel;
			x     = input[index];
			y     = x - state->channel[channel].x_prev +
			    (state->coefficient * state->channel[channel].y_prev);

			state->channel[channel].x_prev = x;
			state->channel[channel].y_prev = y;
			output[index]                  = y;
		}
	}

	return CP_OK;
}

int
cp_dc_blocker_reset(struct cp_dc_blocker *state)
{
	size_t channel;

	if (state == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	for (channel = 0; channel < CP_MAX_CHANNELS; channel++) {
		state->channel[channel].x_prev = 0.0f;
		state->channel[channel].y_prev = 0.0f;
	}

	return CP_OK;
}

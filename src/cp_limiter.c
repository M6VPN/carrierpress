/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_limiter.c */

#include <sys/types.h>

#include <stdint.h>

#include "cp_limiter.h"

int
cp_limiter_init(struct cp_limiter *state, size_t channels,
	cp_sample_t ceiling)
{
	if (state == NULL)
		return CP_ERR_NULL;
	if (channels != CP_CHANNELS_MONO && channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (ceiling <= 0.0f || ceiling > CP_SAMPLE_MAX)
		return CP_ERR_RANGE;

	state->channels = channels;
	state->ceiling  = ceiling;

	return cp_limiter_reset(state);
}

int
cp_limiter_process(struct cp_limiter *state, const cp_sample_t *input,
	cp_sample_t *output, size_t frames)
{
	cp_sample_t sample;
	size_t index;
	size_t samples;

	if (state == NULL || input == NULL || output == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (frames > (SIZE_MAX / state->channels))
		return CP_ERR_RANGE;

	samples = frames * state->channels;

	for (index = 0; index < samples; index++) {
		sample = input[index];

		if (sample > state->ceiling)
			sample = state->ceiling;
		else if (sample < -state->ceiling)
			sample = -state->ceiling;

		output[index] = sample;
	}

	return CP_OK;
}

int
cp_limiter_reset(struct cp_limiter *state)
{
	if (state == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	return CP_OK;
}

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_meter.c */

#include <sys/types.h>

#include <math.h>
#include <stdint.h>

#include "cp_meter.h"

int
cp_meter_init(struct cp_meter *meter, size_t channels)
{
	if (meter == NULL)
		return CP_ERR_NULL;
	if (channels != CP_CHANNELS_MONO && channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	meter->channels = channels;

	return cp_meter_reset(meter);
}

int
cp_meter_process(struct cp_meter *meter, const cp_sample_t *input,
	size_t frames)
{
	cp_sample_t abs_sample;
	cp_sample_t sample;
	cp_sample_t sum_sq[CP_MAX_CHANNELS];
	size_t channel;
	size_t frame;
	size_t index;

	if (meter == NULL || input == NULL)
		return CP_ERR_NULL;
	if (meter->channels != CP_CHANNELS_MONO &&
	    meter->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (frames == 0)
		return CP_ERR_RANGE;
	if (frames > (SIZE_MAX / meter->channels))
		return CP_ERR_RANGE;

	for (channel = 0; channel < CP_MAX_CHANNELS; channel++) {
		meter->peak[channel] = 0.0f;
		meter->rms[channel]  = 0.0f;
		sum_sq[channel]      = 0.0f;
	}

	for (frame = 0; frame < frames; frame++) {
		for (channel = 0; channel < meter->channels; channel++) {
			index      = (frame * meter->channels) + channel;
			sample     = input[index];
			abs_sample = fabsf(sample);

			if (abs_sample > meter->peak[channel])
				meter->peak[channel] = abs_sample;

			sum_sq[channel] += sample * sample;
		}
	}

	for (channel = 0; channel < meter->channels; channel++)
		meter->rms[channel] = sqrtf(sum_sq[channel] / (cp_sample_t)frames);

	return CP_OK;
}

int
cp_meter_reset(struct cp_meter *meter)
{
	size_t channel;

	if (meter == NULL)
		return CP_ERR_NULL;
	if (meter->channels != CP_CHANNELS_MONO &&
	    meter->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	for (channel = 0; channel < CP_MAX_CHANNELS; channel++) {
		meter->peak[channel] = 0.0f;
		meter->rms[channel]  = 0.0f;
	}

	return CP_OK;
}

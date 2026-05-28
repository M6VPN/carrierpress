/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_dehummer.c */

#include <sys/types.h>

#include <math.h>
#include <stdint.h>

#include "cp_dehummer.h"

static int	cp_dehummer_design(struct cp_dehummer *,
		    const struct cp_dehummer_config *);
static cp_sample_t	cp_dehummer_safe_sample(cp_sample_t);
static int	cp_dehummer_valid_config(const struct cp_dehummer_config *);

void
cp_dehummer_default_config(struct cp_dehummer_config *config)
{
	if (config == NULL)
		return;

	config->sample_rate    = CP_DEHUMMER_DEFAULT_SAMPLE_RATE;
	config->base_frequency = CP_DEHUMMER_DEFAULT_BASE_HZ;
	config->harmonic_count = CP_DEHUMMER_DEFAULT_HARMONICS;
	config->q_factor       = CP_DEHUMMER_DEFAULT_Q;
	config->enabled        = CP_DEHUMMER_DEFAULT_ENABLED;
	config->channel_count  = CP_CHANNELS_MONO;
}

int
cp_dehummer_init(struct cp_dehummer *state,
	const struct cp_dehummer_config *config)
{
	if (state == NULL || config == NULL)
		return CP_ERR_NULL;
	if (!cp_dehummer_valid_config(config))
		return CP_ERR_RANGE;

	state->config         = *config;
	state->channels       = config->channel_count;
	state->harmonic_count = config->harmonic_count;
	state->enabled        = config->enabled ? 1 : 0;

	if (cp_dehummer_design(state, config) != CP_OK)
		return CP_ERR_RANGE;

	return cp_dehummer_reset(state);
}

int
cp_dehummer_process(struct cp_dehummer *state, const cp_sample_t *input,
	cp_sample_t *output, size_t frames)
{
	cp_sample_t sample;
	size_t channel;
	size_t frame;
	size_t harmonic;
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
			index  = (frame * state->channels) + channel;
			sample = cp_dehummer_safe_sample(input[index]);

			if (state->enabled) {
				for (harmonic = 0; harmonic < state->harmonic_count;
				    harmonic++) {
					sample = cp_biquad_process_sample(
					    &state->coeff[harmonic],
					    &state->state[harmonic][channel],
					    sample);
				}
			}

			output[index] = cp_dehummer_safe_sample(sample);
		}
	}

	return CP_OK;
}

int
cp_dehummer_reset(struct cp_dehummer *state)
{
	size_t channel;
	size_t harmonic;

	if (state == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	for (harmonic = 0; harmonic < CP_DEHUMMER_MAX_HARMONICS; harmonic++) {
		for (channel = 0; channel < CP_MAX_CHANNELS; channel++)
			cp_biquad_reset(&state->state[harmonic][channel]);
	}

	return CP_OK;
}

static int
cp_dehummer_design(struct cp_dehummer *state,
	const struct cp_dehummer_config *config)
{
	cp_sample_t frequency;
	size_t harmonic;
	int status;

	for (harmonic = 0; harmonic < config->harmonic_count; harmonic++) {
		frequency = config->base_frequency * (cp_sample_t)(harmonic + 1);
		status = cp_biquad_notch(&state->coeff[harmonic],
		    config->sample_rate, frequency, config->q_factor);
		if (status != CP_OK)
			return status;
	}

	return CP_OK;
}

static cp_sample_t
cp_dehummer_safe_sample(cp_sample_t sample)
{
	if (!isfinite(sample))
		return 0.0f;

	return sample;
}

static int
cp_dehummer_valid_config(const struct cp_dehummer_config *config)
{
	cp_sample_t highest_harmonic;

	if (config == NULL)
		return 0;
	if (config->channel_count != CP_CHANNELS_MONO &&
	    config->channel_count != CP_CHANNELS_STEREO)
		return 0;
	if (!isfinite(config->sample_rate) ||
	    config->sample_rate < CP_DEHUMMER_MIN_SAMPLE_RATE ||
	    config->sample_rate > CP_DEHUMMER_MAX_SAMPLE_RATE)
		return 0;
	if (!isfinite(config->base_frequency) ||
	    config->base_frequency < CP_DEHUMMER_MIN_BASE_HZ ||
	    config->base_frequency > CP_DEHUMMER_MAX_BASE_HZ)
		return 0;
	if (config->harmonic_count == 0 ||
	    config->harmonic_count > CP_DEHUMMER_MAX_HARMONICS)
		return 0;
	if (!isfinite(config->q_factor) ||
	    config->q_factor < CP_DEHUMMER_MIN_Q ||
	    config->q_factor > CP_DEHUMMER_MAX_Q)
		return 0;

	highest_harmonic = config->base_frequency *
	    (cp_sample_t)config->harmonic_count;
	if (highest_harmonic >= (config->sample_rate * 0.5f))
		return 0;

	return 1;
}

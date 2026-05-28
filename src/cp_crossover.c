/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_crossover.c */

#include <sys/types.h>

#include <math.h>
#include <stdint.h>

#include "cp_crossover.h"

static cp_sample_t	cp_crossover_clean(cp_sample_t);
static int		cp_crossover_valid_config(
			    const struct cp_crossover_config *);

void
cp_crossover_default_config(struct cp_crossover_config *config,
	size_t channels, size_t bands)
{
	size_t point;

	if (config == NULL)
		return;

	config->sample_rate = CP_CROSSOVER_DEFAULT_RATE;
	config->channels    = channels;
	config->band_count  = bands;
	for (point = 0; point < CP_CROSSOVER_MAX_POINTS; point++)
		config->frequency[point] = 0.0f;

	if (bands == 2) {
		config->frequency[0] = 250.0f;
	} else if (bands == 3) {
		config->frequency[0] = 250.0f;
		config->frequency[1] = 2500.0f;
	} else if (bands == 4) {
		config->frequency[0] = 120.0f;
		config->frequency[1] = 800.0f;
		config->frequency[2] = 3500.0f;
	}
}

int
cp_crossover_init(struct cp_crossover *state,
	const struct cp_crossover_config *config)
{
	size_t point;
	int status;

	if (state == NULL || config == NULL)
		return CP_ERR_NULL;
	if (!cp_crossover_valid_config(config))
		return CP_ERR_RANGE;

	state->config      = *config;
	state->channels    = config->channels;
	state->band_count  = config->band_count;
	state->point_count = config->band_count - 1;

	for (point = 0; point < state->point_count; point++) {
		status = cp_biquad_lowpass(&state->coeff[point],
		    config->sample_rate, config->frequency[point],
		    CP_CROSSOVER_DEFAULT_Q);
		if (status != CP_OK)
			return status;
	}

	return cp_crossover_reset(state);
}

int
cp_crossover_process(struct cp_crossover *state, const cp_sample_t *input,
	cp_sample_t **bands, size_t frames)
{
	cp_sample_t band_sample[CP_CROSSOVER_MAX_BANDS];
	size_t band;
	size_t channel;
	size_t frame;
	size_t index;
	int status;

	if (state == NULL || input == NULL || bands == NULL)
		return CP_ERR_NULL;
	for (band = 0; band < state->band_count; band++) {
		if (bands[band] == NULL)
			return CP_ERR_NULL;
	}
	if (frames > (SIZE_MAX / state->channels))
		return CP_ERR_RANGE;

	for (frame = 0; frame < frames; frame++) {
		for (channel = 0; channel < state->channels; channel++) {
			index = (frame * state->channels) + channel;
			status = cp_crossover_process_sample(state, input[index],
			    channel, band_sample);
			if (status != CP_OK)
				return status;
			for (band = 0; band < state->band_count; band++)
				bands[band][index] = band_sample[band];
		}
	}

	return CP_OK;
}

int
cp_crossover_process_sample(struct cp_crossover *state, cp_sample_t input,
	size_t channel, cp_sample_t *bands)
{
	cp_sample_t low[CP_CROSSOVER_MAX_POINTS];
	cp_sample_t sample;
	size_t point;

	if (state == NULL || bands == NULL)
		return CP_ERR_NULL;
	if (channel >= state->channels)
		return CP_ERR_CHANNELS;

	sample = cp_crossover_clean(input);
	for (point = 0; point < state->point_count; point++) {
		low[point] = cp_biquad_process_sample(&state->coeff[point],
		    &state->state[point][0][channel], sample);
		low[point] = cp_biquad_process_sample(&state->coeff[point],
		    &state->state[point][1][channel], low[point]);
	}

	bands[0] = low[0];
	for (point = 1; point < state->point_count; point++)
		bands[point] = low[point] - low[point - 1];
	bands[state->band_count - 1] = sample - low[state->point_count - 1];

	return CP_OK;
}

int
cp_crossover_reset(struct cp_crossover *state)
{
	size_t channel;
	size_t point;
	size_t section;

	if (state == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	for (point = 0; point < CP_CROSSOVER_MAX_POINTS; point++) {
		for (section = 0; section < CP_CROSSOVER_SECTIONS; section++) {
			for (channel = 0; channel < CP_MAX_CHANNELS; channel++)
				cp_biquad_reset(&state->state[point][section][channel]);
		}
	}

	return CP_OK;
}

static cp_sample_t
cp_crossover_clean(cp_sample_t sample)
{
	if (!isfinite(sample))
		return 0.0f;

	return sample;
}

static int
cp_crossover_valid_config(const struct cp_crossover_config *config)
{
	size_t point;

	if (config == NULL)
		return 0;
	if (config->channels != CP_CHANNELS_MONO &&
	    config->channels != CP_CHANNELS_STEREO)
		return 0;
	if (config->band_count < CP_CROSSOVER_MIN_BANDS ||
	    config->band_count > CP_CROSSOVER_M5_MAX_BANDS)
		return 0;
	if (!isfinite(config->sample_rate) || config->sample_rate < 8000.0f ||
	    config->sample_rate > 384000.0f)
		return 0;

	for (point = 0; point < config->band_count - 1; point++) {
		if (!isfinite(config->frequency[point]) ||
		    config->frequency[point] <= 0.0f ||
		    config->frequency[point] >= (config->sample_rate * 0.5f))
			return 0;
		if (point > 0 &&
		    config->frequency[point] <= config->frequency[point - 1])
			return 0;
	}

	return 1;
}

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_agc.c */

#include <sys/types.h>

#include <math.h>
#include <stdint.h>

#include "cp_agc.h"

#define CP_AGC_MIN_RMS	(0.000001f)
#define CP_AGC_MIN_GAIN	(0.0f)

static int	cp_agc_valid_coeff(cp_sample_t);

int
cp_agc_init(struct cp_agc *state, size_t channels, cp_sample_t target_rms,
	cp_sample_t max_gain, cp_sample_t attack_coeff, cp_sample_t release_coeff,
	cp_sample_t smooth_coeff)
{
	if (state == NULL)
		return CP_ERR_NULL;
	if (channels != CP_CHANNELS_MONO && channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (target_rms <= 0.0f || target_rms > CP_SAMPLE_MAX)
		return CP_ERR_RANGE;
	if (max_gain < 1.0f)
		return CP_ERR_RANGE;
	if (!cp_agc_valid_coeff(attack_coeff) ||
	    !cp_agc_valid_coeff(release_coeff) ||
	    !cp_agc_valid_coeff(smooth_coeff))
		return CP_ERR_RANGE;

	state->channels      = channels;
	state->target_rms    = target_rms;
	state->max_gain      = max_gain;
	state->attack_coeff  = attack_coeff;
	state->release_coeff = release_coeff;
	state->smooth_coeff  = smooth_coeff;

	return cp_agc_reset(state);
}

int
cp_agc_process(struct cp_agc *state, const cp_sample_t *input,
	cp_sample_t *output, size_t frames)
{
	cp_sample_t block_rms;
	cp_sample_t coeff;
	cp_sample_t desired_gain;
	cp_sample_t sample;
	cp_sample_t sum_sq;
	size_t index;
	size_t samples;

	if (state == NULL || input == NULL || output == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (frames == 0)
		return CP_ERR_RANGE;
	if (frames > (SIZE_MAX / state->channels))
		return CP_ERR_RANGE;

	samples = frames * state->channels;
	sum_sq  = 0.0f;

	for (index = 0; index < samples; index++) {
		sample  = input[index];
		sum_sq += sample * sample;
	}

	block_rms = sqrtf(sum_sq / (cp_sample_t)samples);
	coeff     = (block_rms > state->envelope_rms) ?
	    state->attack_coeff : state->release_coeff;

	state->envelope_rms += coeff * (block_rms - state->envelope_rms);

	if (state->envelope_rms < CP_AGC_MIN_RMS)
		desired_gain = state->max_gain;
	else
		desired_gain = state->target_rms / state->envelope_rms;

	if (desired_gain > state->max_gain)
		desired_gain = state->max_gain;
	if (desired_gain < CP_AGC_MIN_GAIN)
		desired_gain = CP_AGC_MIN_GAIN;

	state->gain += state->smooth_coeff * (desired_gain - state->gain);

	for (index = 0; index < samples; index++)
		output[index] = input[index] * state->gain;

	return CP_OK;
}

int
cp_agc_reset(struct cp_agc *state)
{
	if (state == NULL)
		return CP_ERR_NULL;
	if (state->channels != CP_CHANNELS_MONO &&
	    state->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	state->envelope_rms = state->target_rms;
	state->gain         = 1.0f;

	return CP_OK;
}

static int
cp_agc_valid_coeff(cp_sample_t coefficient)
{
	return coefficient >= 0.0f && coefficient <= 1.0f;
}

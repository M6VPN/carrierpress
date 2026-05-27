/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_block.c */

#include <sys/types.h>

#include <stdint.h>

#include "cp_block.h"

void
cp_block_default_config(struct cp_block_config *config, size_t channels)
{
	if (config == NULL)
		return;

	config->channels        = channels;
	config->dc_coefficient  = CP_DEFAULT_DC_R;
	config->target_rms      = CP_DEFAULT_TARGET_RMS;
	config->max_gain        = CP_DEFAULT_MAX_GAIN;
	config->attack_coeff    = CP_DEFAULT_ATTACK;
	config->release_coeff   = CP_DEFAULT_RELEASE;
	config->smooth_coeff    = CP_DEFAULT_SMOOTH;
	config->limiter_ceiling = CP_DEFAULT_CEILING;
}

int
cp_block_init(struct cp_block_processor *processor,
	const struct cp_block_config *config)
{
	int status;

	if (processor == NULL || config == NULL)
		return CP_ERR_NULL;
	if (config->channels != CP_CHANNELS_MONO &&
	    config->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	processor->channels = config->channels;

	status = cp_dc_blocker_init(&processor->dc_blocker, config->channels,
	    config->dc_coefficient);
	if (status != CP_OK)
		return status;

	status = cp_agc_init(&processor->agc, config->channels,
	    config->target_rms, config->max_gain, config->attack_coeff,
	    config->release_coeff, config->smooth_coeff);
	if (status != CP_OK)
		return status;

	status = cp_limiter_init(&processor->limiter, config->channels,
	    config->limiter_ceiling);
	if (status != CP_OK)
		return status;

	status = cp_meter_init(&processor->input_meter, config->channels);
	if (status != CP_OK)
		return status;

	return cp_meter_init(&processor->output_meter, config->channels);
}

int
cp_block_process(struct cp_block_processor *processor,
	const cp_sample_t *input, cp_sample_t *output, cp_sample_t *scratch,
	size_t scratch_samples, size_t frames)
{
	size_t samples;
	int status;

	if (processor == NULL || input == NULL || output == NULL ||
	    scratch == NULL)
		return CP_ERR_NULL;
	if (processor->channels != CP_CHANNELS_MONO &&
	    processor->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (frames == 0)
		return CP_ERR_RANGE;
	if (frames > (SIZE_MAX / processor->channels))
		return CP_ERR_RANGE;

	samples = frames * processor->channels;
	if (scratch_samples < samples)
		return CP_ERR_BUFFER;

	status = cp_meter_process(&processor->input_meter, input, frames);
	if (status != CP_OK)
		return status;

	status = cp_dc_blocker_process(&processor->dc_blocker, input, scratch,
	    frames);
	if (status != CP_OK)
		return status;

	status = cp_agc_process(&processor->agc, scratch, output, frames);
	if (status != CP_OK)
		return status;

	status = cp_limiter_process(&processor->limiter, output, output,
	    frames);
	if (status != CP_OK)
		return status;

	return cp_meter_process(&processor->output_meter, output, frames);
}

int
cp_block_reset(struct cp_block_processor *processor)
{
	int status;

	if (processor == NULL)
		return CP_ERR_NULL;
	if (processor->channels != CP_CHANNELS_MONO &&
	    processor->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	status = cp_dc_blocker_reset(&processor->dc_blocker);
	if (status != CP_OK)
		return status;

	status = cp_agc_reset(&processor->agc);
	if (status != CP_OK)
		return status;

	status = cp_limiter_reset(&processor->limiter);
	if (status != CP_OK)
		return status;

	status = cp_meter_reset(&processor->input_meter);
	if (status != CP_OK)
		return status;

	return cp_meter_reset(&processor->output_meter);
}

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_block.c */

#include <sys/types.h>

#include <math.h>
#include <stdint.h>

#include "cp_audio.h"
#include "cp_block.h"

void
cp_block_default_config(struct cp_block_config *config, size_t channels)
{
	if (config == NULL)
		return;

	config->channels        = channels;
	config->dc_coefficient  = CP_DEFAULT_DC_R;
	config->dehummer_enabled = CP_DEHUMMER_DEFAULT_ENABLED;
	config->hum_base_frequency = CP_DEHUMMER_DEFAULT_BASE_HZ;
	config->hum_harmonic_count = CP_DEHUMMER_DEFAULT_HARMONICS;
	config->hum_q_factor    = CP_DEHUMMER_DEFAULT_Q;
	config->target_rms      = CP_DEFAULT_TARGET_RMS;
	config->min_gain        = CP_AGC_DEFAULT_MIN_GAIN;
	config->max_gain        = CP_DEFAULT_MAX_GAIN;
	config->attack_coeff    = CP_DEFAULT_ATTACK;
	config->release_coeff   = CP_DEFAULT_RELEASE;
	config->smooth_coeff    = CP_DEFAULT_SMOOTH;
	config->attack_ms       = CP_AGC_DEFAULT_ATTACK_MS;
	config->release_ms      = CP_AGC_DEFAULT_RELEASE_MS;
	config->fast_attack_ms  = CP_AGC_DEFAULT_FAST_ATTACK_MS;
	config->hold_ms         = CP_AGC_DEFAULT_HOLD_MS;
	config->gate_threshold_db = CP_AGC_DEFAULT_GATE_DB;
	config->silence_threshold_db = CP_AGC_DEFAULT_SILENCE_DB;
	config->max_gain_step_db = CP_AGC_DEFAULT_MAX_STEP_DB;
	config->sample_rate     = CP_AGC_DEFAULT_SAMPLE_RATE;
	config->multiband_enabled = CP_MULTIBAND_DEFAULT_ENABLED;
	config->multiband_band_count = CP_MULTIBAND_DEFAULT_BANDS;
	config->multiband_preset = CP_MULTIBAND_PRESET_SPEECH;
	cp_bass_eq_default_config(&config->bass_eq_config);
	cp_am_default_config(&config->am_config);
	cp_ssb_default_config(&config->ssb_config);
	config->limiter_ceiling = CP_DEFAULT_CEILING;
}

int
cp_block_config_from_audio(struct cp_block_config *block_config,
	const struct cp_audio_config *audio_config, size_t channels,
	cp_sample_t sample_rate)
{
	if (block_config == NULL || audio_config == NULL)
		return CP_ERR_NULL;
	if (channels != CP_CHANNELS_MONO && channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (!isfinite(sample_rate) ||
	    sample_rate < (cp_sample_t)CP_AUDIO_MIN_SAMPLE_RATE ||
	    sample_rate > (cp_sample_t)CP_AUDIO_MAX_SAMPLE_RATE)
		return CP_ERR_RANGE;

	cp_block_default_config(block_config, channels);
	block_config->sample_rate = sample_rate;
	block_config->dehummer_enabled = audio_config->dehummer_enabled;
	block_config->hum_base_frequency = audio_config->hum_base_frequency;
	block_config->hum_harmonic_count = audio_config->hum_harmonic_count;
	block_config->hum_q_factor = audio_config->hum_q_factor;
	block_config->multiband_enabled = audio_config->multiband_enabled;
	block_config->multiband_band_count =
	    audio_config->multiband_band_count;
	block_config->multiband_preset = audio_config->multiband_preset;
	block_config->bass_eq_config = audio_config->bass_eq_config;
	block_config->bass_eq_config.channel_count = channels;
	block_config->bass_eq_config.sample_rate = sample_rate;
	block_config->am_config = audio_config->am_config;
	block_config->am_config.channel_count = channels;
	block_config->am_config.sample_rate = sample_rate;
	block_config->ssb_config = audio_config->ssb_config;
	block_config->ssb_config.channel_count = channels;
	block_config->ssb_config.sample_rate = sample_rate;

	if (block_config->am_config.enabled &&
	    block_config->ssb_config.enabled)
		return CP_ERR_RANGE;

	return CP_OK;
}

int
cp_block_init(struct cp_block_processor *processor,
	const struct cp_block_config *config)
{
	struct cp_agc_config agc_config;
	struct cp_bass_eq_config bass_eq_config;
	struct cp_dehummer_config dehummer_config;
	struct cp_multiband_config multiband_config;
	int status;

	if (processor == NULL || config == NULL)
		return CP_ERR_NULL;
	if (config->channels != CP_CHANNELS_MONO &&
	    config->channels != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (config->am_config.enabled && config->ssb_config.enabled)
		return CP_ERR_RANGE;

	processor->channels = config->channels;

	status = cp_dc_blocker_init(&processor->dc_blocker, config->channels,
	    config->dc_coefficient);
	if (status != CP_OK)
		return status;

	cp_dehummer_default_config(&dehummer_config);
	dehummer_config.channel_count  = config->channels;
	dehummer_config.sample_rate    = config->sample_rate;
	dehummer_config.enabled        = config->dehummer_enabled;
	dehummer_config.base_frequency = config->hum_base_frequency;
	dehummer_config.harmonic_count = config->hum_harmonic_count;
	dehummer_config.q_factor       = config->hum_q_factor;

	status = cp_dehummer_init(&processor->dehummer, &dehummer_config);
	if (status != CP_OK)
		return status;

	agc_config.target_rms           = config->target_rms;
	agc_config.min_gain             = config->min_gain;
	agc_config.max_gain             = config->max_gain;
	agc_config.attack_ms            = config->attack_ms;
	agc_config.release_ms           = config->release_ms;
	agc_config.fast_attack_ms       = config->fast_attack_ms;
	agc_config.hold_ms              = config->hold_ms;
	agc_config.gate_threshold_db    = config->gate_threshold_db;
	agc_config.silence_threshold_db = config->silence_threshold_db;
	agc_config.max_gain_step_db     = config->max_gain_step_db;
	agc_config.sample_rate          = config->sample_rate;

	status = cp_agc_init_config(&processor->agc, config->channels,
	    &agc_config);
	if (status != CP_OK)
		return status;

	cp_multiband_default_config(&multiband_config);
	multiband_config.channels   = config->channels;
	multiband_config.sample_rate = config->sample_rate;
	multiband_config.enabled    = config->multiband_enabled;
	multiband_config.band_count = config->multiband_band_count;
	multiband_config.preset     = config->multiband_preset;

	status = cp_multiband_init(&processor->multiband, &multiband_config);
	if (status != CP_OK)
		return status;

	bass_eq_config = config->bass_eq_config;
	bass_eq_config.channel_count = config->channels;
	bass_eq_config.sample_rate = config->sample_rate;
	status = cp_bass_eq_init(&processor->bass_eq, &bass_eq_config);
	if (status != CP_OK)
		return status;

	processor->am.config = config->am_config;
	processor->am.config.channel_count = config->channels;
	processor->am.config.sample_rate = config->sample_rate;
	status = cp_am_init(&processor->am, &processor->am.config);
	if (status != CP_OK)
		return status;

	processor->ssb.config = config->ssb_config;
	processor->ssb.config.channel_count = config->channels;
	processor->ssb.config.sample_rate = config->sample_rate;
	status = cp_ssb_init(&processor->ssb, &processor->ssb.config);
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

	status = cp_dehummer_process(&processor->dehummer, scratch, scratch,
	    frames);
	if (status != CP_OK)
		return status;

	status = cp_agc_process(&processor->agc, scratch, output, frames);
	if (status != CP_OK)
		return status;

	status = cp_multiband_process(&processor->multiband, output, output,
	    frames);
	if (status != CP_OK)
		return status;

	status = cp_bass_eq_process(&processor->bass_eq, output, output,
	    frames);
	if (status != CP_OK)
		return status;

	status = cp_am_process(&processor->am, output, output, frames);
	if (status != CP_OK)
		return status;

	status = cp_ssb_process(&processor->ssb, output, output, frames);
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

	status = cp_dehummer_reset(&processor->dehummer);
	if (status != CP_OK)
		return status;

	status = cp_agc_reset(&processor->agc);
	if (status != CP_OK)
		return status;

	status = cp_multiband_reset(&processor->multiband);
	if (status != CP_OK)
		return status;

	status = cp_bass_eq_reset(&processor->bass_eq);
	if (status != CP_OK)
		return status;

	status = cp_am_reset(&processor->am);
	if (status != CP_OK)
		return status;

	status = cp_ssb_reset(&processor->ssb);
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

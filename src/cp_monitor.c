/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_monitor.c */

#include <math.h>
#include <string.h>

#include "cp_am.h"
#include "cp_bass_eq.h"
#include "cp_block.h"
#include "cp_monitor.h"
#include "cp_ssb.h"

static int	cp_monitor_am_preset_id(const struct cp_am_config *);
static int	cp_monitor_ssb_preset_id(const struct cp_ssb_config *);

int
cp_monitor_db_to_centibel(cp_sample_t db)
{
	if (!isfinite(db))
		return 0;
	if (db > ((cp_sample_t)CP_MONITOR_MAX_DB / 100.0f))
		return CP_MONITOR_MAX_DB;
	if (db < ((cp_sample_t)CP_MONITOR_MIN_DB / 100.0f))
		return CP_MONITOR_MIN_DB;

	return (int)lrintf(db * 100.0f);
}

cp_sample_t
cp_monitor_centibel_to_db(int centibel)
{
	if (centibel > CP_MONITOR_MAX_DB)
		centibel = CP_MONITOR_MAX_DB;
	if (centibel < CP_MONITOR_MIN_DB)
		centibel = CP_MONITOR_MIN_DB;

	return (cp_sample_t)centibel / 100.0f;
}

cp_sample_t
cp_monitor_level_to_sample(unsigned int value)
{
	if (value > CP_MONITOR_MAX_LEVEL)
		value = CP_MONITOR_MAX_LEVEL;

	return (cp_sample_t)value / (cp_sample_t)CP_MONITOR_SCALE;
}

unsigned int
cp_monitor_sample_to_level(cp_sample_t value)
{
	if (!isfinite(value) || value <= 0.0f)
		return 0u;
	if (value >= ((cp_sample_t)CP_MONITOR_MAX_LEVEL /
	    (cp_sample_t)CP_MONITOR_SCALE))
		return CP_MONITOR_MAX_LEVEL;

	return (unsigned int)lrintf(value * (cp_sample_t)CP_MONITOR_SCALE);
}

void
cp_monitor_snapshot_clear(struct cp_monitor_snapshot *snapshot)
{
	if (snapshot == NULL)
		return;

	(void)memset(snapshot, 0, sizeof(*snapshot));
}

int
cp_monitor_snapshot_from_processor(const struct cp_block_processor *processor,
	struct cp_monitor_snapshot *snapshot)
{
	size_t band;

	if (processor == NULL || snapshot == NULL)
		return CP_ERR_NULL;

	cp_monitor_snapshot_clear(snapshot);
	snapshot->input_peak =
	    cp_monitor_sample_to_level(processor->input_meter.peak[0]);
	snapshot->input_rms =
	    cp_monitor_sample_to_level(processor->input_meter.rms[0]);
	snapshot->output_peak =
	    cp_monitor_sample_to_level(processor->output_meter.peak[0]);
	snapshot->output_rms =
	    cp_monitor_sample_to_level(processor->output_meter.rms[0]);
	snapshot->agc_gain = cp_monitor_sample_to_level(processor->agc.gain);
	snapshot->agc_gain_db_centibel =
	    cp_monitor_db_to_centibel(processor->agc.gain_db);
	snapshot->agc_state = (int)processor->agc.gate_state;
	snapshot->dsp_status = CP_OK;
	snapshot->dehummer_enabled =
	    processor->dehummer.config.enabled ? 1u : 0u;
	snapshot->dehummer_base_hz =
	    (unsigned int)lrintf(processor->dehummer.config.base_frequency);
	snapshot->dehummer_harmonic_count =
	    (unsigned int)processor->dehummer.config.harmonic_count;
	snapshot->multiband_enabled =
	    processor->multiband.config.enabled ? 1u : 0u;
	snapshot->multiband_preset = processor->multiband.config.preset;
	snapshot->restoration_enabled =
	    processor->restoration.config.enabled ? 1u : 0u;
	snapshot->restoration_clipped_ratio = cp_monitor_sample_to_level(
	    processor->restoration.metrics.clipped_sample_ratio);
	snapshot->restoration_hf_ratio = cp_monitor_sample_to_level(
	    processor->restoration.metrics.high_frequency_ratio);
	snapshot->restoration_clipping_confidence =
	    cp_monitor_sample_to_level(
	    processor->restoration.metrics.clipping_confidence);
	snapshot->restoration_lossy_confidence =
	    cp_monitor_sample_to_level(
	    processor->restoration.metrics.lossy_confidence);
	snapshot->restoration_low_ceiling_confidence =
	    cp_monitor_sample_to_level(
	    processor->restoration.metrics.low_ceiling_clipping_confidence);
	snapshot->restoration_transient_confidence =
	    cp_monitor_sample_to_level(
	    processor->restoration.metrics.transient_confidence);
	snapshot->restoration_flat_run_ratio =
	    cp_monitor_sample_to_level(
	    processor->restoration.metrics.flat_run_ratio);
	snapshot->restoration_peak_repeat_ratio =
	    cp_monitor_sample_to_level(
	    processor->restoration.metrics.peak_repeat_ratio);
	snapshot->restoration_observed_peak =
	    cp_monitor_sample_to_level(
	    processor->restoration.metrics.observed_peak);
	snapshot->restoration_crest_factor =
	    cp_monitor_sample_to_level(
	    processor->restoration.metrics.crest_factor);
	snapshot->restoration_flat_runs =
	    (unsigned int)processor->restoration.metrics.flat_run_count;
	snapshot->restoration_peak_repeats =
	    (unsigned int)processor->restoration.metrics.peak_repeat_count;
	snapshot->restoration_reason_flags =
	    processor->restoration.metrics.reason_flags;
	snapshot->restoration_source_profile =
	    (int)processor->restoration.metrics.source_profile;
	snapshot->bass_eq_enabled =
	    processor->bass_eq.config.enabled ? 1u : 0u;
	snapshot->bass_eq_low_hz =
	    (unsigned int)lrintf(processor->bass_eq.config.low_shelf_hz);
	snapshot->bass_eq_low_gain_db_centibel =
	    cp_monitor_db_to_centibel(processor->bass_eq.config.low_gain_db);
	snapshot->bass_eq_high_hz =
	    (unsigned int)lrintf(processor->bass_eq.config.high_shelf_hz);
	snapshot->bass_eq_high_gain_db_centibel =
	    cp_monitor_db_to_centibel(processor->bass_eq.config.high_gain_db);
	snapshot->bass_eq_preset = processor->bass_eq.config.preset;
	snapshot->am_enabled = processor->am.config.enabled ? 1u : 0u;
	snapshot->am_highpass_hz =
	    (unsigned int)lrintf(processor->am.config.highpass_hz);
	snapshot->am_lowpass_hz =
	    (unsigned int)lrintf(processor->am.config.lowpass_hz);
	snapshot->am_positive_peak =
	    cp_monitor_sample_to_level(processor->am.config.positive_peak_limit);
	snapshot->am_negative_peak =
	    cp_monitor_sample_to_level(processor->am.config.negative_peak_limit);
	snapshot->am_asymmetry_enabled =
	    processor->am.config.asymmetry_enabled ? 1u : 0u;
	snapshot->am_asymmetry_ratio =
	    cp_monitor_sample_to_level(processor->am.config.asymmetry_ratio);
	snapshot->am_preset = cp_monitor_am_preset_id(&processor->am.config);
	snapshot->ssb_enabled = processor->ssb.config.enabled ? 1u : 0u;
	snapshot->ssb_highpass_hz =
	    (unsigned int)lrintf(processor->ssb.config.highpass_hz);
	snapshot->ssb_lowpass_hz =
	    (unsigned int)lrintf(processor->ssb.config.lowpass_hz);
	snapshot->ssb_peak_limit =
	    cp_monitor_sample_to_level(processor->ssb.config.peak_limit);
	snapshot->ssb_phase_rotator_enabled =
	    processor->ssb.config.phase_rotator_enabled ? 1u : 0u;
	snapshot->ssb_preset = cp_monitor_ssb_preset_id(
	    &processor->ssb.config);
	snapshot->band_count = processor->multiband.band_count;
	if (snapshot->band_count > CP_MONITOR_MAX_BANDS)
		snapshot->band_count = CP_MONITOR_MAX_BANDS;
	for (band = 0; band < snapshot->band_count; band++) {
		snapshot->band_rms[band] = cp_monitor_sample_to_level(
		    processor->multiband.band_rms[band]);
		snapshot->band_gr_db_centibel[band] =
		    cp_monitor_db_to_centibel(
		    processor->multiband.band_gain_reduction_db[band]);
	}

	return CP_OK;
}

static int
cp_monitor_am_preset_id(const struct cp_am_config *config)
{
	enum cp_am_preset preset;

	if (config == NULL)
		return (int)CP_AM_PRESET_SAFE;
	if (cp_am_preset_from_string(config->preset_name, &preset) == CP_OK)
		return (int)preset;

	return (int)CP_AM_PRESET_SAFE;
}

static int
cp_monitor_ssb_preset_id(const struct cp_ssb_config *config)
{
	enum cp_ssb_preset preset;

	if (config == NULL)
		return (int)CP_SSB_PRESET_SPEECH;
	if (cp_ssb_preset_from_string(config->preset_name, &preset) == CP_OK)
		return (int)preset;

	return (int)CP_SSB_PRESET_SPEECH;
}

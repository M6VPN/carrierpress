/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_monitor.h */

#ifndef CP_MONITOR_H
#define CP_MONITOR_H

#include "cp_bass_eq.h"
#include "cp_multiband.h"
#include "cp_types.h"

#define CP_MONITOR_SCALE		1000000u
#define CP_MONITOR_MAX_LEVEL		4000000u
#define CP_MONITOR_MAX_DB		9600
#define CP_MONITOR_MIN_DB		(-9600)
#define CP_MONITOR_MAX_BANDS		CP_MULTIBAND_M5_MAX_BANDS

struct cp_block_processor;

enum cp_monitor_stream_flag {
	CP_MONITOR_INPUT_UNDERFLOW  = 1u << 0,
	CP_MONITOR_INPUT_OVERFLOW   = 1u << 1,
	CP_MONITOR_OUTPUT_UNDERFLOW = 1u << 2,
	CP_MONITOR_OUTPUT_OVERFLOW  = 1u << 3,
	CP_MONITOR_PRIMING_OUTPUT   = 1u << 4
};

struct cp_monitor_snapshot {
	unsigned int input_peak;
	unsigned int input_rms;
	unsigned int output_peak;
	unsigned int output_rms;
	unsigned int agc_gain;
	int agc_gain_db_centibel;
	int agc_state;
	unsigned int stream_flags;
	int dsp_status;
	unsigned int dehummer_enabled;
	unsigned int dehummer_base_hz;
	unsigned int dehummer_harmonic_count;
	unsigned int declipper_enabled;
	unsigned int declipper_repaired_samples;
	unsigned int declipper_repaired_runs;
	unsigned int declipper_max_delta;
	int declipper_bypass_reason;
	unsigned int declipper_finite;
	unsigned int natural_dynamics_enabled;
	unsigned int natural_dynamics_rms;
	unsigned int natural_dynamics_gain;
	int natural_dynamics_gr_db_centibel;
	unsigned int low_level_boost_enabled;
	unsigned int low_level_boost_rms;
	unsigned int low_level_boost_gain;
	int low_level_boost_gain_db_centibel;
	int low_level_boost_state;
	unsigned int multiband_enabled;
	int multiband_preset;
	unsigned int multiband2_enabled;
	int multiband2_preset;
	unsigned int restoration_enabled;
	unsigned int restoration_clipped_ratio;
	unsigned int restoration_hf_ratio;
	unsigned int restoration_clipping_confidence;
	unsigned int restoration_lossy_confidence;
	unsigned int restoration_low_ceiling_confidence;
	unsigned int restoration_transient_confidence;
	unsigned int restoration_flat_run_ratio;
	unsigned int restoration_peak_repeat_ratio;
	unsigned int restoration_observed_peak;
	unsigned int restoration_crest_factor;
	unsigned int restoration_flat_runs;
	unsigned int restoration_peak_repeats;
	unsigned int restoration_reason_flags;
	int restoration_source_profile;
	unsigned int bass_eq_enabled;
	unsigned int bass_eq_low_hz;
	int bass_eq_low_gain_db_centibel;
	unsigned int bass_eq_high_hz;
	int bass_eq_high_gain_db_centibel;
	int bass_eq_preset;
	unsigned int am_enabled;
	unsigned int am_highpass_hz;
	unsigned int am_lowpass_hz;
	unsigned int am_positive_peak;
	unsigned int am_negative_peak;
	unsigned int am_asymmetry_enabled;
	unsigned int am_asymmetry_ratio;
	int am_preset;
	unsigned int ssb_enabled;
	unsigned int ssb_highpass_hz;
	unsigned int ssb_lowpass_hz;
	unsigned int ssb_peak_limit;
	unsigned int ssb_phase_rotator_enabled;
	int ssb_preset;
	int control_command;
	int control_status;
	size_t band_count;
	unsigned int band_rms[CP_MONITOR_MAX_BANDS];
	int band_gr_db_centibel[CP_MONITOR_MAX_BANDS];
	size_t band2_count;
	unsigned int band2_rms[CP_MONITOR_MAX_BANDS];
	int band2_gr_db_centibel[CP_MONITOR_MAX_BANDS];
};

int		cp_monitor_db_to_centibel(cp_sample_t);
cp_sample_t	cp_monitor_centibel_to_db(int);
cp_sample_t	cp_monitor_level_to_sample(unsigned int);
unsigned int	cp_monitor_sample_to_level(cp_sample_t);
void		cp_monitor_snapshot_clear(struct cp_monitor_snapshot *);
int		cp_monitor_snapshot_from_processor(
		    const struct cp_block_processor *,
		    struct cp_monitor_snapshot *);

#endif

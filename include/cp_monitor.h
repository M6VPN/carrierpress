/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_monitor.h */

#ifndef CP_MONITOR_H
#define CP_MONITOR_H

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
	unsigned int multiband_enabled;
	int multiband_preset;
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

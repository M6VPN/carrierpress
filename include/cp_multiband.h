/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_multiband.h */

#ifndef CP_MULTIBAND_H
#define CP_MULTIBAND_H

#include "cp_compressor.h"
#include "cp_crossover.h"
#include "cp_types.h"

#define CP_MULTIBAND_DEFAULT_ENABLED		0
#define CP_MULTIBAND_DEFAULT_BANDS		3
#define CP_MULTIBAND_DEFAULT_RATE		(48000.0f)
#define CP_MULTIBAND_MIN_BANDS			CP_CROSSOVER_MIN_BANDS
#define CP_MULTIBAND_MAX_BANDS			CP_CROSSOVER_MAX_BANDS
#define CP_MULTIBAND_M5_MAX_BANDS		CP_CROSSOVER_M5_MAX_BANDS

enum cp_multiband_preset {
	CP_MULTIBAND_PRESET_SPEECH = 0,
	CP_MULTIBAND_PRESET_MUSIC  = 1
};

enum cp_multiband_stage {
	CP_MULTIBAND_STAGE_PRIMARY = 0,
	CP_MULTIBAND_STAGE_POLISH  = 1
};

struct cp_multiband_config {
	cp_sample_t sample_rate;
	size_t channels;
	size_t band_count;
	enum cp_multiband_preset preset;
	int enabled;
	enum cp_multiband_stage stage;
};

struct cp_multiband {
	struct cp_multiband_config config;
	struct cp_crossover crossover;
	struct cp_compressor compressor[CP_MULTIBAND_MAX_BANDS];
	cp_sample_t band_peak[CP_MULTIBAND_MAX_BANDS];
	cp_sample_t band_rms[CP_MULTIBAND_MAX_BANDS];
	cp_sample_t band_gain_reduction_db[CP_MULTIBAND_MAX_BANDS];
	size_t channels;
	size_t band_count;
	int enabled;
};

void		cp_multiband_default_config(struct cp_multiband_config *);
int		cp_multiband_init(struct cp_multiband *,
		    const struct cp_multiband_config *);
int		cp_multiband_process(struct cp_multiband *,
		    const cp_sample_t *, cp_sample_t *, size_t);
int		cp_multiband_reset(struct cp_multiband *);
const char	*cp_multiband_preset_string(enum cp_multiband_preset);

#endif

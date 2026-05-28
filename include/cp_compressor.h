/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_compressor.h */

#ifndef CP_COMPRESSOR_H
#define CP_COMPRESSOR_H

#include "cp_types.h"

#define CP_COMPRESSOR_DEFAULT_THRESHOLD_DB	(-18.0f)
#define CP_COMPRESSOR_DEFAULT_RATIO		(2.0f)
#define CP_COMPRESSOR_DEFAULT_ATTACK_MS		(20.0f)
#define CP_COMPRESSOR_DEFAULT_RELEASE_MS		(250.0f)
#define CP_COMPRESSOR_DEFAULT_MAKEUP_DB		(0.0f)
#define CP_COMPRESSOR_DEFAULT_KNEE_DB		(6.0f)
#define CP_COMPRESSOR_DEFAULT_RATE		(48000.0f)

struct cp_compressor_config {
	cp_sample_t threshold_db;
	cp_sample_t ratio;
	cp_sample_t attack_ms;
	cp_sample_t release_ms;
	cp_sample_t makeup_gain_db;
	cp_sample_t knee_db;
	cp_sample_t sample_rate;
	size_t channels;
	int enabled;
};

struct cp_compressor {
	struct cp_compressor_config config;
	size_t channels;
	cp_sample_t envelope_db;
	cp_sample_t gain_db;
	cp_sample_t gain_reduction_db;
	cp_sample_t makeup_gain;
};

void	cp_compressor_default_config(struct cp_compressor_config *);
int	cp_compressor_init(struct cp_compressor *,
	    const struct cp_compressor_config *);
int	cp_compressor_process(struct cp_compressor *, const cp_sample_t *,
	    cp_sample_t *, size_t);
int	cp_compressor_process_frame(struct cp_compressor *, const cp_sample_t *,
	    cp_sample_t *);
int	cp_compressor_reset(struct cp_compressor *);

#endif

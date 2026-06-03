/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_audio.h */

#ifndef CP_AUDIO_H
#define CP_AUDIO_H

#include "cp_am.h"
#include "cp_multiband.h"
#include "cp_types.h"

#define CP_AUDIO_DEFAULT_DEVICE		(-1)
#define CP_AUDIO_DEFAULT_SAMPLE_RATE	48000.0
#define CP_AUDIO_DEFAULT_CHANNELS	CP_CHANNELS_STEREO
#define CP_AUDIO_DEFAULT_BLOCK_SIZE	256
#define CP_AUDIO_DEFAULT_METER_MS	1000
#define CP_AUDIO_MIN_SAMPLE_RATE	8000.0
#define CP_AUDIO_MAX_SAMPLE_RATE	192000.0
#define CP_AUDIO_MIN_BLOCK_SIZE		16
#define CP_AUDIO_MAX_BLOCK_SIZE		4096
#define CP_AUDIO_MIN_METER_MS		100
#define CP_AUDIO_MAX_METER_MS		60000

enum cp_audio_backend {
	CP_AUDIO_BACKEND_AUTO = 0,
	CP_AUDIO_BACKEND_JACK,
	CP_AUDIO_BACKEND_ALSA,
	CP_AUDIO_BACKEND_PULSE,
	CP_AUDIO_BACKEND_DEFAULT
};

enum cp_audio_status {
	CP_AUDIO_OK          = 0,
	CP_AUDIO_ERR_NULL    = -200,
	CP_AUDIO_ERR_DEVICE  = -201,
	CP_AUDIO_ERR_RATE    = -202,
	CP_AUDIO_ERR_CHANNEL = -203,
	CP_AUDIO_ERR_BLOCK   = -204,
	CP_AUDIO_ERR_METER   = -205,
	CP_AUDIO_ERR_HUM     = -206,
	CP_AUDIO_ERR_MB      = -207,
	CP_AUDIO_ERR_AM      = -208,
	CP_AUDIO_ERR_BACKEND = -209
};

struct cp_audio_device_candidate {
	int index;
	const char *name;
	const char *host_api;
	int max_input_channels;
	int max_output_channels;
	double default_sample_rate;
	int default_input;
	int default_output;
};

struct cp_audio_config {
	int input_device;
	int output_device;
	enum cp_audio_backend backend;
	const char *device_name;
	double sample_rate;
	int sample_rate_explicit;
	size_t channels;
	size_t block_size;
	unsigned int meter_interval_ms;
	int dehummer_enabled;
	cp_sample_t hum_base_frequency;
	size_t hum_harmonic_count;
	cp_sample_t hum_q_factor;
	int multiband_enabled;
	size_t multiband_band_count;
	enum cp_multiband_preset multiband_preset;
	struct cp_am_config am_config;
	int tui_enabled;
};

int		cp_audio_backend_from_string(const char *,
		    enum cp_audio_backend *);
const char	*cp_audio_backend_string(enum cp_audio_backend);
int		cp_audio_choose_sample_rate(const struct cp_audio_config *,
		    double, double, int, int, int, double *);
void		cp_audio_default_config(struct cp_audio_config *);
int		cp_audio_select_device_candidate(
		    const struct cp_audio_config *,
		    const struct cp_audio_device_candidate *, size_t, int *);
const char	*cp_audio_status_string(int);
int		cp_audio_validate_config(const struct cp_audio_config *);

#endif

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_types.h */

#ifndef CP_TYPES_H
#define CP_TYPES_H

#include <stddef.h>

typedef float cp_sample_t;

enum cp_status {
	CP_OK           = 0,
	CP_ERR_NULL     = -1,
	CP_ERR_RANGE    = -2,
	CP_ERR_CHANNELS = -3,
	CP_ERR_BUFFER   = -4
};

enum cp_channels {
	CP_CHANNELS_MONO   = 1,
	CP_CHANNELS_STEREO = 2,
	CP_MAX_CHANNELS    = 2
};

#define CP_SAMPLE_MIN		(-1.0f)
#define CP_SAMPLE_MAX		(1.0f)
#define CP_DEFAULT_CEILING	(0.95f)
#define CP_DEFAULT_DC_R		(0.995f)
#define CP_DEFAULT_TARGET_RMS	(0.20f)
#define CP_DEFAULT_MAX_GAIN	(8.0f)
#define CP_DEFAULT_ATTACK	(0.20f)
#define CP_DEFAULT_RELEASE	(0.02f)
#define CP_DEFAULT_SMOOTH	(0.10f)

#endif

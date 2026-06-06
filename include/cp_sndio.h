/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_sndio.h */

#ifndef CP_SNDIO_H
#define CP_SNDIO_H

#include <signal.h>

#include "cp_audio.h"

enum cp_sndio_status {
	CP_SNDIO_OK          = 0,
	CP_SNDIO_ERR_CONFIG  = -800,
	CP_SNDIO_ERR_OPEN    = -801,
	CP_SNDIO_ERR_PARAM   = -802,
	CP_SNDIO_ERR_ALLOC   = -803,
	CP_SNDIO_ERR_DSP     = -804,
	CP_SNDIO_ERR_START   = -805,
	CP_SNDIO_ERR_READ    = -806,
	CP_SNDIO_ERR_WRITE   = -807
};

int		cp_sndio_run(const struct cp_audio_config *,
		    volatile sig_atomic_t *);
const char	*cp_sndio_status_string(int);

#endif

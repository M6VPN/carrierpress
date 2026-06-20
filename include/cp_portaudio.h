/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_portaudio.h */

#ifndef CP_PORTAUDIO_H
#define CP_PORTAUDIO_H

#include <signal.h>

#include "cp_audio.h"
#include "cp_cat.h"

enum cp_portaudio_status {
	CP_PORTAUDIO_OK          = 0,
	CP_PORTAUDIO_ERR_CONFIG  = -300,
	CP_PORTAUDIO_ERR_INIT    = -301,
	CP_PORTAUDIO_ERR_DEVICE  = -302,
	CP_PORTAUDIO_ERR_ALLOC   = -303,
	CP_PORTAUDIO_ERR_DSP     = -304,
	CP_PORTAUDIO_ERR_STREAM  = -305,
	CP_PORTAUDIO_ERR_START   = -306,
	CP_PORTAUDIO_ERR_STOP    = -307
};

int		cp_portaudio_list_devices(void);
int		cp_portaudio_run(const struct cp_audio_config *,
		    const struct cp_cat_config *, volatile sig_atomic_t *);
const char	*cp_portaudio_status_string(int);

#endif

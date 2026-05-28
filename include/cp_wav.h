/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_wav.h */

#ifndef CP_WAV_H
#define CP_WAV_H

#include "cp_block.h"
#include "cp_types.h"

enum cp_wav_status {
	CP_WAV_OK            = 0,
	CP_WAV_ERR_NULL      = -100,
	CP_WAV_ERR_OPEN_IN   = -101,
	CP_WAV_ERR_OPEN_OUT  = -102,
	CP_WAV_ERR_CHANNELS  = -103,
	CP_WAV_ERR_FORMAT    = -104,
	CP_WAV_ERR_ALLOC     = -105,
	CP_WAV_ERR_READ      = -106,
	CP_WAV_ERR_WRITE     = -107,
	CP_WAV_ERR_PROCESS   = -108
};

int		cp_wav_process_file(const char *, const char *, size_t);
int		cp_wav_process_file_config(const char *, const char *, size_t,
		    const struct cp_block_config *);
const char	*cp_wav_status_string(int);

#endif

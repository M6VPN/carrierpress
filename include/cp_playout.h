/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_playout.h */

#ifndef CP_PLAYOUT_H
#define CP_PLAYOUT_H

#include <sys/types.h>

#include "cp_audio.h"
#include "cp_block.h"

#define CP_PLAYOUT_DEFAULT_BLOCK_FRAMES	512
#define CP_PLAYOUT_MAX_LINE		4096

enum cp_playout_status {
	CP_PLAYOUT_OK             = 0,
	CP_PLAYOUT_ERR_NULL       = -700,
	CP_PLAYOUT_ERR_ALLOC      = -701,
	CP_PLAYOUT_ERR_PLAYLIST   = -702,
	CP_PLAYOUT_ERR_OPEN_IN    = -703,
	CP_PLAYOUT_ERR_FORMAT     = -704,
	CP_PLAYOUT_ERR_CHANNELS   = -705,
	CP_PLAYOUT_ERR_AUDIO      = -706,
	CP_PLAYOUT_ERR_STREAM     = -707,
	CP_PLAYOUT_ERR_PROCESS    = -708,
	CP_PLAYOUT_ERR_READ       = -709,
	CP_PLAYOUT_ERR_WRITE      = -710,
	CP_PLAYOUT_ERR_UNSUPPORTED = -711
};

struct cp_playlist {
	char **paths;
	size_t count;
	size_t capacity;
};

struct cp_playout_config {
	struct cp_audio_config audio_config;
	struct cp_block_config block_config;
	size_t block_frames;
};

void		cp_playout_default_config(struct cp_playout_config *);
void		cp_playlist_free(struct cp_playlist *);
const char	*cp_playlist_get(const struct cp_playlist *, size_t);
int		cp_playlist_load(const char *, struct cp_playlist *);
size_t		cp_playlist_count(const struct cp_playlist *);
int		cp_playout_path_is_wav(const char *);
int		cp_playout_run_file(const char *,
		    const struct cp_playout_config *);
int		cp_playout_run_playlist(const char *,
		    const struct cp_playout_config *);
const char	*cp_playout_status_string(int);

#endif

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_playlist_check.h */

#ifndef CP_PLAYLIST_CHECK_H
#define CP_PLAYLIST_CHECK_H

#include <sys/types.h>

#include <stdio.h>

#define CP_PLAYLIST_CHECK_MAX_LINE	4096

enum cp_playlist_check_status {
	CP_PLAYLIST_CHECK_OK       = 0,
	CP_PLAYLIST_CHECK_ERR_NULL = -900,
	CP_PLAYLIST_CHECK_ERR_OPEN = -901,
	CP_PLAYLIST_CHECK_ERR_BAD  = -902
};

struct cp_playlist_check_result {
	size_t total_lines;
	size_t playable_entries;
	size_t skipped_lines;
	size_t unsupported_entries;
	size_t too_long_lines;
	size_t errors;
};

void	cp_playlist_check_result_init(struct cp_playlist_check_result *);
int	cp_playlist_check_file(const char *,
	    struct cp_playlist_check_result *, FILE *);
int	cp_playlist_check_path_is_wav(const char *);
const char	*cp_playlist_check_status_string(int);

#endif

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_cat.h */

#ifndef CP_CAT_H
#define CP_CAT_H

#include <sys/types.h>

#include <stdint.h>

#include "cp_types.h"

#define CP_CAT_MODE_TEXT	16
#define CP_CAT_STATUS_TEXT	64
#define CP_CAT_HOST_TEXT	128
#define CP_CAT_DEFAULT_FREQUENCY_HZ	14230000ULL
#define CP_CAT_FLRIG_DEFAULT_HOST	"127.0.0.1"
#define CP_CAT_FLRIG_DEFAULT_PORT	12345u
#define CP_CAT_DEFAULT_TIMEOUT_MS	150u
#define CP_CAT_MIN_PORT		1u
#define CP_CAT_MAX_PORT		65535u
#define CP_CAT_MIN_TIMEOUT_MS	10u
#define CP_CAT_MAX_TIMEOUT_MS	5000u

enum cp_cat_backend {
	CP_CAT_BACKEND_NONE = 0,
	CP_CAT_BACKEND_MOCK,
	CP_CAT_BACKEND_FLRIG,
	CP_CAT_BACKEND_HAMLIB
};

enum cp_cat_status {
	CP_CAT_STATUS_DISABLED = 0,
	CP_CAT_STATUS_UNAVAILABLE,
	CP_CAT_STATUS_OK,
	CP_CAT_STATUS_STALE,
	CP_CAT_STATUS_ERROR
};

enum cp_cat_ptt_state {
	CP_CAT_PTT_UNKNOWN = 0,
	CP_CAT_PTT_OFF,
	CP_CAT_PTT_ON
};

struct cp_cat_config {
	enum cp_cat_backend backend;
	int enabled;
	uint64_t mock_frequency_hz;
	char mock_mode[CP_CAT_MODE_TEXT];
	enum cp_cat_ptt_state mock_ptt;
	enum cp_cat_status mock_status;
	char flrig_host[CP_CAT_HOST_TEXT];
	unsigned int flrig_port;
	unsigned int timeout_ms;
};

struct cp_cat_snapshot {
	enum cp_cat_backend backend;
	int enabled;
	int connected;
	enum cp_cat_status status;
	uint64_t frequency_hz;
	char mode[CP_CAT_MODE_TEXT];
	enum cp_cat_ptt_state ptt;
	char status_text[CP_CAT_STATUS_TEXT];
};

const char	*cp_cat_backend_string(enum cp_cat_backend);
int		cp_cat_backend_from_string(const char *,
		    enum cp_cat_backend *);
int		cp_cat_config_set_flrig_host(struct cp_cat_config *,
		    const char *);
void		cp_cat_default_config(struct cp_cat_config *);
int		cp_cat_format_frequency(uint64_t, char *, size_t);
int		cp_cat_mode_set(char *, size_t, const char *);
const char	*cp_cat_ptt_string(enum cp_cat_ptt_state);
int		cp_cat_ptt_from_string(const char *,
		    enum cp_cat_ptt_state *);
int		cp_cat_snapshot_format(const struct cp_cat_snapshot *,
		    char *, size_t);
int		cp_cat_snapshot_update(const struct cp_cat_config *,
		    struct cp_cat_snapshot *);
const char	*cp_cat_status_string(enum cp_cat_status);
int		cp_cat_validate_config(const struct cp_cat_config *);

#endif

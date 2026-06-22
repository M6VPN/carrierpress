/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_selector.h */

#ifndef CP_SELECTOR_H
#define CP_SELECTOR_H

#include <sys/types.h>

#include "cp_types.h"

struct cp_audio_device_candidate;

#define CP_SELECTOR_LABEL_MAX	96
#define CP_SELECTOR_VALUE_MAX	256
#define CP_SELECTOR_ITEMS_MAX	32

enum cp_selector_kind {
	CP_SELECTOR_OUTPUT_DEVICE = 0,
	CP_SELECTOR_AUDIO_FILE,
	CP_SELECTOR_PLAYLIST
};

struct cp_selector_item {
	char label[CP_SELECTOR_LABEL_MAX];
	char value[CP_SELECTOR_VALUE_MAX];
	int enabled;
};

struct cp_selector {
	enum cp_selector_kind kind;
	struct cp_selector_item items[CP_SELECTOR_ITEMS_MAX];
	size_t count;
	size_t selected;
};

int			 cp_selector_add(struct cp_selector *, const char *,
			    const char *, int);
int			 cp_selector_add_audio_file(struct cp_selector *,
			    const char *, const char *, const char *,
			    const char *);
int			 cp_selector_add_playlist(struct cp_selector *,
			    const char *, const char *, const char *,
			    const char *);
const struct cp_selector_item
			*cp_selector_current(const struct cp_selector *);
int			 cp_selector_format_line(const struct cp_selector *,
			    char *, size_t);
int			 cp_selector_format_menu_item(
			    const struct cp_selector *, size_t, char *,
			    size_t);
void			 cp_selector_init(struct cp_selector *,
			    enum cp_selector_kind);
const char		*cp_selector_kind_string(enum cp_selector_kind);
int			 cp_selector_load_output_devices(
			    struct cp_selector *,
			    const struct cp_audio_device_candidate *, size_t,
			    int, int, int);
int			 cp_selector_load_audio_files(struct cp_selector *,
			    const char *const *, size_t, const char *,
			    const char *);
int			 cp_selector_load_playlists(struct cp_selector *,
			    const char *const *, size_t, const char *,
			    const char *);
int			 cp_selector_next(struct cp_selector *);
int			 cp_selector_prev(struct cp_selector *);
int			 cp_selector_select(struct cp_selector *, size_t);

#endif

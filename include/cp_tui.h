/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_tui.h */

#ifndef CP_TUI_H
#define CP_TUI_H

#include "cp_audio.h"
#include "cp_cat.h"
#include "cp_control.h"
#include "cp_monitor.h"

struct cp_tui {
	int active;
	int control_bank_set;
	enum cp_control_bank control_bank;
};

enum cp_tui_mode {
	CP_TUI_MODE_LIVE = 0,
	CP_TUI_MODE_PLAYOUT
};

struct cp_tui_view {
	enum cp_tui_mode mode;
	const struct cp_audio_config *config;
	const struct cp_monitor_snapshot *snapshot;
	const struct cp_cat_snapshot *cat_snapshot;
	const char *path;
	size_t playlist_index;
	size_t playlist_count;
	int next_enabled;
	int output_device;
};

void	cp_tui_close(struct cp_tui *);
const char
	*cp_tui_active_mode_string(const struct cp_monitor_snapshot *);
int	cp_tui_format_cat_status(const struct cp_cat_snapshot *,
	    char *, size_t);
int	cp_tui_format_key_help(const struct cp_tui_view *,
	    enum cp_control_bank, char *, size_t);
int	cp_tui_format_mode_status(const struct cp_monitor_snapshot *,
	    enum cp_control_bank, char *, size_t);
int	cp_tui_init(struct cp_tui *);
int	cp_tui_update(struct cp_tui *, const struct cp_audio_config *,
	    const struct cp_monitor_snapshot *, struct cp_control_command *);
int	cp_tui_update_view(struct cp_tui *, const struct cp_tui_view *,
	    struct cp_control_command *);

#endif

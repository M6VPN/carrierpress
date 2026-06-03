/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_tui.h */

#ifndef CP_TUI_H
#define CP_TUI_H

#include "cp_audio.h"
#include "cp_control.h"
#include "cp_monitor.h"

struct cp_tui {
	int active;
};

enum cp_tui_mode {
	CP_TUI_MODE_LIVE = 0,
	CP_TUI_MODE_PLAYOUT
};

struct cp_tui_view {
	enum cp_tui_mode mode;
	const struct cp_audio_config *config;
	const struct cp_monitor_snapshot *snapshot;
	const char *path;
	size_t playlist_index;
	size_t playlist_count;
	int next_enabled;
	int output_device;
};

void	cp_tui_close(struct cp_tui *);
int	cp_tui_init(struct cp_tui *);
int	cp_tui_update(struct cp_tui *, const struct cp_audio_config *,
	    const struct cp_monitor_snapshot *, struct cp_control_command *);
int	cp_tui_update_view(struct cp_tui *, const struct cp_tui_view *,
	    struct cp_control_command *);

#endif

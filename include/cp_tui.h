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

void	cp_tui_close(struct cp_tui *);
int	cp_tui_init(struct cp_tui *);
int	cp_tui_update(struct cp_tui *, const struct cp_audio_config *,
	    const struct cp_monitor_snapshot *, struct cp_control_command *);

#endif

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_gui_format.h */

#ifndef CP_GUI_FORMAT_H
#define CP_GUI_FORMAT_H

#include <sys/types.h>

#include "cp_audio.h"
#include "cp_cat.h"
#include "cp_control.h"
#include "cp_gui_workflow.h"
#include "cp_monitor.h"
#include "cp_operator_state.h"

enum cp_gui_mode {
	CP_GUI_MODE_LIVE = 0,
	CP_GUI_MODE_PLAYOUT,
	CP_GUI_MODE_DEMO
};

int	cp_gui_format_agc(const struct cp_monitor_snapshot *, char *,
	    size_t);
int	cp_gui_format_cat(const struct cp_cat_snapshot *, char *, size_t);
int	cp_gui_format_chain(const struct cp_monitor_snapshot *, char *,
	    size_t);
int	cp_gui_format_cue_slots(const char *, const char *, char *, size_t);
int	cp_gui_format_flags(unsigned int, char *, size_t);
int	cp_gui_format_help(enum cp_control_bank, int, char *, size_t);
int	cp_gui_format_meters(const struct cp_monitor_snapshot *, char *,
	    size_t);
int	cp_gui_format_mode(const struct cp_monitor_snapshot *, char *,
	    size_t);
int	cp_gui_format_operator_state(const struct cp_operator_state *,
	    char *, size_t);
int	cp_gui_format_output_device(const struct cp_audio_config *, int,
	    const struct cp_gui_workflow_request *, char *, size_t);
int	cp_gui_format_truncate(const char *, char *, size_t, size_t);
int	cp_gui_format_transport(enum cp_gui_mode,
	    const struct cp_audio_config *, const char *, size_t, size_t, int,
	    char *, size_t);
int	cp_gui_format_workflow(const struct cp_gui_workflow_request *,
	    char *, size_t);
int	cp_gui_control_command_from_key(int, enum cp_control_bank, int,
	    struct cp_control_command *);

#endif

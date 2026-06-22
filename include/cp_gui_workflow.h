/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_gui_workflow.h */

#ifndef CP_GUI_WORKFLOW_H
#define CP_GUI_WORKFLOW_H

#include <sys/types.h>

#include "cp_types.h"

struct cp_selector;

#define CP_GUI_WORKFLOW_PATH_SIZE 512
#define CP_GUI_WORKFLOW_REASON_SIZE 128

enum cp_gui_workflow_request_type {
	CP_GUI_WORKFLOW_REQUEST_NONE = 0,
	CP_GUI_WORKFLOW_REQUEST_LOAD_WAV,
	CP_GUI_WORKFLOW_REQUEST_LOAD_PLAYLIST,
	CP_GUI_WORKFLOW_REQUEST_CUE_PLAYLIST_ITEM,
	CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE
};

struct cp_gui_workflow_request {
	enum cp_gui_workflow_request_type type;
	char path[CP_GUI_WORKFLOW_PATH_SIZE];
	char reason[CP_GUI_WORKFLOW_REASON_SIZE];
	int device_index;
	int validated;
	int validation_status;
	size_t playlist_index;
};

void		cp_gui_workflow_request_clear(
		    struct cp_gui_workflow_request *);
int		cp_gui_workflow_request_format(
		    const struct cp_gui_workflow_request *, char *, size_t);
int		cp_gui_workflow_request_from_key(int, const char *,
		    const char *, const char *, size_t, size_t, int,
		    struct cp_gui_workflow_request *);
int		cp_gui_workflow_output_device_restart_needed(int,
		    const struct cp_gui_workflow_request *, int *, int *);
int		cp_gui_workflow_request_set_device(
		    struct cp_gui_workflow_request *, int);
int		cp_gui_workflow_request_from_audio_selector(
		    const struct cp_selector *,
		    struct cp_gui_workflow_request *);
int		cp_gui_workflow_request_from_playlist_selector(
		    const struct cp_selector *,
		    struct cp_gui_workflow_request *);
int		cp_gui_workflow_request_set_path(
		    struct cp_gui_workflow_request *,
		    enum cp_gui_workflow_request_type, const char *);
int		cp_gui_workflow_request_set_playlist_item(
		    struct cp_gui_workflow_request *, const char *, size_t);
int		cp_gui_workflow_request_validate(
		    struct cp_gui_workflow_request *);
int		cp_gui_workflow_request_validate_device(
		    struct cp_gui_workflow_request *);
const char	*cp_gui_workflow_request_type_string(
		    enum cp_gui_workflow_request_type);

#endif

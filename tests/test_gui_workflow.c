/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_gui_workflow.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_gui_workflow.h"

static int	test_clear_request(void);
static int	test_device_request(void);
static int	test_forbidden_text(void);
static int	test_format_bounds(void);
static int	test_path_request(void);
static int	test_playlist_item_request(void);
static int	test_type_strings(void);

int
main(void)
{
	if (!test_clear_request())
		return 1;
	if (!test_type_strings())
		return 1;
	if (!test_path_request())
		return 1;
	if (!test_playlist_item_request())
		return 1;
	if (!test_device_request())
		return 1;
	if (!test_format_bounds())
		return 1;
	if (!test_forbidden_text())
		return 1;

	return 0;
}

static int
test_clear_request(void)
{
	struct cp_gui_workflow_request request;
	char buffer[64];

	request.type = CP_GUI_WORKFLOW_REQUEST_LOAD_WAV;
	request.device_index = 12;
	request.playlist_index = 4;
	(void)snprintf(request.path, sizeof(request.path), "%s",
	    "audio/program.wav");
	cp_gui_workflow_request_clear(&request);
	if (request.type != CP_GUI_WORKFLOW_REQUEST_NONE ||
	    request.device_index != -1 || request.playlist_index != 0 ||
	    request.path[0] != '\0') {
		printf("test_gui_workflow: clear mismatch\n");
		return 0;
	}
	if (cp_gui_workflow_request_format(&request, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strcmp(buffer, "workflow=none") != 0) {
		printf("test_gui_workflow: none format mismatch: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_device_request(void)
{
	struct cp_gui_workflow_request request;
	char buffer[96];

	if (cp_gui_workflow_request_set_device(NULL, 1) != CP_ERR_NULL)
		return 0;
	if (cp_gui_workflow_request_set_device(&request, -1) !=
	    CP_ERR_RANGE)
		return 0;
	if (cp_gui_workflow_request_set_device(&request, 3) != CP_OK)
		return 0;
	if (request.type != CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE ||
	    request.device_index != 3)
		return 0;
	if (cp_gui_workflow_request_format(&request, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strcmp(buffer, "workflow=select_output_device device=3") != 0) {
		printf("test_gui_workflow: device format mismatch: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_forbidden_text(void)
{
	struct cp_gui_workflow_request request;
	char buffer[128];

	if (cp_gui_workflow_request_set_path(&request,
	    CP_GUI_WORKFLOW_REQUEST_LOAD_WAV, "audio/program.wav") != CP_OK ||
	    cp_gui_workflow_request_format(&request, buffer,
	    sizeof(buffer)) != CP_OK)
		return 0;
	if (strstr(buffer, "ptt") != NULL ||
	    strstr(buffer, "transmit") != NULL ||
	    strstr(buffer, "cat_ptt") != NULL ||
	    strstr(buffer, "rig_frequency") != NULL ||
	    strstr(buffer, "rig_mode") != NULL ||
	    strstr(buffer, "hamlib") != NULL ||
	    strstr(buffer, "flrig") != NULL) {
		printf("test_gui_workflow: forbidden text: %s\n", buffer);
		return 0;
	}

	return 1;
}

static int
test_format_bounds(void)
{
	struct cp_gui_workflow_request request;
	char buffer[18];

	if (cp_gui_workflow_request_set_path(&request,
	    CP_GUI_WORKFLOW_REQUEST_LOAD_WAV, "audio/program.wav") != CP_OK)
		return 0;
	if (cp_gui_workflow_request_format(&request, buffer,
	    sizeof(buffer)) != CP_ERR_RANGE) {
		printf("test_gui_workflow: bounded format did not report range\n");
		return 0;
	}
	if (buffer[sizeof(buffer) - 1] != '\0') {
		printf("test_gui_workflow: bounded format not terminated\n");
		return 0;
	}

	return 1;
}

static int
test_path_request(void)
{
	struct cp_gui_workflow_request request;
	char long_path[CP_GUI_WORKFLOW_PATH_SIZE + 1];
	char buffer[128];

	memset(long_path, 'a', sizeof(long_path) - 1);
	long_path[sizeof(long_path) - 1] = '\0';

	if (cp_gui_workflow_request_set_path(NULL,
	    CP_GUI_WORKFLOW_REQUEST_LOAD_WAV, "audio/program.wav") !=
	    CP_ERR_NULL)
		return 0;
	if (cp_gui_workflow_request_set_path(&request,
	    CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE,
	    "audio/program.wav") != CP_ERR_RANGE)
		return 0;
	if (cp_gui_workflow_request_set_path(&request,
	    CP_GUI_WORKFLOW_REQUEST_LOAD_WAV, "") != CP_ERR_RANGE)
		return 0;
	if (cp_gui_workflow_request_set_path(&request,
	    CP_GUI_WORKFLOW_REQUEST_LOAD_WAV, long_path) != CP_ERR_RANGE)
		return 0;
	if (cp_gui_workflow_request_set_path(&request,
	    CP_GUI_WORKFLOW_REQUEST_LOAD_WAV, "audio/program.wav") !=
	    CP_OK)
		return 0;
	if (request.type != CP_GUI_WORKFLOW_REQUEST_LOAD_WAV ||
	    strcmp(request.path, "audio/program.wav") != 0)
		return 0;
	if (cp_gui_workflow_request_format(&request, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strcmp(buffer, "workflow=load_wav path=audio/program.wav") != 0) {
		printf("test_gui_workflow: path format mismatch: %s\n",
		    buffer);
		return 0;
	}

	if (cp_gui_workflow_request_set_path(&request,
	    CP_GUI_WORKFLOW_REQUEST_LOAD_PLAYLIST,
	    "playlists/show.txt") != CP_OK ||
	    cp_gui_workflow_request_format(&request, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "workflow=load_playlist") == NULL) {
		printf("test_gui_workflow: playlist format mismatch: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_playlist_item_request(void)
{
	struct cp_gui_workflow_request request;
	char buffer[128];

	if (cp_gui_workflow_request_set_playlist_item(NULL,
	    "audio/program.wav", 2) != CP_ERR_NULL)
		return 0;
	if (cp_gui_workflow_request_set_playlist_item(&request, "", 2) !=
	    CP_ERR_RANGE)
		return 0;
	if (cp_gui_workflow_request_set_playlist_item(&request,
	    "audio/program.wav", 2) != CP_OK)
		return 0;
	if (request.type != CP_GUI_WORKFLOW_REQUEST_CUE_PLAYLIST_ITEM ||
	    request.playlist_index != 2)
		return 0;
	if (cp_gui_workflow_request_format(&request, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strcmp(buffer,
	    "workflow=cue_playlist_item index=2 path=audio/program.wav") !=
	    0) {
		printf("test_gui_workflow: cue format mismatch: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_type_strings(void)
{
	if (strcmp(cp_gui_workflow_request_type_string(
	    CP_GUI_WORKFLOW_REQUEST_NONE), "none") != 0)
		return 0;
	if (strcmp(cp_gui_workflow_request_type_string(
	    CP_GUI_WORKFLOW_REQUEST_LOAD_WAV), "load_wav") != 0)
		return 0;
	if (strcmp(cp_gui_workflow_request_type_string(
	    CP_GUI_WORKFLOW_REQUEST_LOAD_PLAYLIST), "load_playlist") != 0)
		return 0;
	if (strcmp(cp_gui_workflow_request_type_string(
	    CP_GUI_WORKFLOW_REQUEST_CUE_PLAYLIST_ITEM),
	    "cue_playlist_item") != 0)
		return 0;
	if (strcmp(cp_gui_workflow_request_type_string(
	    CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE),
	    "select_output_device") != 0)
		return 0;
	if (strcmp(cp_gui_workflow_request_type_string(
	    (enum cp_gui_workflow_request_type)99), "unknown") != 0)
		return 0;

	return 1;
}

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_gui_workflow.c */

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>

#include "cp_gui_workflow.h"

#define TEST_GUI_WORKFLOW_DIR	"build/tests"
#define TEST_GUI_WORKFLOW_GOOD	"build/tests/gui-workflow-good.txt"
#define TEST_GUI_WORKFLOW_BAD	"build/tests/gui-workflow-bad.txt"

static int	test_clear_request(void);
static int	test_device_request(void);
static int	test_forbidden_text(void);
static int	test_format_bounds(void);
static int	test_key_mapping(void);
static int	test_path_request(void);
static int	test_rejected_request_display(void);
static int	test_playlist_item_request(void);
static int	test_playlist_validation(void);
static int	test_request_display_bounds(void);
static int	test_restart_decision(void);
static int	test_type_strings(void);
static int	test_valid_request_display(void);
static int	test_wav_validation(void);
static int	write_file(const char *, const char *);

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
	if (!test_request_display_bounds())
		return 1;
	if (!test_wav_validation())
		return 1;
	if (!test_playlist_validation())
		return 1;
	if (!test_valid_request_display())
		return 1;
	if (!test_rejected_request_display())
		return 1;
	if (!test_key_mapping())
		return 1;
	if (!test_restart_decision())
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
	if (cp_gui_workflow_request_set_device(&request, -2) !=
	    CP_ERR_RANGE)
		return 0;
	if (cp_gui_workflow_request_set_device(&request, -1) != CP_OK ||
	    request.type != CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE ||
	    request.device_index != -1)
		return 0;
	if (cp_gui_workflow_request_set_device(&request, 3) != CP_OK)
		return 0;
	if (request.type != CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE ||
	    request.device_index != 3)
		return 0;
	if (cp_gui_workflow_request_validate_device(&request) != CP_OK ||
	    request.validation_status != CP_OK ||
	    strstr(request.reason, "deferred output device request") == NULL)
		return 0;
	request.type = CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE;
	request.device_index = -2;
	if (cp_gui_workflow_request_validate_device(&request) !=
	    CP_ERR_RANGE ||
	    request.validation_status != CP_ERR_RANGE ||
	    strstr(request.reason, "invalid output device") == NULL)
		return 0;
	if (cp_gui_workflow_request_set_device(&request, 3) != CP_OK)
		return 0;
	if (cp_gui_workflow_request_format(&request, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strcmp(buffer,
	    "workflow=select_output_device status=pending device=3") != 0) {
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
test_key_mapping(void)
{
	struct cp_gui_workflow_request request;

	if (cp_gui_workflow_request_from_key('l', "audio/program.wav",
	    "playlist.txt", "audio/current.wav", 0, 1, 3, &request) !=
	    CP_OK ||
	    request.type != CP_GUI_WORKFLOW_REQUEST_LOAD_WAV ||
	    strcmp(request.path, "audio/program.wav") != 0) {
		printf("test_gui_workflow: WAV key mapping failed\n");
		return 0;
	}
	if (cp_gui_workflow_request_from_key('p', "audio/program.wav",
	    "playlist.txt", "audio/current.wav", 0, 1, 3, &request) !=
	    CP_OK ||
	    request.type != CP_GUI_WORKFLOW_REQUEST_LOAD_PLAYLIST ||
	    strcmp(request.path, "playlist.txt") != 0) {
		printf("test_gui_workflow: playlist key mapping failed\n");
		return 0;
	}
	if (cp_gui_workflow_request_from_key('c', "audio/program.wav",
	    "playlist.txt", "audio/current.wav", 2, 5, 3, &request) !=
	    CP_OK ||
	    request.type != CP_GUI_WORKFLOW_REQUEST_CUE_PLAYLIST_ITEM ||
	    request.playlist_index != 2 ||
	    strcmp(request.path, "audio/current.wav") != 0) {
		printf("test_gui_workflow: cue key mapping failed\n");
		return 0;
	}
	if (cp_gui_workflow_request_from_key('o', "audio/program.wav",
	    "playlist.txt", "audio/current.wav", 2, 5, -1, &request) !=
	    CP_OK ||
	    request.type != CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE ||
	    request.device_index != 0) {
		printf("test_gui_workflow: output next key mapping failed\n");
		return 0;
	}
	if (cp_gui_workflow_request_from_key('O', "audio/program.wav",
	    "playlist.txt", "audio/current.wav", 2, 5, 0, &request) !=
	    CP_OK ||
	    request.type != CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE ||
	    request.device_index != -1) {
		printf("test_gui_workflow: output prev key mapping failed\n");
		return 0;
	}
	if (cp_gui_workflow_request_from_key('t', "audio/program.wav",
	    "playlist.txt", "audio/current.wav", 0, 1, 3, &request) !=
	    CP_ERR_RANGE ||
	    request.type != CP_GUI_WORKFLOW_REQUEST_NONE) {
		printf("test_gui_workflow: forbidden key mapped request\n");
		return 0;
	}

	return 1;
}

static int
test_restart_decision(void)
{
	struct cp_gui_workflow_request request;
	int requested_device;
	int restart_needed;

	requested_device = -99;
	restart_needed = -1;
	if (cp_gui_workflow_output_device_restart_needed(2, NULL,
	    &restart_needed, &requested_device) != CP_ERR_NULL)
		return 0;
	if (cp_gui_workflow_request_set_device(&request, 3) != CP_OK)
		return 0;
	if (cp_gui_workflow_output_device_restart_needed(2, &request,
	    &restart_needed, &requested_device) != CP_OK ||
	    restart_needed != 1 || requested_device != 3) {
		printf("test_gui_workflow: restart decision failed\n");
		return 0;
	}
	if (cp_gui_workflow_output_device_restart_needed(3, &request,
	    &restart_needed, &requested_device) != CP_OK ||
	    restart_needed != 0 || requested_device != 3) {
		printf("test_gui_workflow: no-op restart decision failed\n");
		return 0;
	}
	if (cp_gui_workflow_request_set_device(&request, -1) != CP_OK ||
	    cp_gui_workflow_output_device_restart_needed(-1, &request,
	    &restart_needed, &requested_device) != CP_OK ||
	    restart_needed != 0 || requested_device != -1) {
		printf("test_gui_workflow: default restart decision failed\n");
		return 0;
	}
	request.type = CP_GUI_WORKFLOW_REQUEST_LOAD_WAV;
	if (cp_gui_workflow_output_device_restart_needed(2, &request,
	    &restart_needed, &requested_device) != CP_ERR_RANGE)
		return 0;
	request.type = CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE;
	request.device_index = -2;
	if (cp_gui_workflow_output_device_restart_needed(2, &request,
	    &restart_needed, &requested_device) != CP_ERR_RANGE)
		return 0;

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
test_request_display_bounds(void)
{
	struct cp_gui_workflow_request request;
	char buffer[256];
	char path[CP_GUI_WORKFLOW_PATH_SIZE];
	size_t index;

	for (index = 0; index < sizeof(path) - 5; index++)
		path[index] = 'a';
	(void)snprintf(path + index, sizeof(path) - index, ".wav");
	if (cp_gui_workflow_request_set_path(&request,
	    CP_GUI_WORKFLOW_REQUEST_LOAD_WAV, path) != CP_OK)
		return 0;
	if (cp_gui_workflow_request_format(&request, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "...") == NULL ||
	    strlen(buffer) >= sizeof(buffer)) {
		printf("test_gui_workflow: long pending display: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_playlist_validation(void)
{
	struct cp_gui_workflow_request request;
	char buffer[256];

	(void)mkdir("build", 0777);
	(void)mkdir(TEST_GUI_WORKFLOW_DIR, 0777);
	if (!write_file(TEST_GUI_WORKFLOW_GOOD,
	    "# show\n"
	    "audio/intro.wav\n"
	    "\n"
	    "audio/outro.WAV\n"))
		return 0;
	if (!write_file(TEST_GUI_WORKFLOW_BAD, "audio/music.mp3\n"))
		return 0;

	if (cp_gui_workflow_request_set_path(&request,
	    CP_GUI_WORKFLOW_REQUEST_LOAD_PLAYLIST,
	    TEST_GUI_WORKFLOW_GOOD) != CP_OK ||
	    cp_gui_workflow_request_validate(&request) != CP_OK ||
	    request.validation_status != CP_OK ||
	    strcmp(request.reason, "ok") != 0) {
		printf("test_gui_workflow: good playlist validation failed\n");
		return 0;
	}
	if (cp_gui_workflow_request_set_path(&request,
	    CP_GUI_WORKFLOW_REQUEST_LOAD_PLAYLIST,
	    TEST_GUI_WORKFLOW_BAD) != CP_OK ||
	    cp_gui_workflow_request_validate(&request) != CP_ERR_RANGE ||
	    request.validation_status != CP_ERR_RANGE ||
	    strstr(request.reason, "playlist") == NULL) {
		printf("test_gui_workflow: bad playlist validation failed\n");
		return 0;
	}
	if (cp_gui_workflow_request_format(&request, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "status=error") == NULL ||
	    strstr(buffer, "reason=playlist has errors") == NULL ||
	    strstr(buffer, "workflow=load_playlist") == NULL) {
		printf("test_gui_workflow: playlist status format failed: %s\n",
		    buffer);
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
	    strcmp(buffer,
	    "workflow=load_wav status=pending path=audio/program.wav") !=
	    0) {
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
test_wav_validation(void)
{
	static const char *bad_paths[] = {
		"audio/music.mp3",
		"audio/music.flac",
		"audio/music.ogg",
		"audio/music.opus",
		"audio/music.m4a"
	};
	struct cp_gui_workflow_request request;
	size_t index;

	if (cp_gui_workflow_request_set_path(&request,
	    CP_GUI_WORKFLOW_REQUEST_LOAD_WAV, "audio/program.wav") != CP_OK ||
	    cp_gui_workflow_request_validate(&request) != CP_OK ||
	    request.validation_status != CP_OK ||
	    strcmp(request.reason, "ok") != 0) {
		printf("test_gui_workflow: WAV validation failed\n");
		return 0;
	}
	if (cp_gui_workflow_request_set_path(&request,
	    CP_GUI_WORKFLOW_REQUEST_LOAD_WAV, "audio/program.WAV") != CP_OK ||
	    cp_gui_workflow_request_validate(&request) != CP_OK) {
		printf("test_gui_workflow: uppercase WAV validation failed\n");
		return 0;
	}
	for (index = 0; index < sizeof(bad_paths) / sizeof(bad_paths[0]);
	    index++) {
		if (cp_gui_workflow_request_set_path(&request,
		    CP_GUI_WORKFLOW_REQUEST_LOAD_WAV,
		    bad_paths[index]) != CP_OK ||
		    cp_gui_workflow_request_validate(&request) !=
		    CP_ERR_RANGE ||
		    strstr(request.reason, "convert to WAV first") == NULL) {
			printf("test_gui_workflow: bad WAV validation: %s\n",
			    bad_paths[index]);
			return 0;
		}
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
	    "workflow=cue_playlist_item status=pending index=2 "
	    "path=audio/program.wav") != 0) {
		printf("test_gui_workflow: cue format mismatch: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_rejected_request_display(void)
{
	struct cp_gui_workflow_request request;
	char buffer[192];

	if (cp_gui_workflow_request_set_path(&request,
	    CP_GUI_WORKFLOW_REQUEST_LOAD_WAV, "audio/music.mp3") != CP_OK ||
	    cp_gui_workflow_request_validate(&request) != CP_ERR_RANGE ||
	    cp_gui_workflow_request_format(&request, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "workflow=load_wav") == NULL ||
	    strstr(buffer, "status=error") == NULL ||
	    strstr(buffer, "reason=unsupported format") == NULL ||
	    strstr(buffer, "path=audio/music.mp3") == NULL) {
		printf("test_gui_workflow: rejected display mismatch: %s\n",
		    buffer);
		return 0;
	}

	(void)memset(request.reason, 'r', sizeof(request.reason) - 1);
	request.reason[sizeof(request.reason) - 1] = '\0';
	if (cp_gui_workflow_request_format(&request, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "...") == NULL ||
	    strlen(buffer) >= sizeof(buffer)) {
		printf("test_gui_workflow: rejected long reason: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_valid_request_display(void)
{
	struct cp_gui_workflow_request request;
	char buffer[160];

	if (cp_gui_workflow_request_set_path(&request,
	    CP_GUI_WORKFLOW_REQUEST_LOAD_WAV, "audio/program.wav") != CP_OK ||
	    cp_gui_workflow_request_validate(&request) != CP_OK ||
	    cp_gui_workflow_request_format(&request, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "workflow=load_wav") == NULL ||
	    strstr(buffer, "status=valid") == NULL ||
	    strstr(buffer, "reason=ok") == NULL ||
	    strstr(buffer, "path=audio/program.wav") == NULL) {
		printf("test_gui_workflow: valid display mismatch: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
write_file(const char *path, const char *text)
{
	FILE *file;

	file = fopen(path, "w");
	if (file == NULL) {
		printf("test_gui_workflow: could not open %s\n", path);
		return 0;
	}
	if (fputs(text, file) == EOF) {
		(void)fclose(file);
		printf("test_gui_workflow: could not write %s\n", path);
		return 0;
	}
	if (fclose(file) != 0) {
		printf("test_gui_workflow: could not close %s\n", path);
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

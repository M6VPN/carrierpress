/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_no_transmit_controls.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_control.h"
#include "cp_gui_workflow.h"

static int	text_has_forbidden_control(const char *);
static int	test_control_commands_are_audio_only(void);
static int	test_gui_workflow_requests_are_non_transmit(void);
static int	test_no_transmit_key_mapping(void);

int
main(void)
{
	if (!test_control_commands_are_audio_only())
		return 1;
	if (!test_gui_workflow_requests_are_non_transmit())
		return 1;
	if (!test_no_transmit_key_mapping())
		return 1;

	return 0;
}

static int
text_has_forbidden_control(const char *text)
{
	if (text == NULL)
		return 1;

	return strstr(text, "ptt") != NULL ||
	    strstr(text, "transmit") != NULL ||
	    strstr(text, "cat_ptt") != NULL ||
	    strstr(text, "rig_frequency") != NULL ||
	    strstr(text, "rig_mode") != NULL ||
	    strstr(text, "hamlib") != NULL ||
	    strstr(text, "flrig") != NULL;
}

static int
test_control_commands_are_audio_only(void)
{
	enum cp_control_command_type commands[] = {
		CP_CONTROL_COMMAND_NONE,
		CP_CONTROL_COMMAND_STOP,
		CP_CONTROL_COMMAND_PLAYOUT_NEXT,
		CP_CONTROL_COMMAND_DEHUMMER_TOGGLE,
		CP_CONTROL_COMMAND_MULTIBAND_CYCLE,
		CP_CONTROL_COMMAND_MULTIBAND2_CYCLE,
		CP_CONTROL_COMMAND_SELECT_AM,
		CP_CONTROL_COMMAND_SELECT_SSB,
		CP_CONTROL_COMMAND_AM_OFF,
		CP_CONTROL_COMMAND_AM_PRESET,
		CP_CONTROL_COMMAND_SSB_OFF,
		CP_CONTROL_COMMAND_SSB_PRESET
	};
	size_t index;
	const char *text;

	for (index = 0; index < sizeof(commands) / sizeof(commands[0]);
	    index++) {
		text = cp_control_command_string(commands[index]);
		if (text_has_forbidden_control(text)) {
			printf("test_no_transmit_controls: control text %s\n",
			    text);
			return 0;
		}
	}

	return 1;
}

static int
test_gui_workflow_requests_are_non_transmit(void)
{
	enum cp_gui_workflow_request_type requests[] = {
		CP_GUI_WORKFLOW_REQUEST_NONE,
		CP_GUI_WORKFLOW_REQUEST_LOAD_WAV,
		CP_GUI_WORKFLOW_REQUEST_LOAD_PLAYLIST,
		CP_GUI_WORKFLOW_REQUEST_CUE_PLAYLIST_ITEM,
		CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE
	};
	size_t index;
	const char *text;

	for (index = 0; index < sizeof(requests) / sizeof(requests[0]);
	    index++) {
		text = cp_gui_workflow_request_type_string(requests[index]);
		if (text_has_forbidden_control(text)) {
			printf("test_no_transmit_controls: workflow text %s\n",
			    text);
			return 0;
		}
	}

	return 1;
}

static int
test_no_transmit_key_mapping(void)
{
	struct cp_control_command command;
	struct cp_gui_workflow_request request;

	if (cp_control_command_from_key('t', CP_CONTROL_BANK_AM,
	    &command) != CP_ERR_RANGE) {
		printf("test_no_transmit_controls: control t accepted\n");
		return 0;
	}
	if (cp_control_command_from_key('T', CP_CONTROL_BANK_AM,
	    &command) != CP_ERR_RANGE) {
		printf("test_no_transmit_controls: control T accepted\n");
		return 0;
	}
	if (cp_control_command_from_key('r', CP_CONTROL_BANK_AM,
	    &command) != CP_ERR_RANGE) {
		printf("test_no_transmit_controls: control r accepted\n");
		return 0;
	}
	if (cp_control_command_from_key('u', CP_CONTROL_BANK_AM,
	    &command) != CP_ERR_RANGE) {
		printf("test_no_transmit_controls: control u accepted\n");
		return 0;
	}
	if (cp_control_command_from_key('E', CP_CONTROL_BANK_AM,
	    &command) != CP_ERR_RANGE) {
		printf("test_no_transmit_controls: control E accepted\n");
		return 0;
	}
	if (cp_gui_workflow_request_from_key('t', "audio/program.wav",
	    "playlist.txt", "audio/current.wav", 0, 1, 0,
	    &request) != CP_ERR_RANGE) {
		printf("test_no_transmit_controls: workflow t accepted\n");
		return 0;
	}
	if (cp_gui_workflow_request_from_key('T', "audio/program.wav",
	    "playlist.txt", "audio/current.wav", 0, 1, 0,
	    &request) != CP_ERR_RANGE) {
		printf("test_no_transmit_controls: workflow T accepted\n");
		return 0;
	}
	if (cp_gui_workflow_request_from_key('r', "audio/program.wav",
	    "playlist.txt", "audio/current.wav", 0, 1, 0,
	    &request) != CP_ERR_RANGE) {
		printf("test_no_transmit_controls: workflow r accepted\n");
		return 0;
	}
	if (cp_gui_workflow_request_from_key('u', "audio/program.wav",
	    "playlist.txt", "audio/current.wav", 0, 1, 0,
	    &request) != CP_ERR_RANGE) {
		printf("test_no_transmit_controls: workflow u accepted\n");
		return 0;
	}

	return 1;
}

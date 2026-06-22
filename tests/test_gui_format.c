/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_gui_format.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_audio.h"
#include "cp_cat.h"
#include "cp_gui_format.h"
#include "cp_monitor.h"
#include "cp_operator_state.h"

static int	test_cat_format(void);
static int	test_chain_format(void);
static int	test_control_mapping(void);
static int	test_cue_slots_format(void);
static int	test_audio_choices_format(void);
static int	test_help_format(void);
static int	test_meter_format(void);
static int	test_mode_format(void);
static int	test_operator_format(void);
static int	test_output_choices_format(void);
static int	test_output_device_format(void);
static int	test_playlist_choices_format(void);
static int	test_truncate_format(void);
static int	test_transport_format(void);
static int	test_workflow_format(void);

int
main(void)
{
	if (!test_meter_format())
		return 1;
	if (!test_mode_format())
		return 1;
	if (!test_chain_format())
		return 1;
	if (!test_control_mapping())
		return 1;
	if (!test_cue_slots_format())
		return 1;
	if (!test_audio_choices_format())
		return 1;
	if (!test_cat_format())
		return 1;
	if (!test_help_format())
		return 1;
	if (!test_transport_format())
		return 1;
	if (!test_operator_format())
		return 1;
	if (!test_output_choices_format())
		return 1;
	if (!test_output_device_format())
		return 1;
	if (!test_playlist_choices_format())
		return 1;
	if (!test_truncate_format())
		return 1;
	if (!test_workflow_format())
		return 1;

	return 0;
}

static int
test_audio_choices_format(void)
{
	const char *paths[] = {
		"audio/intro.wav",
		"audio/music.mp3",
		"audio/news-bed.WAV",
		"audio/readme.txt"
	};
	char buffer[256];
	char long_path[320];

	if (cp_gui_format_audio_choices(NULL, 0, NULL, NULL, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strcmp(buffer, "audio selector: no candidates") != 0) {
		printf("test_gui_format: audio no candidates mismatch: %s\n",
		    buffer);
		return 0;
	}
	if (cp_gui_format_audio_choices(paths, 4, "audio/intro.wav",
	    "audio/news-bed.WAV", buffer, sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "selector=audio_file") == NULL ||
	    strstr(buffer, "news-bed.WAV") == NULL ||
	    strstr(buffer, "requested") == NULL ||
	    strstr(buffer, "convert externally") == NULL ||
	    strstr(buffer, "disabled") == NULL) {
		printf("test_gui_format: audio choices mismatch: %s\n",
		    buffer);
		return 0;
	}
	memset(long_path, 'a', sizeof(long_path) - 5);
	(void)snprintf(long_path + sizeof(long_path) - 5, 5, ".wav");
	paths[0] = long_path;
	if (cp_gui_format_audio_choices(paths, 1, long_path, NULL, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strlen(buffer) >= sizeof(buffer) ||
	    strstr(buffer, "...") == NULL) {
		printf("test_gui_format: long audio choices mismatch: %s\n",
		    buffer);
		return 0;
	}
	if (strstr(buffer, "ptt") != NULL ||
	    strstr(buffer, "transmit") != NULL ||
	    strstr(buffer, "cat_ptt") != NULL ||
	    strstr(buffer, "rig_frequency") != NULL ||
	    strstr(buffer, "rig_mode") != NULL ||
	    strstr(buffer, "hamlib") != NULL ||
	    strstr(buffer, "flrig") != NULL) {
		printf("test_gui_format: audio forbidden text: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_cat_format(void)
{
	struct cp_cat_config config;
	struct cp_cat_snapshot snapshot;
	char buffer[256];

	if (cp_gui_format_cat(NULL, buffer, sizeof(buffer)) != CP_OK ||
	    strcmp(buffer, "CAT disabled") != 0) {
		printf("test_gui_format: disabled CAT mismatch: %s\n",
		    buffer);
		return 0;
	}

	cp_cat_default_config(&config);
	config.backend = CP_CAT_BACKEND_MOCK;
	config.enabled = 1;
	config.mock_frequency_hz = 14230000ULL;
	config.mock_ptt = CP_CAT_PTT_OFF;
	if (cp_cat_mode_set(config.mock_mode, sizeof(config.mock_mode),
	    "USB") != CP_OK ||
	    cp_cat_snapshot_update(&config, &snapshot) != CP_OK ||
	    cp_gui_format_cat(&snapshot, buffer, sizeof(buffer)) != CP_OK) {
		printf("test_gui_format: mock CAT setup failed\n");
		return 0;
	}
	if (strstr(buffer, "CAT mock ok") == NULL ||
	    strstr(buffer, "14230000 Hz") == NULL ||
	    strstr(buffer, "mode=USB") == NULL ||
	    strstr(buffer, "ptt=RX") == NULL) {
		printf("test_gui_format: mock CAT mismatch: %s\n", buffer);
		return 0;
	}

	config.mock_status = CP_CAT_STATUS_ERROR;
	if (cp_cat_snapshot_update(&config, &snapshot) != CP_OK ||
	    cp_gui_format_cat(&snapshot, buffer, sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "error") == NULL) {
		printf("test_gui_format: CAT error mismatch: %s\n", buffer);
		return 0;
	}

	return 1;
}

static int
test_chain_format(void)
{
	struct cp_monitor_snapshot snapshot;
	char buffer[512];

	cp_monitor_snapshot_clear(&snapshot);
	snapshot.dehummer_enabled = 1;
	snapshot.restoration_enabled = 1;
	snapshot.multiband_enabled = 1;
	snapshot.bass_eq_enabled = 1;
	snapshot.ssb_enabled = 1;

	if (cp_gui_format_chain(&snapshot, buffer, sizeof(buffer)) != CP_OK) {
		printf("test_gui_format: chain format failed\n");
		return 0;
	}
	if (strstr(buffer, "dehummer on") == NULL ||
	    strstr(buffer, "restoration on") == NULL ||
	    strstr(buffer, "MB1 on") == NULL ||
	    strstr(buffer, "AM off") == NULL ||
	    strstr(buffer, "SSB on") == NULL) {
		printf("test_gui_format: chain mismatch: %s\n", buffer);
		return 0;
	}

	return 1;
}

static int
test_control_mapping(void)
{
	struct cp_control_command command;

	if (cp_gui_control_command_from_key('d', CP_CONTROL_BANK_AM, 0,
	    &command) != CP_OK ||
	    command.type != CP_CONTROL_COMMAND_DEHUMMER_TOGGLE) {
		printf("test_gui_format: dehummer key mismatch\n");
		return 0;
	}
	if (cp_gui_control_command_from_key('1', CP_CONTROL_BANK_SSB, 0,
	    &command) != CP_OK ||
	    command.type != CP_CONTROL_COMMAND_SSB_PRESET ||
	    command.ssb_preset != CP_SSB_PRESET_SPEECH) {
		printf("test_gui_format: SSB preset key mismatch\n");
		return 0;
	}
	if (cp_gui_control_command_from_key('n', CP_CONTROL_BANK_AM, 0,
	    &command) != CP_OK ||
	    command.type != CP_CONTROL_COMMAND_NONE) {
		printf("test_gui_format: locked next key mismatch\n");
		return 0;
	}
	if (cp_gui_control_command_from_key('n', CP_CONTROL_BANK_AM, 1,
	    &command) != CP_OK ||
	    command.type != CP_CONTROL_COMMAND_PLAYOUT_NEXT) {
		printf("test_gui_format: next key mismatch\n");
		return 0;
	}

	return 1;
}

static int
test_cue_slots_format(void)
{
	char buffer[192];
	char long_path[160];
	size_t index;

	if (cp_gui_format_cue_slots("audio/program.wav",
	    "playlists/show.txt", buffer, sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "Cue: WAV audio/program.wav") == NULL ||
	    strstr(buffer, "Playlist playlists/show.txt") == NULL) {
		printf("test_gui_format: cue slots mismatch: %s\n", buffer);
		return 0;
	}
	if (cp_gui_format_cue_slots(NULL, "", buffer, sizeof(buffer)) !=
	    CP_OK ||
	    strcmp(buffer, "Cue: WAV - | Playlist -") != 0) {
		printf("test_gui_format: empty cue slots mismatch: %s\n",
		    buffer);
		return 0;
	}
	for (index = 0; index < sizeof(long_path) - 1; index++)
		long_path[index] = 'p';
	long_path[sizeof(long_path) - 1] = '\0';
	if (cp_gui_format_cue_slots(long_path, long_path, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "...") == NULL ||
	    strlen(buffer) >= sizeof(buffer)) {
		printf("test_gui_format: long cue slots mismatch: %s\n",
		    buffer);
		return 0;
	}
	if (strstr(buffer, "ptt") != NULL ||
	    strstr(buffer, "transmit") != NULL ||
	    strstr(buffer, "cat_ptt") != NULL ||
	    strstr(buffer, "rig_frequency") != NULL ||
	    strstr(buffer, "rig_mode") != NULL ||
	    strstr(buffer, "hamlib") != NULL ||
	    strstr(buffer, "flrig") != NULL) {
		printf("test_gui_format: cue forbidden text: %s\n", buffer);
		return 0;
	}

	return 1;
}

static int
test_meter_format(void)
{
	struct cp_monitor_snapshot snapshot;
	char buffer[256];

	cp_monitor_snapshot_clear(&snapshot);
	snapshot.input_peak = cp_monitor_sample_to_level(0.5f);
	snapshot.input_rms = cp_monitor_sample_to_level(0.25f);
	snapshot.output_peak = cp_monitor_sample_to_level(0.75f);
	snapshot.output_rms = cp_monitor_sample_to_level(0.125f);

	if (cp_gui_format_meters(&snapshot, buffer, sizeof(buffer)) !=
	    CP_OK) {
		printf("test_gui_format: meter format failed\n");
		return 0;
	}
	if (strstr(buffer, "in peak 0.500") == NULL ||
	    strstr(buffer, "rms 0.250") == NULL ||
	    strstr(buffer, "out peak 0.750") == NULL ||
	    strstr(buffer, "rms 0.125") == NULL) {
		printf("test_gui_format: meter mismatch: %s\n", buffer);
		return 0;
	}

	snapshot.stream_flags = CP_MONITOR_INPUT_OVERFLOW |
	    CP_MONITOR_OUTPUT_UNDERFLOW;
	if (cp_gui_format_flags(snapshot.stream_flags, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "in_overflow") == NULL ||
	    strstr(buffer, "out_underflow") == NULL) {
		printf("test_gui_format: flags mismatch: %s\n", buffer);
		return 0;
	}

	return 1;
}

static int
test_help_format(void)
{
	char buffer[256];

	if (cp_gui_format_help(CP_CONTROL_BANK_AM, 1, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "q/Esc stop") == NULL ||
	    strstr(buffer, "n next") == NULL ||
	    strstr(buffer, "m MB1") == NULL ||
	    strstr(buffer, "b MB2") == NULL ||
	    strstr(buffer, "l WAV") == NULL ||
	    strstr(buffer, "p playlist") == NULL ||
	    strstr(buffer, "c cue") == NULL ||
	    strstr(buffer, "o/O output") == NULL ||
	    strstr(buffer, "AM bank") == NULL) {
		printf("test_gui_format: AM help mismatch: %s\n", buffer);
		return 0;
	}
	if (cp_gui_format_help(CP_CONTROL_BANK_SSB, 0, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "n next locked") == NULL ||
	    strstr(buffer, "SSB bank") == NULL ||
	    strstr(buffer, "SSB presets") == NULL) {
		printf("test_gui_format: SSB help mismatch: %s\n", buffer);
		return 0;
	}
	if (strstr(buffer, "ptt") != NULL ||
	    strstr(buffer, "transmit") != NULL ||
	    strstr(buffer, "hamlib") != NULL ||
	    strstr(buffer, "flrig") != NULL) {
		printf("test_gui_format: help forbidden text: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_mode_format(void)
{
	struct cp_monitor_snapshot snapshot;
	char buffer[64];

	cp_monitor_snapshot_clear(&snapshot);
	if (cp_gui_format_mode(&snapshot, buffer, sizeof(buffer)) != CP_OK ||
	    strcmp(buffer, "Mode: NEUTRAL") != 0) {
		printf("test_gui_format: neutral mode mismatch: %s\n",
		    buffer);
		return 0;
	}

	snapshot.am_enabled = 1;
	if (cp_gui_format_mode(&snapshot, buffer, sizeof(buffer)) != CP_OK ||
	    strcmp(buffer, "Mode: AM") != 0) {
		printf("test_gui_format: AM mode mismatch: %s\n", buffer);
		return 0;
	}

	snapshot.am_enabled = 0;
	snapshot.ssb_enabled = 1;
	if (cp_gui_format_mode(&snapshot, buffer, sizeof(buffer)) != CP_OK ||
	    strcmp(buffer, "Mode: SSB") != 0) {
		printf("test_gui_format: SSB mode mismatch: %s\n", buffer);
		return 0;
	}

	return 1;
}

static int
test_operator_format(void)
{
	struct cp_operator_state state;
	char buffer[512];

	cp_operator_state_clear(&state);
	state.config_path = "configs/default.conf";
	state.profile_path = "profiles/ssb-speech.profile";
	state.profile_name = "SSB Speech";
	state.report_enabled = 1;
	state.report_path = "build/report.json";
	state.cue_path = "audio/program.wav";
	state.playlist_index = 2;
	state.playlist_count = 4;
	if (cp_gui_format_operator_state(&state, buffer,
	    sizeof(buffer)) != CP_OK) {
		printf("test_gui_format: operator format failed\n");
		return 0;
	}
	if (strstr(buffer, "Operator:") == NULL ||
	    strstr(buffer, "config=configs/default.conf") == NULL ||
	    strstr(buffer, "profile=profiles/ssb-speech.profile") == NULL ||
	    strstr(buffer, "cue=3/4") == NULL ||
	    strstr(buffer, "report=build/report.json") == NULL) {
		printf("test_gui_format: operator mismatch: %s\n", buffer);
		return 0;
	}
	if (strstr(buffer, "ptt") != NULL ||
	    strstr(buffer, "transmit") != NULL ||
	    strstr(buffer, "rig_frequency") != NULL ||
	    strstr(buffer, "rig_mode") != NULL ||
	    strstr(buffer, "hamlib") != NULL ||
	    strstr(buffer, "flrig") != NULL) {
		printf("test_gui_format: operator forbidden text: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_output_choices_format(void)
{
	struct cp_audio_device_candidate choices[5];
	char buffer[256];
	char long_name[120];
	size_t index;

	memset(choices, 0, sizeof(choices));
	if (cp_gui_format_output_choices(NULL, 0, 2, 0, 0, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strcmp(buffer, "outputs: enumeration unavailable") != 0) {
		printf("test_gui_format: no choices mismatch: %s\n",
		    buffer);
		return 0;
	}

	choices[0].index = 1;
	choices[0].name = "Built-in";
	choices[0].host_api = "pulse";
	choices[0].max_output_channels = 2;
	choices[0].default_output = 1;
	if (cp_gui_format_output_choices(choices, 1, 1, 0, 0, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "selector=output_device") == NULL ||
	    strstr(buffer, "Built-in") == NULL ||
	    strstr(buffer, "current") == NULL ||
	    strstr(buffer, "default") == NULL) {
		printf("test_gui_format: one choice mismatch: %s\n",
		    buffer);
		return 0;
	}

	choices[1].index = 2;
	choices[1].name = "USB Audio";
	choices[1].host_api = "alsa";
	choices[1].max_output_channels = 2;
	choices[2].index = 3;
	choices[2].name = "input only";
	choices[2].host_api = "alsa";
	choices[2].max_input_channels = 2;
	choices[3].index = 4;
	choices[3].name = "pulse";
	choices[3].host_api = "pulse";
	choices[3].max_output_channels = 2;
	if (cp_gui_format_output_choices(choices, 4, 2, 1, 4, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "selected=3/3") == NULL ||
	    strstr(buffer, "2 USB Audio") == NULL ||
	    strstr(buffer, "4 pulse") == NULL ||
	    strstr(buffer, "requested") == NULL ||
	    strstr(buffer, "input only") != NULL) {
		printf("test_gui_format: multiple choice mismatch: %s\n",
		    buffer);
		return 0;
	}

	for (index = 0; index < sizeof(long_name) - 1; index++)
		long_name[index] = 'd';
	long_name[sizeof(long_name) - 1] = '\0';
	choices[4].index = 5;
	choices[4].name = long_name;
	choices[4].host_api = "alsa";
	choices[4].max_output_channels = 2;
	if (cp_gui_format_output_choices(choices, 5, 2, 1, 5, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strlen(buffer) >= sizeof(buffer) ||
	    strstr(buffer, "...") == NULL) {
		printf("test_gui_format: long choice mismatch: %s\n",
		    buffer);
		return 0;
	}
	if (strstr(buffer, "ptt") != NULL ||
	    strstr(buffer, "transmit") != NULL ||
	    strstr(buffer, "cat_ptt") != NULL ||
	    strstr(buffer, "rig_frequency") != NULL ||
	    strstr(buffer, "rig_mode") != NULL ||
	    strstr(buffer, "hamlib") != NULL ||
	    strstr(buffer, "flrig") != NULL) {
		printf("test_gui_format: choices forbidden text: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_output_device_format(void)
{
	struct cp_audio_config config;
	struct cp_gui_workflow_request request;
	char buffer[256];

	cp_audio_default_config(&config);
	if (cp_gui_format_output_device(&config, CP_AUDIO_DEFAULT_DEVICE,
	    NULL, buffer, sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "output_device=current:-1 requested:-") == NULL ||
	    strstr(buffer, "backend=auto") == NULL) {
		printf("test_gui_format: default output mismatch: %s\n",
		    buffer);
		return 0;
	}
	config.device_name =
	    "very-long-device-name-for-gui-output-display-that-must-fit";
	if (cp_gui_workflow_request_set_device(&request, 4) != CP_OK ||
	    cp_gui_format_output_device(&config, 3, &request, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "output_device=current:3 requested:4") == NULL ||
	    strstr(buffer, "device=") == NULL) {
		printf("test_gui_format: requested output mismatch: %s\n",
		    buffer);
		return 0;
	}
	if (strstr(buffer, "ptt") != NULL ||
	    strstr(buffer, "transmit") != NULL ||
	    strstr(buffer, "cat_ptt") != NULL ||
	    strstr(buffer, "rig_frequency") != NULL ||
	    strstr(buffer, "rig_mode") != NULL) {
		printf("test_gui_format: output forbidden text: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_playlist_choices_format(void)
{
	const char *paths[] = {
		"playlists/show.txt",
		"audio/track.wav",
		"playlists/morning.playlist",
		"audio/music.mp3"
	};
	char buffer[256];
	char long_path[320];

	if (cp_gui_format_playlist_choices(NULL, 0, NULL, NULL, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strcmp(buffer, "playlist selector: no candidates") != 0) {
		printf("test_gui_format: playlist no candidates mismatch: "
		    "%s\n", buffer);
		return 0;
	}
	if (cp_gui_format_playlist_choices(paths, 4, "playlists/show.txt",
	    "playlists/morning.playlist", buffer, sizeof(buffer)) !=
	    CP_OK ||
	    strstr(buffer, "selector=playlist") == NULL ||
	    strstr(buffer, "morning.playlist") == NULL ||
	    strstr(buffer, "requested") == NULL ||
	    strstr(buffer, "not a playlist") == NULL ||
	    strstr(buffer, "disabled") == NULL) {
		printf("test_gui_format: playlist choices mismatch: %s\n",
		    buffer);
		return 0;
	}
	memset(long_path, 'p', sizeof(long_path) - 10);
	(void)snprintf(long_path + sizeof(long_path) - 10, 10,
	    ".playlist");
	paths[0] = long_path;
	if (cp_gui_format_playlist_choices(paths, 1, long_path, NULL,
	    buffer, sizeof(buffer)) != CP_OK ||
	    strlen(buffer) >= sizeof(buffer) ||
	    strstr(buffer, "...") == NULL) {
		printf("test_gui_format: long playlist choices mismatch: "
		    "%s\n", buffer);
		return 0;
	}
	if (strstr(buffer, "ptt") != NULL ||
	    strstr(buffer, "transmit") != NULL ||
	    strstr(buffer, "cat_ptt") != NULL ||
	    strstr(buffer, "rig_frequency") != NULL ||
	    strstr(buffer, "rig_mode") != NULL ||
	    strstr(buffer, "hamlib") != NULL ||
	    strstr(buffer, "flrig") != NULL) {
		printf("test_gui_format: playlist forbidden text: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_truncate_format(void)
{
	char buffer[16];

	if (cp_gui_format_truncate("short", buffer, sizeof(buffer), 12) !=
	    CP_OK || strcmp(buffer, "short") != 0) {
		printf("test_gui_format: short truncate mismatch: %s\n",
		    buffer);
		return 0;
	}
	if (cp_gui_format_truncate("abcdefghijklmnopqrstuvwxyz", buffer,
	    sizeof(buffer), 10) != CP_ERR_RANGE ||
	    strcmp(buffer, "abcdefg...") != 0) {
		printf("test_gui_format: long truncate mismatch: %s\n",
		    buffer);
		return 0;
	}
	if (cp_gui_format_truncate("abcdef", buffer, sizeof(buffer), 3) !=
	    CP_ERR_RANGE || strcmp(buffer, "abc") != 0) {
		printf("test_gui_format: small truncate mismatch: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_transport_format(void)
{
	struct cp_audio_config config;
	char buffer[256];

	cp_audio_default_config(&config);
	if (cp_gui_format_transport(CP_GUI_MODE_LIVE, &config, NULL, 0, 0,
	    CP_AUDIO_DEFAULT_DEVICE, buffer, sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "Transport LIVE") == NULL) {
		printf("test_gui_format: live transport mismatch: %s\n",
		    buffer);
		return 0;
	}

	if (cp_gui_format_transport(CP_GUI_MODE_PLAYOUT, &config,
	    "input.wav", 1, 3, 7, buffer, sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "Transport PLAYLIST 2/3") == NULL ||
	    strstr(buffer, "input.wav") == NULL ||
	    strstr(buffer, "out 7") == NULL) {
		printf("test_gui_format: playlist transport mismatch: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_workflow_format(void)
{
	struct cp_gui_workflow_request request;
	char buffer[160];

	cp_gui_workflow_request_clear(&request);
	if (cp_gui_format_workflow(&request, buffer, sizeof(buffer)) !=
	    CP_OK ||
	    strcmp(buffer, "workflow=none") != 0) {
		printf("test_gui_format: workflow none mismatch: %s\n",
		    buffer);
		return 0;
	}
	if (cp_gui_workflow_request_set_device(&request, 4) != CP_OK ||
	    cp_gui_format_workflow(&request, buffer, sizeof(buffer)) !=
	    CP_OK ||
	    strcmp(buffer,
	    "workflow=select_output_device status=pending device=4") != 0) {
		printf("test_gui_format: workflow device mismatch: %s\n",
		    buffer);
		return 0;
	}
	if (cp_gui_workflow_request_set_playlist_item(&request,
	    "audio/program.wav", 7) != CP_OK ||
	    cp_gui_format_workflow(&request, buffer, sizeof(buffer)) !=
	    CP_OK ||
	    strstr(buffer,
	    "workflow=cue_playlist_item status=pending index=7") == NULL ||
	    strstr(buffer, "audio/program.wav") == NULL) {
		printf("test_gui_format: workflow cue mismatch: %s\n",
		    buffer);
		return 0;
	}
	if (strstr(buffer, "ptt") != NULL ||
	    strstr(buffer, "transmit") != NULL ||
	    strstr(buffer, "cat_ptt") != NULL ||
	    strstr(buffer, "rig_frequency") != NULL ||
	    strstr(buffer, "rig_mode") != NULL ||
	    strstr(buffer, "hamlib") != NULL ||
	    strstr(buffer, "flrig") != NULL) {
		printf("test_gui_format: workflow forbidden text: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

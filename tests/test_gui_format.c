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
static int	test_meter_format(void);
static int	test_mode_format(void);
static int	test_operator_format(void);
static int	test_transport_format(void);

int
main(void)
{
	if (!test_meter_format())
		return 1;
	if (!test_mode_format())
		return 1;
	if (!test_chain_format())
		return 1;
	if (!test_cat_format())
		return 1;
	if (!test_transport_format())
		return 1;
	if (!test_operator_format())
		return 1;

	return 0;
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

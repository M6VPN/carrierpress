/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_tui.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_audio.h"
#include "cp_monitor.h"
#include "cp_tui.h"

static int	test_key_help(void);
static int	test_mode_state(void);

int
main(void)
{
	if (!test_mode_state())
		return 1;
	if (!test_key_help())
		return 1;

	return 0;
}

static int
test_key_help(void)
{
	struct cp_audio_config config;
	struct cp_monitor_snapshot snapshot;
	struct cp_tui_view view;
	char buffer[256];

	cp_audio_default_config(&config);
	cp_monitor_snapshot_clear(&snapshot);
	memset(&view, 0, sizeof(view));
	view.config = &config;
	view.snapshot = &snapshot;

	view.mode = CP_TUI_MODE_LIVE;
	view.next_enabled = 0;
	if (cp_tui_format_key_help(&view, CP_CONTROL_BANK_AM, buffer,
	    sizeof(buffer)) != CP_OK) {
		printf("test_tui: live key help failed\n");
		return 0;
	}
	if (strstr(buffer, "AM BANK") == NULL ||
	    strstr(buffer, "n next") != NULL ||
	    strstr(buffer, "mode NEUTRAL") == NULL) {
		printf("test_tui: live key help mismatch: %s\n", buffer);
		return 0;
	}

	view.mode = CP_TUI_MODE_PLAYOUT;
	view.next_enabled = 1;
	if (cp_tui_format_key_help(&view, CP_CONTROL_BANK_SSB, buffer,
	    sizeof(buffer)) != CP_OK) {
		printf("test_tui: playlist key help failed\n");
		return 0;
	}
	if (strstr(buffer, "SSB BANK") == NULL ||
	    strstr(buffer, "n next") == NULL ||
	    strstr(buffer, "0 SSB off") == NULL) {
		printf("test_tui: playlist key help mismatch: %s\n", buffer);
		return 0;
	}

	return 1;
}

static int
test_mode_state(void)
{
	struct cp_monitor_snapshot snapshot;
	char buffer[256];

	cp_monitor_snapshot_clear(&snapshot);
	if (strcmp(cp_tui_active_mode_string(&snapshot), "NEUTRAL") != 0) {
		printf("test_tui: neutral mode mismatch\n");
		return 0;
	}
	if (cp_tui_format_mode_status(&snapshot, CP_CONTROL_BANK_AM, buffer,
	    sizeof(buffer)) != CP_OK) {
		printf("test_tui: neutral status failed\n");
		return 0;
	}
	if (strstr(buffer, "Mode NEUTRAL") == NULL ||
	    strstr(buffer, "SSB controls locked by AM bank") == NULL) {
		printf("test_tui: neutral status mismatch: %s\n", buffer);
		return 0;
	}

	snapshot.am_enabled = 1;
	if (strcmp(cp_tui_active_mode_string(&snapshot), "AM") != 0) {
		printf("test_tui: AM mode mismatch\n");
		return 0;
	}
	if (cp_tui_format_mode_status(&snapshot, CP_CONTROL_BANK_AM, buffer,
	    sizeof(buffer)) != CP_OK) {
		printf("test_tui: AM status failed\n");
		return 0;
	}
	if (strstr(buffer, "AM controls active") == NULL ||
	    strstr(buffer, "SSB controls locked by AM mode") == NULL) {
		printf("test_tui: AM lock mismatch: %s\n", buffer);
		return 0;
	}

	snapshot.am_enabled = 0;
	snapshot.ssb_enabled = 1;
	if (strcmp(cp_tui_active_mode_string(&snapshot), "SSB") != 0) {
		printf("test_tui: SSB mode mismatch\n");
		return 0;
	}
	if (cp_tui_format_mode_status(&snapshot, CP_CONTROL_BANK_SSB, buffer,
	    sizeof(buffer)) != CP_OK) {
		printf("test_tui: SSB status failed\n");
		return 0;
	}
	if (strstr(buffer, "AM controls locked by SSB mode") == NULL ||
	    strstr(buffer, "SSB controls active") == NULL) {
		printf("test_tui: SSB lock mismatch: %s\n", buffer);
		return 0;
	}

	if (cp_tui_format_mode_status(&snapshot, CP_CONTROL_BANK_AM, buffer,
	    sizeof(buffer)) != CP_OK) {
		printf("test_tui: AM armed status failed\n");
		return 0;
	}
	if (strstr(buffer, "AM bank armed") == NULL) {
		printf("test_tui: armed status mismatch: %s\n", buffer);
		return 0;
	}

	return 1;
}

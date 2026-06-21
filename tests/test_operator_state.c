/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_operator_state.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_operator_state.h"

static int	has_forbidden_text(const char *);
static int	test_forbidden_text(void);
static int	test_missing_fields(void);
static int	test_playlist_cue(void);
static int	test_report_status(void);
static int	test_sources(void);
static int	test_truncation(void);

int
main(void)
{
	if (!test_sources())
		return 1;
	if (!test_missing_fields())
		return 1;
	if (!test_playlist_cue())
		return 1;
	if (!test_report_status())
		return 1;
	if (!test_truncation())
		return 1;
	if (!test_forbidden_text())
		return 1;

	return 0;
}

static int
has_forbidden_text(const char *text)
{
	if (text == NULL)
		return 0;

	return strstr(text, "ptt") != NULL ||
	    strstr(text, "transmit") != NULL ||
	    strstr(text, "rig_frequency") != NULL ||
	    strstr(text, "rig_mode") != NULL ||
	    strstr(text, "hamlib") != NULL ||
	    strstr(text, "flrig") != NULL ||
	    strstr(text, "cat_ptt") != NULL;
}

static int
test_forbidden_text(void)
{
	struct cp_operator_state state;
	char buffer[512];

	cp_operator_state_clear(&state);
	state.config_path = "configs/default.conf";
	state.profile_path = "profiles/am-safe.profile";
	state.profile_name = "AM Safe";
	state.report_enabled = 1;
	state.report_path = "build/report.json";
	state.cue_path = "audio/program.wav";
	state.playlist_index = 1;
	state.playlist_count = 3;

	if (cp_operator_state_format_summary(&state, buffer,
	    sizeof(buffer)) != CP_OK) {
		printf("test_operator_state: summary format failed\n");
		return 0;
	}
	if (has_forbidden_text(buffer)) {
		printf("test_operator_state: forbidden text in summary: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_missing_fields(void)
{
	struct cp_operator_state state;
	char buffer[128];

	cp_operator_state_clear(&state);
	if (cp_operator_state_format_sources(&state, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strcmp(buffer, "config=- profile=- name=\"-\"") != 0) {
		printf("test_operator_state: missing sources mismatch: %s\n",
		    buffer);
		return 0;
	}
	if (cp_operator_state_format_cue(&state, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strcmp(buffer, "cue=- path=-") != 0) {
		printf("test_operator_state: missing cue mismatch: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_playlist_cue(void)
{
	struct cp_operator_state state;
	char buffer[128];

	cp_operator_state_clear(&state);
	state.cue_path = "audio/program.wav";
	state.playlist_index = 1;
	state.playlist_count = 5;
	if (cp_operator_state_format_cue(&state, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strcmp(buffer, "cue=2/5 path=audio/program.wav") != 0) {
		printf("test_operator_state: playlist cue mismatch: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_report_status(void)
{
	struct cp_operator_state state;
	char buffer[128];

	cp_operator_state_clear(&state);
	state.report_enabled = 1;
	state.report_path = "output.report.json";
	state.batch_enabled = 1;
	state.batch_path = "examples/batch-list.txt";
	if (cp_operator_state_format_report(&state, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strcmp(buffer,
	    "report=output.report.json batch=examples/batch-list.txt") !=
	    0) {
		printf("test_operator_state: report status mismatch: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_sources(void)
{
	struct cp_operator_state state;
	char buffer[160];

	cp_operator_state_clear(&state);
	state.config_path = "configs/default.conf";
	state.profile_path = "profiles/am-safe.profile";
	state.profile_name = "AM Safe";
	if (cp_operator_state_format_sources(&state, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strcmp(buffer,
	    "config=configs/default.conf profile=profiles/am-safe.profile "
	    "name=\"AM Safe\"") != 0) {
		printf("test_operator_state: sources mismatch: %s\n",
		    buffer);
		return 0;
	}

	return 1;
}

static int
test_truncation(void)
{
	struct cp_operator_state state;
	char buffer[16];

	cp_operator_state_clear(&state);
	state.config_path = "configs/default.conf";
	state.profile_path = "profiles/am-safe.profile";
	state.profile_name = "AM Safe";
	if (cp_operator_state_format_sources(&state, buffer,
	    sizeof(buffer)) != CP_ERR_RANGE) {
		printf("test_operator_state: truncation not reported\n");
		return 0;
	}

	return 1;
}

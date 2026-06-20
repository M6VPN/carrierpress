/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_cat_hamlib.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include <hamlib/rig.h>
#include <hamlib/riglist.h>

#include "cp_cat_hamlib.h"

static int	test_config_defaults(void);
static int	test_dummy_snapshot(void);
static int	test_mode_mapping(void);
static int	test_ptt_mapping(void);

int
main(void)
{
	if (!test_config_defaults())
		return 1;
	if (!test_mode_mapping())
		return 1;
	if (!test_ptt_mapping())
		return 1;
	if (!test_dummy_snapshot())
		return 1;

	return 0;
}

static int
test_config_defaults(void)
{
	struct cp_cat_config config;
	char path[CP_CAT_PATH_TEXT + 1];

	cp_cat_default_config(&config);
	if (config.hamlib_rig_model != CP_CAT_HAMLIB_DEFAULT_RIG_MODEL ||
	    config.hamlib_rig_path[0] != '\0' ||
	    config.hamlib_rig_speed != CP_CAT_HAMLIB_DEFAULT_RIG_SPEED) {
		printf("test_cat_hamlib: default config mismatch\n");
		return 0;
	}
	if (cp_cat_config_set_hamlib_path(&config,
	    "/dev/ttyUSB0") != CP_OK ||
	    strcmp(config.hamlib_rig_path, "/dev/ttyUSB0") != 0) {
		printf("test_cat_hamlib: path set failed\n");
		return 0;
	}

	memset(path, 'a', sizeof(path));
	path[sizeof(path) - 1u] = '\0';
	if (cp_cat_config_set_hamlib_path(&config, path) == CP_OK) {
		printf("test_cat_hamlib: overlong path accepted\n");
		return 0;
	}

	config.backend = CP_CAT_BACKEND_HAMLIB;
	config.enabled = 1;
	config.hamlib_rig_speed = CP_CAT_MIN_RIG_SPEED - 1u;
	if (cp_cat_validate_config(&config) == CP_OK) {
		printf("test_cat_hamlib: invalid speed accepted\n");
		return 0;
	}

	return 1;
}

static int
test_dummy_snapshot(void)
{
	struct cp_cat_config config;
	struct cp_cat_snapshot snapshot;

	cp_cat_default_config(&config);
	config.backend = CP_CAT_BACKEND_HAMLIB;
	config.enabled = 1;
	if (cp_cat_snapshot_update(&config, &snapshot) != CP_OK ||
	    snapshot.status != CP_CAT_STATUS_UNAVAILABLE ||
	    snapshot.connected) {
		printf("test_cat_hamlib: missing model did not fail cleanly\n");
		return 0;
	}

	config.hamlib_rig_model = RIG_MODEL_DUMMY;
	if (cp_cat_snapshot_update(&config, &snapshot) != CP_OK) {
		printf("test_cat_hamlib: dummy snapshot update failed\n");
		return 0;
	}
	if (snapshot.status != CP_CAT_STATUS_OK || !snapshot.connected ||
	    snapshot.frequency_hz == 0 ||
	    strcmp(snapshot.mode, "FM") != 0) {
		printf("test_cat_hamlib: dummy snapshot mismatch\n");
		return 0;
	}

	return 1;
}

static int
test_mode_mapping(void)
{
	char mode[CP_CAT_MODE_TEXT];

	if (cp_cat_hamlib_mode_to_text((uint64_t)RIG_MODE_USB, mode,
	    sizeof(mode)) != CP_OK ||
	    strcmp(mode, "USB") != 0) {
		printf("test_cat_hamlib: USB mode mapping failed\n");
		return 0;
	}
	if (cp_cat_hamlib_mode_to_text((uint64_t)RIG_MODE_AM, mode,
	    sizeof(mode)) != CP_OK ||
	    strcmp(mode, "AM") != 0) {
		printf("test_cat_hamlib: AM mode mapping failed\n");
		return 0;
	}
	if (cp_cat_hamlib_mode_to_text((uint64_t)RIG_MODE_NONE, mode,
	    sizeof(mode)) == CP_OK) {
		printf("test_cat_hamlib: invalid mode accepted\n");
		return 0;
	}

	return 1;
}

static int
test_ptt_mapping(void)
{
	enum cp_cat_ptt_state ptt;

	if (cp_cat_hamlib_ptt_to_state((int)RIG_PTT_OFF, &ptt) != CP_OK ||
	    ptt != CP_CAT_PTT_OFF) {
		printf("test_cat_hamlib: PTT off mapping failed\n");
		return 0;
	}
	if (cp_cat_hamlib_ptt_to_state((int)RIG_PTT_ON, &ptt) != CP_OK ||
	    ptt != CP_CAT_PTT_ON) {
		printf("test_cat_hamlib: PTT on mapping failed\n");
		return 0;
	}
	if (cp_cat_hamlib_ptt_to_state(99, &ptt) == CP_OK ||
	    ptt != CP_CAT_PTT_UNKNOWN) {
		printf("test_cat_hamlib: invalid PTT mapping failed\n");
		return 0;
	}

	return 1;
}

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_cat.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_cat.h"

static int	test_backend_parse(void);
static int	test_frequency_format(void);
static int	test_flrig_unavailable_without_build(void);
static int	test_hamlib_unavailable_without_build(void);
static int	test_mode_parse(void);
static int	test_mock_snapshot(void);
static int	test_ptt_parse(void);
static int	test_status_variants(void);

int
main(void)
{
	if (!test_backend_parse())
		return 1;
	if (!test_frequency_format())
		return 1;
	if (!test_flrig_unavailable_without_build())
		return 1;
	if (!test_hamlib_unavailable_without_build())
		return 1;
	if (!test_mode_parse())
		return 1;
	if (!test_ptt_parse())
		return 1;
	if (!test_mock_snapshot())
		return 1;
	if (!test_status_variants())
		return 1;

	return 0;
}

static int
test_backend_parse(void)
{
	enum cp_cat_backend backend;

	if (cp_cat_backend_from_string("mock", &backend) != CP_OK ||
	    backend != CP_CAT_BACKEND_MOCK) {
		printf("test_cat: mock backend parse failed\n");
		return 0;
	}
	if (cp_cat_backend_from_string("FLRIG", &backend) != CP_OK ||
	    backend != CP_CAT_BACKEND_FLRIG) {
		printf("test_cat: flrig backend parse failed\n");
		return 0;
	}
	if (cp_cat_backend_from_string("hamlib", &backend) != CP_OK ||
	    backend != CP_CAT_BACKEND_HAMLIB) {
		printf("test_cat: hamlib backend parse failed\n");
		return 0;
	}
	if (cp_cat_backend_from_string("bad", &backend) == CP_OK) {
		printf("test_cat: invalid backend accepted\n");
		return 0;
	}

	return 1;
}

static int
test_flrig_unavailable_without_build(void)
{
#ifndef CP_WITH_FLRIG
	struct cp_cat_config config;
	struct cp_cat_snapshot snapshot;
	char buffer[128];

	cp_cat_default_config(&config);
	config.backend = CP_CAT_BACKEND_FLRIG;
	config.enabled = 1;
	if (cp_cat_snapshot_update(&config, &snapshot) != CP_OK) {
		printf("test_cat: flrig unavailable snapshot failed\n");
		return 0;
	}
	if (snapshot.status != CP_CAT_STATUS_UNAVAILABLE ||
	    snapshot.connected) {
		printf("test_cat: flrig unavailable mismatch\n");
		return 0;
	}
	if (cp_cat_snapshot_format(&snapshot, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "CAT flrig unavailable") == NULL) {
		printf("test_cat: flrig unavailable format mismatch: %s\n",
		    buffer);
		return 0;
	}
#endif

	return 1;
}

static int
test_hamlib_unavailable_without_build(void)
{
#ifndef CP_WITH_HAMLIB
	struct cp_cat_config config;
	struct cp_cat_snapshot snapshot;
	char buffer[128];

	cp_cat_default_config(&config);
	config.backend = CP_CAT_BACKEND_HAMLIB;
	config.enabled = 1;
	if (cp_cat_snapshot_update(&config, &snapshot) != CP_OK) {
		printf("test_cat: hamlib unavailable snapshot failed\n");
		return 0;
	}
	if (snapshot.status != CP_CAT_STATUS_UNAVAILABLE ||
	    snapshot.connected) {
		printf("test_cat: hamlib unavailable mismatch\n");
		return 0;
	}
	if (cp_cat_snapshot_format(&snapshot, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strstr(buffer, "CAT hamlib unavailable") == NULL) {
		printf("test_cat: hamlib unavailable format mismatch: %s\n",
		    buffer);
		return 0;
	}
#endif

	return 1;
}

static int
test_frequency_format(void)
{
	char buffer[32];

	if (cp_cat_format_frequency(14230000ULL, buffer,
	    sizeof(buffer)) != CP_OK) {
		printf("test_cat: frequency format failed\n");
		return 0;
	}
	if (strcmp(buffer, "14230000 Hz") != 0) {
		printf("test_cat: frequency mismatch: %s\n", buffer);
		return 0;
	}

	return 1;
}

static int
test_mode_parse(void)
{
	char mode[CP_CAT_MODE_TEXT];
	char long_mode[CP_CAT_MODE_TEXT + 1];

	if (cp_cat_mode_set(mode, sizeof(mode), "USB") != CP_OK ||
	    strcmp(mode, "USB") != 0) {
		printf("test_cat: mode parse failed\n");
		return 0;
	}
	memset(long_mode, 'A', sizeof(long_mode));
	long_mode[sizeof(long_mode) - 1] = '\0';
	if (cp_cat_mode_set(mode, sizeof(mode), long_mode) == CP_OK) {
		printf("test_cat: long mode accepted\n");
		return 0;
	}
	if (cp_cat_mode_set(mode, sizeof(mode), "USB!") == CP_OK) {
		printf("test_cat: invalid mode accepted\n");
		return 0;
	}

	return 1;
}

static int
test_mock_snapshot(void)
{
	struct cp_cat_config config;
	struct cp_cat_snapshot snapshot;
	char buffer[128];

	cp_cat_default_config(&config);
	config.backend = CP_CAT_BACKEND_MOCK;
	config.enabled = 1;
	config.mock_frequency_hz = 14230000ULL;
	config.mock_ptt = CP_CAT_PTT_OFF;
	if (cp_cat_mode_set(config.mock_mode, sizeof(config.mock_mode),
	    "USB") != CP_OK)
		return 0;

	if (cp_cat_snapshot_update(&config, &snapshot) != CP_OK) {
		printf("test_cat: mock snapshot failed\n");
		return 0;
	}
	if (!snapshot.enabled || !snapshot.connected ||
	    snapshot.frequency_hz != 14230000ULL ||
	    strcmp(snapshot.mode, "USB") != 0 ||
	    snapshot.ptt != CP_CAT_PTT_OFF) {
		printf("test_cat: mock snapshot mismatch\n");
		return 0;
	}
	if (cp_cat_snapshot_format(&snapshot, buffer,
	    sizeof(buffer)) != CP_OK) {
		printf("test_cat: mock format failed\n");
		return 0;
	}
	if (strstr(buffer, "CAT mock ok") == NULL ||
	    strstr(buffer, "14230000 Hz") == NULL ||
	    strstr(buffer, "mode=USB") == NULL ||
	    strstr(buffer, "ptt=RX") == NULL) {
		printf("test_cat: mock format mismatch: %s\n", buffer);
		return 0;
	}

	return 1;
}

static int
test_ptt_parse(void)
{
	enum cp_cat_ptt_state ptt;

	if (cp_cat_ptt_from_string("off", &ptt) != CP_OK ||
	    ptt != CP_CAT_PTT_OFF ||
	    strcmp(cp_cat_ptt_string(ptt), "RX") != 0) {
		printf("test_cat: ptt off parse failed\n");
		return 0;
	}
	if (cp_cat_ptt_from_string("TX", &ptt) != CP_OK ||
	    ptt != CP_CAT_PTT_ON ||
	    strcmp(cp_cat_ptt_string(ptt), "TX") != 0) {
		printf("test_cat: ptt tx parse failed\n");
		return 0;
	}
	if (cp_cat_ptt_from_string("bad", &ptt) == CP_OK) {
		printf("test_cat: invalid ptt accepted\n");
		return 0;
	}

	return 1;
}

static int
test_status_variants(void)
{
	struct cp_cat_config config;
	struct cp_cat_snapshot snapshot;

	cp_cat_default_config(&config);
	if (cp_cat_snapshot_update(&config, &snapshot) != CP_OK ||
	    snapshot.status != CP_CAT_STATUS_DISABLED ||
	    snapshot.enabled) {
		printf("test_cat: disabled snapshot mismatch\n");
		return 0;
	}

	config.backend = CP_CAT_BACKEND_MOCK;
	config.enabled = 1;
	config.mock_status = CP_CAT_STATUS_STALE;
	if (cp_cat_snapshot_update(&config, &snapshot) != CP_OK ||
	    snapshot.status != CP_CAT_STATUS_STALE ||
	    !snapshot.connected) {
		printf("test_cat: stale snapshot mismatch\n");
		return 0;
	}

	config.mock_status = CP_CAT_STATUS_ERROR;
	if (cp_cat_snapshot_update(&config, &snapshot) != CP_OK ||
	    snapshot.status != CP_CAT_STATUS_ERROR ||
	    snapshot.connected) {
		printf("test_cat: error snapshot mismatch\n");
		return 0;
	}

	return 1;
}

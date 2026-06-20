/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_cat_flrig.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_cat_flrig.h"

static int	test_config_defaults(void);
static int	test_frequency_parse(void);
static int	test_malformed_parse(void);
static int	test_mode_parse(void);
static int	test_ptt_parse(void);

int
main(void)
{
	if (!test_config_defaults())
		return 1;
	if (!test_frequency_parse())
		return 1;
	if (!test_mode_parse())
		return 1;
	if (!test_ptt_parse())
		return 1;
	if (!test_malformed_parse())
		return 1;

	return 0;
}

static int
test_config_defaults(void)
{
	struct cp_cat_config config;

	cp_cat_default_config(&config);
	if (strcmp(config.flrig_host, CP_CAT_FLRIG_DEFAULT_HOST) != 0 ||
	    config.flrig_port != CP_CAT_FLRIG_DEFAULT_PORT ||
	    config.timeout_ms != CP_CAT_DEFAULT_TIMEOUT_MS) {
		printf("test_cat_flrig: default config mismatch\n");
		return 0;
	}
	if (cp_cat_config_set_flrig_host(&config, "localhost") != CP_OK ||
	    strcmp(config.flrig_host, "localhost") != 0) {
		printf("test_cat_flrig: host set failed\n");
		return 0;
	}
	config.backend = CP_CAT_BACKEND_FLRIG;
	config.enabled = 1;
	config.flrig_port = 0;
	if (cp_cat_validate_config(&config) == CP_OK) {
		printf("test_cat_flrig: invalid port accepted\n");
		return 0;
	}
	config.flrig_port = CP_CAT_FLRIG_DEFAULT_PORT;
	config.timeout_ms = CP_CAT_MIN_TIMEOUT_MS - 1u;
	if (cp_cat_validate_config(&config) == CP_OK) {
		printf("test_cat_flrig: invalid timeout accepted\n");
		return 0;
	}

	return 1;
}

static int
test_frequency_parse(void)
{
	const char *xml;
	uint64_t frequency;

	xml = "HTTP/1.1 200 OK\r\n\r\n"
	    "<methodResponse><params><param><value>"
	    "<string>14230000</string>"
	    "</value></param></params></methodResponse>";
	if (cp_cat_flrig_parse_frequency(xml, &frequency) != CP_OK ||
	    frequency != 14230000ULL) {
		printf("test_cat_flrig: string frequency parse failed\n");
		return 0;
	}

	xml = "<methodResponse><params><param><value>"
	    "<double>14230000.000000</double>"
	    "</value></param></params></methodResponse>";
	if (cp_cat_flrig_parse_frequency(xml, &frequency) != CP_OK ||
	    frequency != 14230000ULL) {
		printf("test_cat_flrig: double frequency parse failed\n");
		return 0;
	}

	return 1;
}

static int
test_malformed_parse(void)
{
	uint64_t frequency;
	enum cp_cat_ptt_state ptt;
	char mode[CP_CAT_MODE_TEXT];

	if (cp_cat_flrig_parse_frequency("<fault></fault>",
	    &frequency) == CP_OK) {
		printf("test_cat_flrig: fault frequency accepted\n");
		return 0;
	}
	if (cp_cat_flrig_parse_mode("<value><string>USB!",
	    mode, sizeof(mode)) == CP_OK) {
		printf("test_cat_flrig: malformed mode accepted\n");
		return 0;
	}
	if (cp_cat_flrig_parse_ptt("<value><int>2</int></value>",
	    &ptt) == CP_OK) {
		printf("test_cat_flrig: invalid ptt accepted\n");
		return 0;
	}

	return 1;
}

static int
test_mode_parse(void)
{
	const char *xml;
	char mode[CP_CAT_MODE_TEXT];

	xml = "<methodResponse><params><param><value>"
	    "<string>USB</string>"
	    "</value></param></params></methodResponse>";
	if (cp_cat_flrig_parse_mode(xml, mode, sizeof(mode)) != CP_OK ||
	    strcmp(mode, "USB") != 0) {
		printf("test_cat_flrig: mode parse failed\n");
		return 0;
	}

	return 1;
}

static int
test_ptt_parse(void)
{
	const char *xml;
	enum cp_cat_ptt_state ptt;

	xml = "<methodResponse><params><param><value>"
	    "<int>0</int>"
	    "</value></param></params></methodResponse>";
	if (cp_cat_flrig_parse_ptt(xml, &ptt) != CP_OK ||
	    ptt != CP_CAT_PTT_OFF) {
		printf("test_cat_flrig: ptt off parse failed\n");
		return 0;
	}

	xml = "<methodResponse><params><param><value>"
	    "<i4>1</i4>"
	    "</value></param></params></methodResponse>";
	if (cp_cat_flrig_parse_ptt(xml, &ptt) != CP_OK ||
	    ptt != CP_CAT_PTT_ON) {
		printf("test_cat_flrig: ptt on parse failed\n");
		return 0;
	}

	return 1;
}

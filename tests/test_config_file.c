/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_config_file.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_audio.h"
#include "cp_config_file.h"

static int	test_comments_and_blank_lines(void);
static int	test_duplicate_key_rejected(void);
static int	test_forbidden_keys_rejected(void);
static int	test_invalid_backend_rejected(void);
static int	test_invalid_boolean_rejected(void);
static int	test_invalid_ranges_rejected(void);
static int	test_unknown_key_rejected(void);
static int	test_valid_examples(void);

int
main(void)
{
	if (!test_valid_examples())
		return 1;
	if (!test_unknown_key_rejected())
		return 1;
	if (!test_forbidden_keys_rejected())
		return 1;
	if (!test_duplicate_key_rejected())
		return 1;
	if (!test_invalid_backend_rejected())
		return 1;
	if (!test_invalid_boolean_rejected())
		return 1;
	if (!test_invalid_ranges_rejected())
		return 1;
	if (!test_comments_and_blank_lines())
		return 1;

	return 0;
}

static int
test_comments_and_blank_lines(void)
{
	struct cp_config_file config;
	struct cp_config_file_error error;

	cp_config_file_init(&config);
	if (cp_config_file_parse_line("# comment\n", 1, &config,
	    &error) != CP_OK ||
	    cp_config_file_parse_line("\n", 2, &config, &error) != CP_OK ||
	    cp_config_file_parse_line("   \t\n", 3, &config,
	    &error) != CP_OK ||
	    cp_config_file_parse_line("channels = 2\n", 4, &config,
	    &error) != CP_OK ||
	    cp_config_file_validate(&config, &error) != CP_OK) {
		printf("test_config_file: comments or blank lines failed\n");
		return 0;
	}

	return 1;
}

static int
test_duplicate_key_rejected(void)
{
	struct cp_config_file config;
	struct cp_config_file_error error;

	cp_config_file_init(&config);
	if (cp_config_file_parse_line("channels = 2\n", 10, &config,
	    &error) != CP_OK) {
		printf("test_config_file: duplicate setup failed\n");
		return 0;
	}
	if (cp_config_file_parse_line("channels = 1\n", 11, &config,
	    &error) == CP_OK ||
	    error.line_number != 11 ||
	    strcmp(error.key, "channels") != 0) {
		printf("test_config_file: duplicate key accepted\n");
		return 0;
	}

	return 1;
}

static int
test_forbidden_keys_rejected(void)
{
	struct cp_config_file config;
	struct cp_config_file_error error;

	cp_config_file_init(&config);
	if (cp_config_file_parse_line("ptt = on\n", 12, &config,
	    &error) == CP_OK ||
	    error.line_number != 12 ||
	    strcmp(error.key, "ptt") != 0) {
		printf("test_config_file: ptt key accepted\n");
		return 0;
	}
	if (cp_config_file_parse_line("cat_backend = mock\n", 13,
	    &config, &error) == CP_OK ||
	    error.line_number != 13 ||
	    strcmp(error.key, "cat_backend") != 0) {
		printf("test_config_file: cat backend key accepted\n");
		return 0;
	}
	if (cp_config_file_parse_line("frequency = 14230000\n", 14,
	    &config, &error) == CP_OK ||
	    error.line_number != 14 ||
	    strcmp(error.key, "frequency") != 0) {
		printf("test_config_file: frequency key accepted\n");
		return 0;
	}
	if (cp_config_file_parse_line("am_preset = am-safe\n", 15,
	    &config, &error) == CP_OK ||
	    error.line_number != 15 ||
	    strcmp(error.key, "am_preset") != 0) {
		printf("test_config_file: profile key accepted\n");
		return 0;
	}

	return 1;
}

static int
test_invalid_backend_rejected(void)
{
	struct cp_config_file config;
	struct cp_config_file_error error;

	cp_config_file_init(&config);
	if (cp_config_file_parse_line("audio_backend = oss\n", 20,
	    &config, &error) == CP_OK ||
	    error.line_number != 20 ||
	    strcmp(error.key, "audio_backend") != 0) {
		printf("test_config_file: invalid backend accepted\n");
		return 0;
	}

	return 1;
}

static int
test_invalid_boolean_rejected(void)
{
	struct cp_config_file config;
	struct cp_config_file_error error;

	cp_config_file_init(&config);
	if (cp_config_file_parse_line("gui = maybe\n", 21, &config,
	    &error) == CP_OK ||
	    error.line_number != 21 ||
	    strcmp(error.key, "gui") != 0) {
		printf("test_config_file: invalid boolean accepted\n");
		return 0;
	}

	return 1;
}

static int
test_invalid_ranges_rejected(void)
{
	struct cp_config_file config;
	struct cp_config_file_error error;

	cp_config_file_init(&config);
	if (cp_config_file_parse_line("sample_rate = 1\n", 30, &config,
	    &error) == CP_OK ||
	    error.line_number != 30 ||
	    strcmp(error.key, "sample_rate") != 0) {
		printf("test_config_file: invalid sample rate accepted\n");
		return 0;
	}
	cp_config_file_init(&config);
	if (cp_config_file_parse_line("channels = 3\n", 31, &config,
	    &error) == CP_OK ||
	    error.line_number != 31 ||
	    strcmp(error.key, "channels") != 0) {
		printf("test_config_file: invalid channels accepted\n");
		return 0;
	}
	cp_config_file_init(&config);
	if (cp_config_file_parse_line("block_size = 1\n", 32, &config,
	    &error) == CP_OK ||
	    error.line_number != 32 ||
	    strcmp(error.key, "block_size") != 0) {
		printf("test_config_file: invalid block size accepted\n");
		return 0;
	}
	cp_config_file_init(&config);
	if (cp_config_file_parse_line("meter_interval_ms = 1\n", 33,
	    &config, &error) == CP_OK ||
	    error.line_number != 33 ||
	    strcmp(error.key, "meter_interval_ms") != 0) {
		printf("test_config_file: invalid meter interval accepted\n");
		return 0;
	}
	cp_config_file_init(&config);
	if (cp_config_file_parse_line("sample_rate = nan\n", 34, &config,
	    &error) == CP_OK ||
	    error.line_number != 34 ||
	    strcmp(error.key, "sample_rate") != 0) {
		printf("test_config_file: non-finite sample rate accepted\n");
		return 0;
	}
	cp_config_file_init(&config);
	if (cp_config_file_parse_line("device = pulse\n", 35, &config,
	    &error) != CP_OK ||
	    cp_config_file_parse_line("output_device = 1\n", 36, &config,
	    &error) != CP_OK ||
	    cp_config_file_validate(&config, &error) == CP_OK ||
	    strcmp(error.key, "device") != 0) {
		printf("test_config_file: device conflict accepted\n");
		return 0;
	}

	return 1;
}

static int
test_unknown_key_rejected(void)
{
	struct cp_config_file config;
	struct cp_config_file_error error;

	cp_config_file_init(&config);
	if (cp_config_file_parse_line("unknown = value\n", 5, &config,
	    &error) == CP_OK ||
	    error.line_number != 5 ||
	    strcmp(error.key, "unknown") != 0) {
		printf("test_config_file: unknown key accepted\n");
		return 0;
	}

	return 1;
}

static int
test_valid_examples(void)
{
	struct cp_config_file config;
	struct cp_config_file_error error;

	if (cp_config_file_parse_file("configs/default.conf", &config,
	    &error) != CP_OK ||
	    strcmp(config.profile_path, "profiles/am-safe.profile") != 0 ||
	    config.audio_backend != CP_AUDIO_BACKEND_AUTO ||
	    config.channels != CP_CHANNELS_STEREO ||
	    config.tui_enabled != 0 ||
	    config.gui_enabled != 0) {
		printf("test_config_file: default config rejected\n");
		return 0;
	}
	if (cp_config_file_parse_file("configs/live-pulse.conf", &config,
	    &error) != CP_OK ||
	    config.audio_backend != CP_AUDIO_BACKEND_PULSE ||
	    config.tui_enabled != 1 ||
	    config.gui_enabled != 0 ||
	    config.sample_rate != 48000.0) {
		printf("test_config_file: live-pulse config rejected\n");
		return 0;
	}
	if (cp_config_file_parse_file("configs/gui-demo.conf", &config,
	    &error) != CP_OK ||
	    config.gui_enabled != 1 ||
	    config.tui_enabled != 0 ||
	    config.block_size != 256) {
		printf("test_config_file: gui-demo config rejected\n");
		return 0;
	}
	if (cp_config_file_parse_file("configs/playout.conf", &config,
	    &error) != CP_OK ||
	    config.audio_backend != CP_AUDIO_BACKEND_DEFAULT ||
	    config.output_device != CP_AUDIO_DEFAULT_DEVICE ||
	    strcmp(config.profile_path,
	    "profiles/file-cleanup.profile") != 0) {
		printf("test_config_file: playout config rejected\n");
		return 0;
	}

	return 1;
}

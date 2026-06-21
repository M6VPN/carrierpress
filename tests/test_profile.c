/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_profile.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_audio.h"
#include "cp_block.h"
#include "cp_profile.h"

static int	test_apply_examples(void);
static int	test_am_conflict_rejected(void);
static int	test_comments_and_blank_lines(void);
static int	test_duplicate_key_rejected(void);
static int	test_forbidden_key_rejected(void);
static int	test_invalid_enum_rejected(void);
static int	test_missing_mode_rejected(void);
static int	test_missing_name_rejected(void);
static int	test_unknown_key_rejected(void);
static int	test_valid_examples(void);

int
main(void)
{
	if (!test_valid_examples())
		return 1;
	if (!test_apply_examples())
		return 1;
	if (!test_unknown_key_rejected())
		return 1;
	if (!test_forbidden_key_rejected())
		return 1;
	if (!test_invalid_enum_rejected())
		return 1;
	if (!test_duplicate_key_rejected())
		return 1;
	if (!test_am_conflict_rejected())
		return 1;
	if (!test_missing_name_rejected())
		return 1;
	if (!test_missing_mode_rejected())
		return 1;
	if (!test_comments_and_blank_lines())
		return 1;

	return 0;
}

static int
test_apply_examples(void)
{
	struct cp_audio_config audio_config;
	struct cp_block_config block_config;
	struct cp_profile profile;
	struct cp_profile_error error;

	cp_audio_default_config(&audio_config);
	cp_block_default_config(&block_config, CP_CHANNELS_STEREO);
	if (cp_profile_parse_file("profiles/am-safe.profile", &profile,
	    &error) != CP_OK ||
	    cp_profile_apply_to_configs(&profile, &block_config,
	    &audio_config) != CP_OK ||
	    !block_config.am_config.enabled ||
	    block_config.ssb_config.enabled ||
	    !audio_config.am_config.enabled ||
	    audio_config.ssb_config.enabled ||
	    block_config.am_config.highpass_hz != 60.0f ||
	    block_config.dehummer_enabled != 1 ||
	    block_config.hum_base_frequency != 50.0f ||
	    block_config.hum_harmonic_count != 4 ||
	    block_config.multiband_enabled != 1 ||
	    block_config.multiband_preset != CP_MULTIBAND_PRESET_SPEECH ||
	    block_config.multiband2_enabled != 0 ||
	    block_config.bass_eq_config.enabled != 0 ||
	    block_config.natural_dynamics_config.enabled != 1 ||
	    block_config.low_level_boost_config.enabled != 0 ||
	    block_config.restoration_config.enabled != 1 ||
	    block_config.declipper_config.enabled != 0) {
		printf("test_profile: am-safe apply failed\n");
		return 0;
	}

	cp_audio_default_config(&audio_config);
	cp_block_default_config(&block_config, CP_CHANNELS_STEREO);
	if (cp_profile_parse_file("profiles/am-shortwave.profile", &profile,
	    &error) != CP_OK ||
	    cp_profile_apply_to_configs(&profile, &block_config,
	    &audio_config) != CP_OK ||
	    !block_config.am_config.enabled ||
	    block_config.ssb_config.enabled ||
	    block_config.multiband2_enabled != 1 ||
	    block_config.multiband2_preset != CP_MULTIBAND_PRESET_SPEECH ||
	    block_config.bass_eq_config.enabled != 1 ||
	    block_config.bass_eq_config.preset != CP_BASS_EQ_PRESET_SPEECH ||
	    block_config.low_level_boost_config.enabled != 1 ||
	    audio_config.multiband2_enabled !=
	    block_config.multiband2_enabled) {
		printf("test_profile: am-shortwave apply failed\n");
		return 0;
	}

	cp_audio_default_config(&audio_config);
	cp_block_default_config(&block_config, CP_CHANNELS_STEREO);
	if (cp_profile_parse_file("profiles/ssb-speech.profile", &profile,
	    &error) != CP_OK ||
	    cp_profile_apply_to_configs(&profile, &block_config,
	    &audio_config) != CP_OK ||
	    block_config.am_config.enabled ||
	    !block_config.ssb_config.enabled ||
	    audio_config.am_config.enabled ||
	    !audio_config.ssb_config.enabled ||
	    block_config.ssb_config.lowpass_hz != 2800.0f ||
	    block_config.bass_eq_config.enabled != 1 ||
	    block_config.bass_eq_config.preset != CP_BASS_EQ_PRESET_SPEECH ||
	    block_config.hum_harmonic_count != 3) {
		printf("test_profile: ssb-speech apply failed\n");
		return 0;
	}

	cp_audio_default_config(&audio_config);
	cp_block_default_config(&block_config, CP_CHANNELS_STEREO);
	if (cp_profile_parse_file("profiles/file-cleanup.profile", &profile,
	    &error) != CP_OK ||
	    cp_profile_apply_to_configs(&profile, &block_config,
	    &audio_config) != CP_OK ||
	    block_config.am_config.enabled ||
	    block_config.ssb_config.enabled ||
	    audio_config.am_config.enabled ||
	    audio_config.ssb_config.enabled ||
	    block_config.multiband_enabled != 1 ||
	    block_config.multiband_preset != CP_MULTIBAND_PRESET_MUSIC ||
	    block_config.bass_eq_config.enabled != 1 ||
	    block_config.bass_eq_config.preset != CP_BASS_EQ_PRESET_WARM ||
	    block_config.restoration_config.enabled != 1 ||
	    block_config.declipper_config.enabled != 1) {
		printf("test_profile: file-cleanup apply failed\n");
		return 0;
	}

	return 1;
}

static int
test_am_conflict_rejected(void)
{
	struct cp_profile profile;
	struct cp_profile_error error;

	cp_profile_init(&profile);
	if (cp_profile_parse_line("name = conflict\n", 1, &profile,
	    &error) != CP_OK ||
	    cp_profile_parse_line("mode = am\n", 2, &profile,
	    &error) != CP_OK ||
	    cp_profile_parse_line("ssb_preset = ssb-speech\n", 3,
	    &profile, &error) != CP_OK) {
		printf("test_profile: conflict setup failed\n");
		return 0;
	}
	if (cp_profile_validate(&profile, &error) == CP_OK ||
	    strcmp(error.key, "ssb_preset") != 0) {
		printf("test_profile: AM/SSB conflict accepted\n");
		return 0;
	}

	return 1;
}

static int
test_comments_and_blank_lines(void)
{
	struct cp_profile profile;
	struct cp_profile_error error;

	cp_profile_init(&profile);
	if (cp_profile_parse_line("# comment\n", 1, &profile, &error) !=
	    CP_OK ||
	    cp_profile_parse_line("\n", 2, &profile, &error) != CP_OK ||
	    cp_profile_parse_line("   \t\n", 3, &profile, &error) != CP_OK ||
	    cp_profile_parse_line("name = Minimal\n", 4, &profile,
	    &error) != CP_OK ||
	    cp_profile_parse_line("mode = neutral\n", 5, &profile,
	    &error) != CP_OK) {
		printf("test_profile: comments or blank lines failed\n");
		return 0;
	}
	if (cp_profile_validate(&profile, &error) != CP_OK) {
		printf("test_profile: minimal profile rejected\n");
		return 0;
	}

	return 1;
}

static int
test_duplicate_key_rejected(void)
{
	struct cp_profile profile;
	struct cp_profile_error error;

	cp_profile_init(&profile);
	if (cp_profile_parse_line("name = first\n", 10, &profile,
	    &error) != CP_OK) {
		printf("test_profile: first duplicate setup failed\n");
		return 0;
	}
	if (cp_profile_parse_line("name = second\n", 11, &profile,
	    &error) == CP_OK ||
	    error.line_number != 11 ||
	    strcmp(error.key, "name") != 0) {
		printf("test_profile: duplicate key accepted\n");
		return 0;
	}

	return 1;
}

static int
test_forbidden_key_rejected(void)
{
	struct cp_profile profile;
	struct cp_profile_error error;

	cp_profile_init(&profile);
	if (cp_profile_parse_line("ptt = on\n", 12, &profile, &error) ==
	    CP_OK ||
	    error.line_number != 12 ||
	    strcmp(error.key, "ptt") != 0) {
		printf("test_profile: ptt key accepted\n");
		return 0;
	}
	if (cp_profile_parse_line("frequency = 14230000\n", 13,
	    &profile, &error) == CP_OK ||
	    error.line_number != 13 ||
	    strcmp(error.key, "frequency") != 0) {
		printf("test_profile: frequency key accepted\n");
		return 0;
	}

	return 1;
}

static int
test_invalid_enum_rejected(void)
{
	struct cp_profile profile;
	struct cp_profile_error error;

	cp_profile_init(&profile);
	if (cp_profile_parse_line("mode = broadcast\n", 7, &profile,
	    &error) == CP_OK ||
	    error.line_number != 7 ||
	    strcmp(error.key, "mode") != 0) {
		printf("test_profile: invalid mode accepted\n");
		return 0;
	}
	cp_profile_init(&profile);
	if (cp_profile_parse_line("bass_eq = custom\n", 8, &profile,
	    &error) == CP_OK ||
	    error.line_number != 8 ||
	    strcmp(error.key, "bass_eq") != 0) {
		printf("test_profile: invalid bass EQ accepted\n");
		return 0;
	}

	return 1;
}

static int
test_missing_mode_rejected(void)
{
	struct cp_profile profile;
	struct cp_profile_error error;

	cp_profile_init(&profile);
	if (cp_profile_parse_line("name = missing mode\n", 1, &profile,
	    &error) != CP_OK) {
		printf("test_profile: missing mode setup failed\n");
		return 0;
	}
	if (cp_profile_validate(&profile, &error) == CP_OK ||
	    strcmp(error.key, "mode") != 0) {
		printf("test_profile: missing mode accepted\n");
		return 0;
	}

	return 1;
}

static int
test_missing_name_rejected(void)
{
	struct cp_profile profile;
	struct cp_profile_error error;

	cp_profile_init(&profile);
	if (cp_profile_parse_line("mode = neutral\n", 1, &profile,
	    &error) != CP_OK) {
		printf("test_profile: missing name setup failed\n");
		return 0;
	}
	if (cp_profile_validate(&profile, &error) == CP_OK ||
	    strcmp(error.key, "name") != 0) {
		printf("test_profile: missing name accepted\n");
		return 0;
	}

	return 1;
}

static int
test_unknown_key_rejected(void)
{
	struct cp_profile profile;
	struct cp_profile_error error;

	cp_profile_init(&profile);
	if (cp_profile_parse_line("unknown = value\n", 5, &profile,
	    &error) == CP_OK ||
	    error.line_number != 5 ||
	    strcmp(error.key, "unknown") != 0) {
		printf("test_profile: unknown key accepted\n");
		return 0;
	}

	return 1;
}

static int
test_valid_examples(void)
{
	struct cp_profile profile;
	struct cp_profile_error error;

	if (cp_profile_parse_file("profiles/am-safe.profile", &profile,
	    &error) != CP_OK ||
	    profile.mode != CP_PROFILE_MODE_AM ||
	    profile.am_preset != CP_PROFILE_AM_PRESET_SAFE ||
	    profile.ssb_preset != CP_PROFILE_SSB_PRESET_OFF) {
		printf("test_profile: am-safe profile rejected\n");
		return 0;
	}
	if (cp_profile_parse_file("profiles/am-shortwave.profile",
	    &profile, &error) != CP_OK ||
	    profile.mode != CP_PROFILE_MODE_AM ||
	    profile.am_preset != CP_PROFILE_AM_PRESET_SHORTWAVE ||
	    profile.multiband2 != CP_PROFILE_MULTIBAND_SPEECH) {
		printf("test_profile: am-shortwave profile rejected\n");
		return 0;
	}
	if (cp_profile_parse_file("profiles/ssb-speech.profile",
	    &profile, &error) != CP_OK ||
	    profile.mode != CP_PROFILE_MODE_SSB ||
	    profile.ssb_preset != CP_PROFILE_SSB_PRESET_SPEECH ||
	    profile.am_preset != CP_PROFILE_AM_PRESET_OFF) {
		printf("test_profile: ssb-speech profile rejected\n");
		return 0;
	}
	if (cp_profile_parse_file("profiles/file-cleanup.profile",
	    &profile, &error) != CP_OK ||
	    profile.mode != CP_PROFILE_MODE_FILE_CLEANUP ||
	    profile.declipper != CP_PROFILE_SWITCH_ON ||
	    profile.am_preset != CP_PROFILE_AM_PRESET_OFF ||
	    profile.ssb_preset != CP_PROFILE_SSB_PRESET_OFF) {
		printf("test_profile: file-cleanup profile rejected\n");
		return 0;
	}

	return 1;
}

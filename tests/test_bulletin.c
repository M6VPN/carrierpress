/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_bulletin.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_bulletin.h"

static int	test_carousel_schedule_parse(void);
static int	test_data_clean_profile(void);
static int	test_hf_ssb_voice_profile(void);
static int	test_ptt_gate(void);
static int	test_profile_lookup(void);

int
main(void)
{
	if (!test_profile_lookup())
		return 1;
	if (!test_hf_ssb_voice_profile())
		return 1;
	if (!test_data_clean_profile())
		return 1;
	if (!test_ptt_gate())
		return 1;
	if (!test_carousel_schedule_parse())
		return 1;

	return 0;
}

static int
test_carousel_schedule_parse(void)
{
	struct cp_bulletin_schedule schedule;
	char error[128];

	cp_bulletin_schedule_init(&schedule);
	if (cp_bulletin_schedule_parse_line("callsign = \"M6VPN\"\n", 1,
	    &schedule, error, sizeof(error)) != CP_BULLETIN_OK ||
	    cp_bulletin_schedule_parse_line("profile = \"hf-ssb-voice\"\n",
	    2, &schedule, error, sizeof(error)) != CP_BULLETIN_OK ||
	    cp_bulletin_schedule_parse_line("repeat = 3\n", 3, &schedule,
	    error, sizeof(error)) != CP_BULLETIN_OK ||
	    cp_bulletin_schedule_parse_line("pre_roll_ms = 500\n", 4,
	    &schedule, error, sizeof(error)) != CP_BULLETIN_OK ||
	    cp_bulletin_schedule_parse_line("post_roll_ms = 800\n", 5,
	    &schedule, error, sizeof(error)) != CP_BULLETIN_OK ||
	    cp_bulletin_schedule_parse_line("[[items]]\n", 6, &schedule,
	    error, sizeof(error)) != CP_BULLETIN_OK ||
	    cp_bulletin_schedule_parse_line("type = \"id\"\n", 7,
	    &schedule, error, sizeof(error)) != CP_BULLETIN_OK ||
	    cp_bulletin_schedule_parse_line(
	    "text = \"M6VPN test transmission\"\n", 8, &schedule, error,
	    sizeof(error)) != CP_BULLETIN_OK ||
	    cp_bulletin_schedule_parse_line("[[items]]\n", 9, &schedule,
	    error, sizeof(error)) != CP_BULLETIN_OK ||
	    cp_bulletin_schedule_parse_line("type = \"file\"\n", 10,
	    &schedule, error, sizeof(error)) != CP_BULLETIN_OK ||
	    cp_bulletin_schedule_parse_line("path = \"bulletin.wav\"\n", 11,
	    &schedule, error, sizeof(error)) != CP_BULLETIN_OK ||
	    cp_bulletin_schedule_parse_line("[[items]]\n", 12, &schedule,
	    error, sizeof(error)) != CP_BULLETIN_OK ||
	    cp_bulletin_schedule_parse_line("type = \"pause\"\n", 13,
	    &schedule, error, sizeof(error)) != CP_BULLETIN_OK ||
	    cp_bulletin_schedule_parse_line("seconds = 10\n", 14,
	    &schedule, error, sizeof(error)) != CP_BULLETIN_OK) {
		printf("test_bulletin: schedule parse failed: %s\n", error);
		return 0;
	}
	if (strcmp(schedule.callsign, "M6VPN") != 0 ||
	    schedule.profile_id != CP_BULLETIN_PROFILE_HF_SSB_VOICE ||
	    schedule.repeat != 3 ||
	    schedule.pre_roll_ms != 500 ||
	    schedule.post_roll_ms != 800 ||
	    schedule.item_count != 3 ||
	    schedule.items[0].type != CP_BULLETIN_ITEM_ID ||
	    schedule.items[1].type != CP_BULLETIN_ITEM_FILE ||
	    strcmp(schedule.items[1].path, "bulletin.wav") != 0 ||
	    schedule.items[2].seconds != 10) {
		printf("test_bulletin: schedule values mismatch\n");
		return 0;
	}

	return 1;
}

static int
test_data_clean_profile(void)
{
	struct cp_audio_config audio_config;
	struct cp_block_config block_config;

	cp_audio_default_config(&audio_config);
	cp_block_default_config(&block_config, CP_CHANNELS_STEREO);
	if (cp_bulletin_apply_profile(
	    CP_BULLETIN_PROFILE_DATA_CLEAN_PASS_THROUGH, &block_config,
	    &audio_config) != CP_BULLETIN_OK) {
		printf("test_bulletin: data profile apply failed\n");
		return 0;
	}
	if (block_config.ssb_config.enabled ||
	    block_config.am_config.enabled ||
	    block_config.multiband_enabled ||
	    block_config.multiband2_enabled ||
	    block_config.bass_eq_config.enabled ||
	    block_config.natural_dynamics_config.enabled ||
	    block_config.low_level_boost_config.enabled ||
	    block_config.restoration_config.enabled ||
	    block_config.declipper_config.enabled ||
	    block_config.limiter_ceiling != 1.0f) {
		printf("test_bulletin: data profile changed speech path\n");
		return 0;
	}

	return 1;
}

static int
test_hf_ssb_voice_profile(void)
{
	struct cp_audio_config audio_config;
	struct cp_block_config block_config;

	cp_audio_default_config(&audio_config);
	cp_block_default_config(&block_config, CP_CHANNELS_STEREO);
	if (cp_bulletin_apply_profile(CP_BULLETIN_PROFILE_HF_SSB_VOICE,
	    &block_config, &audio_config) != CP_BULLETIN_OK) {
		printf("test_bulletin: hf voice profile apply failed\n");
		return 0;
	}
	if (!block_config.ssb_config.enabled ||
	    block_config.am_config.enabled ||
	    block_config.channels != CP_CHANNELS_MONO ||
	    audio_config.channels != CP_CHANNELS_MONO ||
	    block_config.ssb_config.highpass_hz != 150.0f ||
	    block_config.ssb_config.lowpass_hz != 2700.0f ||
	    block_config.multiband_enabled != 1 ||
	    block_config.bass_eq_config.enabled != 1 ||
	    block_config.natural_dynamics_config.enabled != 1 ||
	    block_config.limiter_ceiling > 0.90f) {
		printf("test_bulletin: hf voice profile mismatch\n");
		return 0;
	}

	return 1;
}

static int
test_profile_lookup(void)
{
	enum cp_bulletin_profile_id id;
	const struct cp_bulletin_profile_summary *summary;

	if (cp_bulletin_profile_from_string("hf-ssb-voice", &id) !=
	    CP_BULLETIN_OK || id != CP_BULLETIN_PROFILE_HF_SSB_VOICE)
		return 0;
	summary = cp_bulletin_profile_summary(id);
	if (summary == NULL || summary->highpass_hz != 150.0 ||
	    summary->lowpass_hz != 2700.0 ||
	    summary->speech_compressor_enabled != 1 ||
	    summary->limiter_enabled != 1)
		return 0;
	if (cp_bulletin_profile_from_string("bad-profile", &id) !=
	    CP_BULLETIN_ERR_RANGE)
		return 0;

	return 1;
}

static int
test_ptt_gate(void)
{
	struct cp_bulletin_tx_gate gate;
	char reason[128];

	(void)memset(&gate, 0, sizeof(gate));
	gate.ptt_mode = CP_BULLETIN_PTT_CAT;
	if (cp_bulletin_tx_gate_validate(&gate, reason, sizeof(reason)) !=
	    CP_BULLETIN_ERR_TX_ARM ||
	    strstr(reason, "--arm-tx") == NULL) {
		printf("test_bulletin: unarmed PTT accepted\n");
		return 0;
	}
	gate.arm_tx = 1;
	if (cp_bulletin_tx_gate_validate(&gate, reason, sizeof(reason)) !=
	    CP_BULLETIN_ERR_TX_ARM ||
	    strstr(reason, "--id") == NULL) {
		printf("test_bulletin: armed PTT without ID accepted\n");
		return 0;
	}
	gate.station_id = "M6VPN";
	if (cp_bulletin_tx_gate_validate(&gate, reason, sizeof(reason)) !=
	    CP_BULLETIN_OK) {
		printf("test_bulletin: armed PTT gate rejected: %s\n",
		    reason);
		return 0;
	}
	gate.ptt_mode = CP_BULLETIN_PTT_VOX;
	gate.arm_tx = 0;
	gate.station_id = NULL;
	if (cp_bulletin_tx_gate_validate(&gate, reason, sizeof(reason)) !=
	    CP_BULLETIN_OK) {
		printf("test_bulletin: VOX audio-only gate rejected\n");
		return 0;
	}

	return 1;
}

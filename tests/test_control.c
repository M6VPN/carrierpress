/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_control.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_control.h"

static int	test_apply_am_off(void);
static int	test_apply_am_preset(void);
static int	test_apply_mutual_exclusion(void);
static int	test_apply_ssb_off(void);
static int	test_apply_ssb_preset(void);
static int	test_command_keys(void);

int
main(void)
{
	if (!test_command_keys())
		return 1;
	if (!test_apply_am_preset())
		return 1;
	if (!test_apply_am_off())
		return 1;
	if (!test_apply_ssb_preset())
		return 1;
	if (!test_apply_ssb_off())
		return 1;
	if (!test_apply_mutual_exclusion())
		return 1;

	return 0;
}

static int
test_apply_am_off(void)
{
	struct cp_block_config config;
	struct cp_block_processor processor;
	struct cp_control_command command;

	cp_block_default_config(&config, CP_CHANNELS_MONO);
	config.am_config.enabled = 1;
	if (cp_block_init(&processor, &config) != CP_OK)
		return 0;

	command.type = CP_CONTROL_COMMAND_AM_OFF;
	command.am_preset = CP_AM_PRESET_SAFE;
	if (cp_control_apply(&processor, &command) != CP_OK)
		return 0;
	if (processor.am.config.enabled || processor.am.enabled) {
		printf("test_control: AM off command failed\n");
		return 0;
	}

	return 1;
}

static int
test_apply_mutual_exclusion(void)
{
	struct cp_block_config config;
	struct cp_block_processor processor;
	struct cp_control_command command;

	cp_block_default_config(&config, CP_CHANNELS_MONO);
	if (cp_block_init(&processor, &config) != CP_OK)
		return 0;

	cp_control_command_clear(&command);
	command.type = CP_CONTROL_COMMAND_AM_PRESET;
	command.am_preset = CP_AM_PRESET_SAFE;
	if (cp_control_apply(&processor, &command) != CP_OK)
		return 0;
	if (!processor.am.config.enabled)
		return 0;

	cp_control_command_clear(&command);
	command.type = CP_CONTROL_COMMAND_SSB_PRESET;
	command.ssb_preset = CP_SSB_PRESET_NARROW;
	if (cp_control_apply(&processor, &command) != CP_OK)
		return 0;
	if (processor.am.config.enabled || processor.am.enabled ||
	    !processor.ssb.config.enabled || !processor.ssb.enabled) {
		printf("test_control: SSB did not disable AM\n");
		return 0;
	}

	cp_control_command_clear(&command);
	command.type = CP_CONTROL_COMMAND_AM_PRESET;
	command.am_preset = CP_AM_PRESET_SHORTWAVE;
	if (cp_control_apply(&processor, &command) != CP_OK)
		return 0;
	if (!processor.am.config.enabled || !processor.am.enabled ||
	    processor.ssb.config.enabled || processor.ssb.enabled) {
		printf("test_control: AM did not disable SSB\n");
		return 0;
	}

	return 1;
}

static int
test_apply_am_preset(void)
{
	struct cp_block_config config;
	struct cp_block_processor processor;
	struct cp_control_command command;

	cp_block_default_config(&config, CP_CHANNELS_MONO);
	if (cp_block_init(&processor, &config) != CP_OK)
		return 0;

	command.type = CP_CONTROL_COMMAND_AM_PRESET;
	command.am_preset = CP_AM_PRESET_SHORTWAVE;
	if (cp_control_apply(&processor, &command) != CP_OK)
		return 0;
	if (!processor.am.config.enabled ||
	    strcmp(processor.am.config.preset_name, "am-shortwave") != 0) {
		printf("test_control: AM preset command failed\n");
		return 0;
	}

	return 1;
}

static int
test_apply_ssb_off(void)
{
	struct cp_block_config config;
	struct cp_block_processor processor;
	struct cp_control_command command;

	cp_block_default_config(&config, CP_CHANNELS_MONO);
	config.ssb_config.enabled = 1;
	if (cp_block_init(&processor, &config) != CP_OK)
		return 0;

	cp_control_command_clear(&command);
	command.type = CP_CONTROL_COMMAND_SSB_OFF;
	if (cp_control_apply(&processor, &command) != CP_OK)
		return 0;
	if (processor.ssb.config.enabled || processor.ssb.enabled) {
		printf("test_control: SSB off command failed\n");
		return 0;
	}

	return 1;
}

static int
test_apply_ssb_preset(void)
{
	struct cp_block_config config;
	struct cp_block_processor processor;
	struct cp_control_command command;

	cp_block_default_config(&config, CP_CHANNELS_MONO);
	if (cp_block_init(&processor, &config) != CP_OK)
		return 0;

	cp_control_command_clear(&command);
	command.type = CP_CONTROL_COMMAND_SSB_PRESET;
	command.ssb_preset = CP_SSB_PRESET_NARROW;
	if (cp_control_apply(&processor, &command) != CP_OK)
		return 0;
	if (!processor.ssb.config.enabled ||
	    strcmp(processor.ssb.config.preset_name, "ssb-narrow") != 0) {
		printf("test_control: SSB preset command failed\n");
		return 0;
	}

	return 1;
}

static int
test_command_keys(void)
{
	struct cp_control_command command;

	if (cp_control_command_from_key('a', CP_CONTROL_BANK_AM,
	    &command) != CP_OK)
		return 0;
	if (command.type != CP_CONTROL_COMMAND_SELECT_AM ||
	    command.bank != CP_CONTROL_BANK_AM)
		return 0;
	if (cp_control_command_from_key('s', CP_CONTROL_BANK_AM,
	    &command) != CP_OK)
		return 0;
	if (command.type != CP_CONTROL_COMMAND_SELECT_SSB ||
	    command.bank != CP_CONTROL_BANK_SSB)
		return 0;
	if (cp_control_command_from_key('1', CP_CONTROL_BANK_AM,
	    &command) != CP_OK)
		return 0;
	if (command.type != CP_CONTROL_COMMAND_AM_PRESET ||
	    command.am_preset != CP_AM_PRESET_SAFE)
		return 0;
	if (cp_control_command_from_key('4', CP_CONTROL_BANK_AM,
	    &command) != CP_OK)
		return 0;
	if (command.type != CP_CONTROL_COMMAND_AM_PRESET ||
	    command.am_preset != CP_AM_PRESET_VOICE)
		return 0;
	if (cp_control_command_from_key('0', CP_CONTROL_BANK_AM,
	    &command) != CP_OK)
		return 0;
	if (command.type != CP_CONTROL_COMMAND_AM_OFF)
		return 0;
	if (cp_control_command_from_key('1', CP_CONTROL_BANK_SSB,
	    &command) != CP_OK)
		return 0;
	if (command.type != CP_CONTROL_COMMAND_SSB_PRESET ||
	    command.ssb_preset != CP_SSB_PRESET_SPEECH)
		return 0;
	if (cp_control_command_from_key('4', CP_CONTROL_BANK_SSB,
	    &command) != CP_OK)
		return 0;
	if (command.type != CP_CONTROL_COMMAND_SSB_PRESET ||
	    command.ssb_preset != CP_SSB_PRESET_GENTLE)
		return 0;
	if (cp_control_command_from_key('0', CP_CONTROL_BANK_SSB,
	    &command) != CP_OK)
		return 0;
	if (command.type != CP_CONTROL_COMMAND_SSB_OFF)
		return 0;
	if (cp_control_command_from_key('n', CP_CONTROL_BANK_AM,
	    &command) != CP_OK)
		return 0;
	if (command.type != CP_CONTROL_COMMAND_PLAYOUT_NEXT)
		return 0;
	if (cp_control_command_from_key('N', CP_CONTROL_BANK_SSB,
	    &command) != CP_OK)
		return 0;
	if (command.type != CP_CONTROL_COMMAND_PLAYOUT_NEXT)
		return 0;
	if (cp_control_command_from_key('x', CP_CONTROL_BANK_AM,
	    &command) != CP_ERR_RANGE)
		return 0;
	if (cp_control_command_from_key('1',
	    (enum cp_control_bank)99, &command) != CP_ERR_RANGE)
		return 0;

	return 1;
}

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_control.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_control.h"

static int	test_apply_am_off(void);
static int	test_apply_am_preset(void);
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
test_command_keys(void)
{
	struct cp_control_command command;

	if (cp_control_command_from_key('1', &command) != CP_OK)
		return 0;
	if (command.type != CP_CONTROL_COMMAND_AM_PRESET ||
	    command.am_preset != CP_AM_PRESET_SAFE)
		return 0;
	if (cp_control_command_from_key('4', &command) != CP_OK)
		return 0;
	if (command.type != CP_CONTROL_COMMAND_AM_PRESET ||
	    command.am_preset != CP_AM_PRESET_VOICE)
		return 0;
	if (cp_control_command_from_key('0', &command) != CP_OK)
		return 0;
	if (command.type != CP_CONTROL_COMMAND_AM_OFF)
		return 0;
	if (cp_control_command_from_key('x', &command) != CP_ERR_RANGE)
		return 0;

	return 1;
}

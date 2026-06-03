/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_control.c */

#include <sys/types.h>

#include "cp_control.h"

int
cp_control_apply(struct cp_block_processor *processor,
	const struct cp_control_command *command)
{
	struct cp_am_config config;
	int status;

	if (processor == NULL || command == NULL)
		return CP_ERR_NULL;

	switch (command->type) {
	case CP_CONTROL_COMMAND_NONE:
	case CP_CONTROL_COMMAND_STOP:
	case CP_CONTROL_COMMAND_PLAYOUT_NEXT:
		return CP_OK;
	case CP_CONTROL_COMMAND_AM_OFF:
		processor->am.config.enabled = 0;
		processor->am.enabled = 0;
		return cp_am_reset(&processor->am);
	case CP_CONTROL_COMMAND_AM_PRESET:
		config = processor->am.config;
		status = cp_am_apply_preset_id(&config, command->am_preset);
		if (status != CP_OK)
			return status;
		config.enabled       = 1;
		config.channel_count = processor->channels;
		status = cp_am_init(&processor->am, &config);
		if (status != CP_OK)
			return status;
		return CP_OK;
	default:
		return CP_ERR_RANGE;
	}
}

void
cp_control_command_clear(struct cp_control_command *command)
{
	if (command == NULL)
		return;

	command->type      = CP_CONTROL_COMMAND_NONE;
	command->am_preset = CP_AM_PRESET_SAFE;
}

int
cp_control_command_from_key(int key, struct cp_control_command *command)
{
	if (command == NULL)
		return CP_ERR_NULL;

	cp_control_command_clear(command);
	switch (key) {
	case 'q':
	case 'Q':
		command->type = CP_CONTROL_COMMAND_STOP;
		return CP_OK;
	case 'n':
	case 'N':
		command->type = CP_CONTROL_COMMAND_PLAYOUT_NEXT;
		return CP_OK;
	case '0':
		command->type = CP_CONTROL_COMMAND_AM_OFF;
		return CP_OK;
	case '1':
		command->type = CP_CONTROL_COMMAND_AM_PRESET;
		command->am_preset = CP_AM_PRESET_SAFE;
		return CP_OK;
	case '2':
		command->type = CP_CONTROL_COMMAND_AM_PRESET;
		command->am_preset = CP_AM_PRESET_SHORTWAVE;
		return CP_OK;
	case '3':
		command->type = CP_CONTROL_COMMAND_AM_PRESET;
		command->am_preset = CP_AM_PRESET_WIDE;
		return CP_OK;
	case '4':
		command->type = CP_CONTROL_COMMAND_AM_PRESET;
		command->am_preset = CP_AM_PRESET_VOICE;
		return CP_OK;
	default:
		return CP_ERR_RANGE;
	}
}

const char *
cp_control_command_string(enum cp_control_command_type type)
{
	switch (type) {
	case CP_CONTROL_COMMAND_NONE:
		return "none";
	case CP_CONTROL_COMMAND_STOP:
		return "stop";
	case CP_CONTROL_COMMAND_PLAYOUT_NEXT:
		return "playout-next";
	case CP_CONTROL_COMMAND_AM_OFF:
		return "am-off";
	case CP_CONTROL_COMMAND_AM_PRESET:
		return "am-preset";
	default:
		return "unknown";
	}
}

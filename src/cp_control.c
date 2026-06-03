/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_control.c */

#include <sys/types.h>

#include "cp_control.h"

int
cp_control_apply(struct cp_block_processor *processor,
	const struct cp_control_command *command)
{
	struct cp_am_config am_config;
	struct cp_ssb_config ssb_config;
	int status;

	if (processor == NULL || command == NULL)
		return CP_ERR_NULL;

	switch (command->type) {
	case CP_CONTROL_COMMAND_NONE:
	case CP_CONTROL_COMMAND_STOP:
	case CP_CONTROL_COMMAND_PLAYOUT_NEXT:
	case CP_CONTROL_COMMAND_SELECT_AM:
	case CP_CONTROL_COMMAND_SELECT_SSB:
		return CP_OK;
	case CP_CONTROL_COMMAND_AM_OFF:
		processor->am.config.enabled = 0;
		processor->am.enabled = 0;
		return cp_am_reset(&processor->am);
	case CP_CONTROL_COMMAND_AM_PRESET:
		am_config = processor->am.config;
		status = cp_am_apply_preset_id(&am_config, command->am_preset);
		if (status != CP_OK)
			return status;
		am_config.enabled       = 1;
		am_config.channel_count = processor->channels;
		status = cp_am_init(&processor->am, &am_config);
		if (status != CP_OK)
			return status;
		processor->ssb.config.enabled = 0;
		processor->ssb.enabled = 0;
		return cp_ssb_reset(&processor->ssb);
	case CP_CONTROL_COMMAND_SSB_OFF:
		processor->ssb.config.enabled = 0;
		processor->ssb.enabled = 0;
		return cp_ssb_reset(&processor->ssb);
	case CP_CONTROL_COMMAND_SSB_PRESET:
		ssb_config = processor->ssb.config;
		status = cp_ssb_apply_preset_id(&ssb_config,
		    command->ssb_preset);
		if (status != CP_OK)
			return status;
		ssb_config.enabled       = 1;
		ssb_config.channel_count = processor->channels;
		status = cp_ssb_init(&processor->ssb, &ssb_config);
		if (status != CP_OK)
			return status;
		processor->am.config.enabled = 0;
		processor->am.enabled = 0;
		return cp_am_reset(&processor->am);
	default:
		return CP_ERR_RANGE;
	}
}

void
cp_control_command_clear(struct cp_control_command *command)
{
	if (command == NULL)
		return;

	command->type       = CP_CONTROL_COMMAND_NONE;
	command->bank       = CP_CONTROL_BANK_AM;
	command->am_preset  = CP_AM_PRESET_SAFE;
	command->ssb_preset = CP_SSB_PRESET_SPEECH;
}

int
cp_control_command_from_key(int key, enum cp_control_bank bank,
	struct cp_control_command *command)
{
	if (command == NULL)
		return CP_ERR_NULL;
	if (bank != CP_CONTROL_BANK_AM && bank != CP_CONTROL_BANK_SSB)
		return CP_ERR_RANGE;

	cp_control_command_clear(command);
	command->bank = bank;
	switch (key) {
	case 'q':
	case 'Q':
		command->type = CP_CONTROL_COMMAND_STOP;
		return CP_OK;
	case 'n':
	case 'N':
		command->type = CP_CONTROL_COMMAND_PLAYOUT_NEXT;
		return CP_OK;
	case 'a':
	case 'A':
		command->type = CP_CONTROL_COMMAND_SELECT_AM;
		command->bank = CP_CONTROL_BANK_AM;
		return CP_OK;
	case 's':
	case 'S':
		command->type = CP_CONTROL_COMMAND_SELECT_SSB;
		command->bank = CP_CONTROL_BANK_SSB;
		return CP_OK;
	case '0':
		command->type = bank == CP_CONTROL_BANK_AM ?
		    CP_CONTROL_COMMAND_AM_OFF : CP_CONTROL_COMMAND_SSB_OFF;
		return CP_OK;
	case '1':
		command->type = bank == CP_CONTROL_BANK_AM ?
		    CP_CONTROL_COMMAND_AM_PRESET : CP_CONTROL_COMMAND_SSB_PRESET;
		command->am_preset = CP_AM_PRESET_SAFE;
		command->ssb_preset = CP_SSB_PRESET_SPEECH;
		return CP_OK;
	case '2':
		command->type = bank == CP_CONTROL_BANK_AM ?
		    CP_CONTROL_COMMAND_AM_PRESET : CP_CONTROL_COMMAND_SSB_PRESET;
		command->am_preset = CP_AM_PRESET_SHORTWAVE;
		command->ssb_preset = CP_SSB_PRESET_NARROW;
		return CP_OK;
	case '3':
		command->type = bank == CP_CONTROL_BANK_AM ?
		    CP_CONTROL_COMMAND_AM_PRESET : CP_CONTROL_COMMAND_SSB_PRESET;
		command->am_preset = CP_AM_PRESET_WIDE;
		command->ssb_preset = CP_SSB_PRESET_WIDE;
		return CP_OK;
	case '4':
		command->type = bank == CP_CONTROL_BANK_AM ?
		    CP_CONTROL_COMMAND_AM_PRESET : CP_CONTROL_COMMAND_SSB_PRESET;
		command->am_preset = CP_AM_PRESET_VOICE;
		command->ssb_preset = CP_SSB_PRESET_GENTLE;
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
	case CP_CONTROL_COMMAND_SELECT_AM:
		return "select-am";
	case CP_CONTROL_COMMAND_SELECT_SSB:
		return "select-ssb";
	case CP_CONTROL_COMMAND_AM_OFF:
		return "am-off";
	case CP_CONTROL_COMMAND_AM_PRESET:
		return "am-preset";
	case CP_CONTROL_COMMAND_SSB_OFF:
		return "ssb-off";
	case CP_CONTROL_COMMAND_SSB_PRESET:
		return "ssb-preset";
	default:
		return "unknown";
	}
}

const char *
cp_control_bank_string(enum cp_control_bank bank)
{
	switch (bank) {
	case CP_CONTROL_BANK_AM:
		return "AM";
	case CP_CONTROL_BANK_SSB:
		return "SSB";
	default:
		return "unknown";
	}
}

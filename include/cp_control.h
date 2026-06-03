/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_control.h */

#ifndef CP_CONTROL_H
#define CP_CONTROL_H

#include "cp_am.h"
#include "cp_block.h"
#include "cp_ssb.h"

enum cp_control_bank {
	CP_CONTROL_BANK_AM = 0,
	CP_CONTROL_BANK_SSB
};

enum cp_control_command_type {
	CP_CONTROL_COMMAND_NONE = 0,
	CP_CONTROL_COMMAND_STOP,
	CP_CONTROL_COMMAND_PLAYOUT_NEXT,
	CP_CONTROL_COMMAND_DEHUMMER_TOGGLE,
	CP_CONTROL_COMMAND_MULTIBAND_CYCLE,
	CP_CONTROL_COMMAND_SELECT_AM,
	CP_CONTROL_COMMAND_SELECT_SSB,
	CP_CONTROL_COMMAND_AM_OFF,
	CP_CONTROL_COMMAND_AM_PRESET,
	CP_CONTROL_COMMAND_SSB_OFF,
	CP_CONTROL_COMMAND_SSB_PRESET
};

struct cp_control_command {
	enum cp_control_command_type type;
	enum cp_control_bank bank;
	enum cp_am_preset am_preset;
	enum cp_ssb_preset ssb_preset;
};

int		cp_control_apply(struct cp_block_processor *,
		    const struct cp_control_command *);
void		cp_control_command_clear(struct cp_control_command *);
int		cp_control_command_from_key(int, enum cp_control_bank,
		    struct cp_control_command *);
const char	*cp_control_command_string(
		    enum cp_control_command_type);
const char	*cp_control_bank_string(enum cp_control_bank);

#endif

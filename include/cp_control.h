/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_control.h */

#ifndef CP_CONTROL_H
#define CP_CONTROL_H

#include "cp_am.h"
#include "cp_block.h"

enum cp_control_command_type {
	CP_CONTROL_COMMAND_NONE = 0,
	CP_CONTROL_COMMAND_STOP,
	CP_CONTROL_COMMAND_AM_OFF,
	CP_CONTROL_COMMAND_AM_PRESET
};

struct cp_control_command {
	enum cp_control_command_type type;
	enum cp_am_preset am_preset;
};

int		cp_control_apply(struct cp_block_processor *,
		    const struct cp_control_command *);
void		cp_control_command_clear(struct cp_control_command *);
int		cp_control_command_from_key(int, struct cp_control_command *);
const char	*cp_control_command_string(
		    enum cp_control_command_type);

#endif

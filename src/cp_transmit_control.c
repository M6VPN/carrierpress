/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_transmit_control.c */

#include <sys/types.h>

#include <stddef.h>

#include "cp_transmit_control.h"

int
cp_tx_control_available(void)
{
#ifdef CP_WITH_TRANSMIT_CONTROL
	return 1;
#else
	return 0;
#endif
}

int
cp_tx_control_arm(struct cp_tx_control *control)
{
	if (control == NULL)
		return CP_TX_ERR_NULL;
	if (!cp_tx_control_available())
		return CP_TX_ERR_DISABLED;

	switch (control->state) {
	case CP_TX_STATE_DISARMED:
		control->state = CP_TX_STATE_ARMED_RX;
		return CP_TX_OK;
	case CP_TX_STATE_ARMED_RX:
		return CP_TX_OK;
	case CP_TX_STATE_DISABLED:
		return CP_TX_ERR_DISABLED;
	case CP_TX_STATE_FAULT:
		return CP_TX_ERR_INVALID_STATE;
	default:
		return CP_TX_ERR_INVALID_STATE;
	}
}

int
cp_tx_control_disarm(struct cp_tx_control *control)
{
	if (control == NULL)
		return CP_TX_ERR_NULL;
	if (!cp_tx_control_available())
		return CP_TX_ERR_DISABLED;

	switch (control->state) {
	case CP_TX_STATE_DISABLED:
		return CP_TX_ERR_DISABLED;
	case CP_TX_STATE_FAULT:
		return CP_TX_ERR_INVALID_STATE;
	default:
		control->state = CP_TX_STATE_DISARMED;
		return CP_TX_OK;
	}
}

int
cp_tx_control_emergency_rx(struct cp_tx_control *control)
{
	if (control == NULL)
		return CP_TX_ERR_NULL;
	if (!cp_tx_control_available())
		return CP_TX_ERR_DISABLED;

	switch (control->state) {
	case CP_TX_STATE_DISARMED:
		return CP_TX_OK;
	case CP_TX_STATE_ARMED_RX:
	case CP_TX_STATE_TX_REQUESTED:
	case CP_TX_STATE_TX_ACTIVE:
	case CP_TX_STATE_RX_REQUESTED:
		control->state = CP_TX_STATE_DISARMED;
		return CP_TX_OK;
	case CP_TX_STATE_DISABLED:
		return CP_TX_ERR_DISABLED;
	case CP_TX_STATE_FAULT:
	default:
		return CP_TX_ERR_INVALID_STATE;
	}
}

void
cp_tx_control_init(struct cp_tx_control *control)
{
	if (control == NULL)
		return;

	control->compile_time_enabled = cp_tx_control_available();
	if (control->compile_time_enabled)
		control->state = CP_TX_STATE_DISARMED;
	else
		control->state = CP_TX_STATE_DISABLED;
}

int
cp_tx_control_is_armed(const struct cp_tx_control *control)
{
	if (control == NULL)
		return 0;

	switch (control->state) {
	case CP_TX_STATE_ARMED_RX:
	case CP_TX_STATE_TX_REQUESTED:
	case CP_TX_STATE_TX_ACTIVE:
	case CP_TX_STATE_RX_REQUESTED:
		return 1;
	default:
		return 0;
	}
}

int
cp_tx_control_is_tx_active(const struct cp_tx_control *control)
{
	return control != NULL && control->state == CP_TX_STATE_TX_ACTIVE;
}

int
cp_tx_control_mock_step(struct cp_tx_control *control)
{
	if (control == NULL)
		return CP_TX_ERR_NULL;
	if (!cp_tx_control_available())
		return CP_TX_ERR_DISABLED;

	switch (control->state) {
	case CP_TX_STATE_TX_REQUESTED:
		control->state = CP_TX_STATE_TX_ACTIVE;
		return CP_TX_OK;
	case CP_TX_STATE_RX_REQUESTED:
		control->state = CP_TX_STATE_ARMED_RX;
		return CP_TX_OK;
	case CP_TX_STATE_DISARMED:
	case CP_TX_STATE_ARMED_RX:
	case CP_TX_STATE_TX_ACTIVE:
		return CP_TX_OK;
	case CP_TX_STATE_DISABLED:
		return CP_TX_ERR_DISABLED;
	case CP_TX_STATE_FAULT:
	default:
		return CP_TX_ERR_INVALID_STATE;
	}
}

int
cp_tx_control_request_rx(struct cp_tx_control *control)
{
	if (control == NULL)
		return CP_TX_ERR_NULL;
	if (!cp_tx_control_available())
		return CP_TX_ERR_DISABLED;

	switch (control->state) {
	case CP_TX_STATE_TX_REQUESTED:
	case CP_TX_STATE_TX_ACTIVE:
		control->state = CP_TX_STATE_RX_REQUESTED;
		return CP_TX_OK;
	case CP_TX_STATE_ARMED_RX:
	case CP_TX_STATE_RX_REQUESTED:
		return CP_TX_OK;
	case CP_TX_STATE_DISARMED:
		return CP_TX_ERR_DISARMED;
	case CP_TX_STATE_DISABLED:
		return CP_TX_ERR_DISABLED;
	case CP_TX_STATE_FAULT:
	default:
		return CP_TX_ERR_INVALID_STATE;
	}
}

int
cp_tx_control_request_transmit(struct cp_tx_control *control)
{
	if (control == NULL)
		return CP_TX_ERR_NULL;
	if (!cp_tx_control_available())
		return CP_TX_ERR_DISABLED;

	switch (control->state) {
	case CP_TX_STATE_ARMED_RX:
		control->state = CP_TX_STATE_TX_REQUESTED;
		return CP_TX_OK;
	case CP_TX_STATE_TX_REQUESTED:
	case CP_TX_STATE_TX_ACTIVE:
		return CP_TX_OK;
	case CP_TX_STATE_DISARMED:
	case CP_TX_STATE_RX_REQUESTED:
		return CP_TX_ERR_DISARMED;
	case CP_TX_STATE_DISABLED:
		return CP_TX_ERR_DISABLED;
	case CP_TX_STATE_FAULT:
	default:
		return CP_TX_ERR_INVALID_STATE;
	}
}

enum cp_tx_state
cp_tx_control_state(const struct cp_tx_control *control)
{
	if (control == NULL)
		return CP_TX_STATE_DISABLED;

	return control->state;
}

const char *
cp_tx_state_string(enum cp_tx_state state)
{
	switch (state) {
	case CP_TX_STATE_DISABLED:
		return "disabled";
	case CP_TX_STATE_DISARMED:
		return "disarmed";
	case CP_TX_STATE_ARMED_RX:
		return "armed_rx";
	case CP_TX_STATE_TX_REQUESTED:
		return "tx_requested";
	case CP_TX_STATE_TX_ACTIVE:
		return "tx_active";
	case CP_TX_STATE_RX_REQUESTED:
		return "rx_requested";
	case CP_TX_STATE_FAULT:
		return "fault";
	default:
		return "unknown";
	}
}

const char *
cp_tx_status_string(int status)
{
	switch (status) {
	case CP_TX_OK:
		return "ok";
	case CP_TX_ERR_NULL:
		return "null";
	case CP_TX_ERR_DISABLED:
		return "disabled";
	case CP_TX_ERR_DISARMED:
		return "disarmed";
	case CP_TX_ERR_UNSUPPORTED:
		return "unsupported";
	case CP_TX_ERR_INVALID_STATE:
		return "invalid_state";
	default:
		return "unknown";
	}
}

#ifdef CP_WITH_TRANSMIT_CONTROL
int
cp_tx_operator_command_apply(struct cp_tx_control *control,
	enum cp_tx_operator_command command)
{
	switch (command) {
	case CP_TX_OPERATOR_ARM:
		return cp_tx_control_arm(control);
	case CP_TX_OPERATOR_DISARM:
		return cp_tx_control_disarm(control);
	case CP_TX_OPERATOR_NONE:
		return CP_TX_OK;
	default:
		return CP_TX_ERR_INVALID_STATE;
	}
}

int
cp_tx_operator_command_from_key(int key,
	enum cp_tx_operator_command *command)
{
	if (command == NULL)
		return CP_TX_ERR_NULL;

	*command = CP_TX_OPERATOR_NONE;
	switch (key) {
	case 'r':
		*command = CP_TX_OPERATOR_ARM;
		return CP_TX_OK;
	case 'u':
		*command = CP_TX_OPERATOR_DISARM;
		return CP_TX_OK;
	default:
		return CP_TX_ERR_UNSUPPORTED;
	}
}

const char *
cp_tx_operator_command_string(enum cp_tx_operator_command command)
{
	switch (command) {
	case CP_TX_OPERATOR_NONE:
		return "none";
	case CP_TX_OPERATOR_ARM:
		return "mock_arm";
	case CP_TX_OPERATOR_DISARM:
		return "mock_disarm";
	default:
		return "unknown";
	}
}
#endif

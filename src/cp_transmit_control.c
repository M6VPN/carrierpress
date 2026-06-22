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

void
cp_tx_control_init(struct cp_tx_control *control)
{
	if (control == NULL)
		return;

	control->state = CP_TX_STATE_DISABLED;
	control->compile_time_enabled = cp_tx_control_available();
}

int
cp_tx_control_request_transmit(struct cp_tx_control *control)
{
	if (control == NULL)
		return CP_TX_ERR_NULL;

	control->state = CP_TX_STATE_DISABLED;
	if (!cp_tx_control_available())
		return CP_TX_ERR_DISABLED;

	return CP_TX_ERR_UNSUPPORTED;
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
	default:
		return "unknown";
	}
}

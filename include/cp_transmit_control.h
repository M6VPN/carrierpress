/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_transmit_control.h */

#ifndef CP_TRANSMIT_CONTROL_H
#define CP_TRANSMIT_CONTROL_H

enum cp_tx_status {
	CP_TX_OK                = 0,
	CP_TX_ERR_NULL          = -1100,
	CP_TX_ERR_DISABLED      = -1101,
	CP_TX_ERR_DISARMED      = -1102,
	CP_TX_ERR_UNSUPPORTED   = -1103,
	CP_TX_ERR_INVALID_STATE = -1104
};

enum cp_tx_state {
	CP_TX_STATE_DISABLED = 0,
	CP_TX_STATE_DISARMED,
	CP_TX_STATE_ARMED_RX,
	CP_TX_STATE_TX_REQUESTED,
	CP_TX_STATE_TX_ACTIVE,
	CP_TX_STATE_RX_REQUESTED,
	CP_TX_STATE_FAULT
};

struct cp_tx_control {
	enum cp_tx_state state;
	int compile_time_enabled;
};

int			cp_tx_control_available(void);
int			cp_tx_control_arm(struct cp_tx_control *);
int			cp_tx_control_disarm(struct cp_tx_control *);
int			cp_tx_control_emergency_rx(struct cp_tx_control *);
void			cp_tx_control_init(struct cp_tx_control *);
int			cp_tx_control_is_armed(
			    const struct cp_tx_control *);
int			cp_tx_control_is_tx_active(
			    const struct cp_tx_control *);
int			cp_tx_control_mock_step(struct cp_tx_control *);
int			cp_tx_control_request_rx(struct cp_tx_control *);
int			cp_tx_control_request_transmit(
			    struct cp_tx_control *);
enum cp_tx_state	cp_tx_control_state(const struct cp_tx_control *);
const char		*cp_tx_state_string(enum cp_tx_state);
const char		*cp_tx_status_string(int);

#endif

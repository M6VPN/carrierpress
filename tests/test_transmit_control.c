/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_transmit_control.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_transmit_control.h"

static int	text_has_backend_name(const char *);
static int	test_availability(void);
static int	test_guarded_emergency_rx(void);
static int	test_guarded_state_machine(void);
static int	test_initial_state(void);
static int	test_null_inputs(void);
static int	test_ordinary_calls_disabled(void);
static int	test_strings(void);

int
main(void)
{
	if (!test_availability())
		return 1;
	if (!test_initial_state())
		return 1;
	if (!test_null_inputs())
		return 1;
	if (!test_ordinary_calls_disabled())
		return 1;
	if (!test_guarded_state_machine())
		return 1;
	if (!test_guarded_emergency_rx())
		return 1;
	if (!test_strings())
		return 1;

	return 0;
}

static int
text_has_backend_name(const char *text)
{
	if (text == NULL)
		return 1;

	return strstr(text, "hamlib") != NULL ||
	    strstr(text, "flrig") != NULL ||
	    strstr(text, "cat_ptt") != NULL ||
	    strstr(text, "rig_frequency") != NULL ||
	    strstr(text, "rig_mode") != NULL;
}

static int
test_availability(void)
{
#ifdef CP_WITH_TRANSMIT_CONTROL
	if (!cp_tx_control_available()) {
		printf("test_transmit_control: guarded scaffold unavailable\n");
		return 0;
	}
#else
	if (cp_tx_control_available()) {
		printf("test_transmit_control: ordinary build available\n");
		return 0;
	}
#endif

	return 1;
}

static int
test_initial_state(void)
{
	struct cp_tx_control control;

	cp_tx_control_init(NULL);
	cp_tx_control_init(&control);
	if (control.compile_time_enabled != cp_tx_control_available()) {
		printf("test_transmit_control: compile flag mismatch\n");
		return 0;
	}
#ifdef CP_WITH_TRANSMIT_CONTROL
	if (control.state != CP_TX_STATE_DISARMED ||
	    cp_tx_control_state(&control) != CP_TX_STATE_DISARMED) {
		printf("test_transmit_control: guarded state is not disarmed\n");
		return 0;
	}
#else
	if (control.state != CP_TX_STATE_DISABLED ||
	    cp_tx_control_state(&control) != CP_TX_STATE_DISABLED) {
		printf("test_transmit_control: initial state is not disabled\n");
		return 0;
	}
#endif
	if (cp_tx_control_state(NULL) != CP_TX_STATE_DISABLED) {
		printf("test_transmit_control: null state is not disabled\n");
		return 0;
	}
	if (cp_tx_control_is_armed(&control)) {
		printf("test_transmit_control: initial state armed\n");
		return 0;
	}
	if (cp_tx_control_is_tx_active(&control)) {
		printf("test_transmit_control: initial state active\n");
		return 0;
	}

	return 1;
}

static int
test_null_inputs(void)
{
	if (cp_tx_control_arm(NULL) != CP_TX_ERR_NULL) {
		printf("test_transmit_control: null arm accepted\n");
		return 0;
	}
	if (cp_tx_control_disarm(NULL) != CP_TX_ERR_NULL) {
		printf("test_transmit_control: null disarm accepted\n");
		return 0;
	}
	if (cp_tx_control_emergency_rx(NULL) != CP_TX_ERR_NULL) {
		printf("test_transmit_control: null emergency accepted\n");
		return 0;
	}
	if (cp_tx_control_mock_step(NULL) != CP_TX_ERR_NULL) {
		printf("test_transmit_control: null mock step accepted\n");
		return 0;
	}
	if (cp_tx_control_request_rx(NULL) != CP_TX_ERR_NULL) {
		printf("test_transmit_control: null rx accepted\n");
		return 0;
	}
	if (cp_tx_control_request_transmit(NULL) != CP_TX_ERR_NULL) {
		printf("test_transmit_control: null request accepted\n");
		return 0;
	}
	if (cp_tx_control_is_armed(NULL) || cp_tx_control_is_tx_active(NULL)) {
		printf("test_transmit_control: null state predicate failed\n");
		return 0;
	}

	return 1;
}

static int
test_ordinary_calls_disabled(void)
{
#ifndef CP_WITH_TRANSMIT_CONTROL
	struct cp_tx_control control;

	cp_tx_control_init(&control);
	if (cp_tx_control_arm(&control) != CP_TX_ERR_DISABLED ||
	    cp_tx_control_disarm(&control) != CP_TX_ERR_DISABLED ||
	    cp_tx_control_emergency_rx(&control) != CP_TX_ERR_DISABLED ||
	    cp_tx_control_request_transmit(&control) != CP_TX_ERR_DISABLED ||
	    cp_tx_control_request_rx(&control) != CP_TX_ERR_DISABLED ||
	    cp_tx_control_mock_step(&control) != CP_TX_ERR_DISABLED) {
		printf("test_transmit_control: ordinary call not disabled\n");
		return 0;
	}
	if (control.state != CP_TX_STATE_DISABLED ||
	    cp_tx_control_is_armed(&control) ||
	    cp_tx_control_is_tx_active(&control)) {
		printf("test_transmit_control: ordinary call changed state\n");
		return 0;
	}
#endif

	return 1;
}

static int
test_guarded_emergency_rx(void)
{
#ifdef CP_WITH_TRANSMIT_CONTROL
	struct cp_tx_control control;

	cp_tx_control_init(&control);
	if (cp_tx_control_emergency_rx(&control) != CP_TX_OK ||
	    control.state != CP_TX_STATE_DISARMED ||
	    cp_tx_control_is_armed(&control) ||
	    cp_tx_control_is_tx_active(&control)) {
		printf("test_transmit_control: disarmed emergency failed\n");
		return 0;
	}
	if (cp_tx_control_arm(&control) != CP_TX_OK ||
	    cp_tx_control_emergency_rx(&control) != CP_TX_OK ||
	    control.state != CP_TX_STATE_DISARMED ||
	    cp_tx_control_is_armed(&control) ||
	    cp_tx_control_is_tx_active(&control)) {
		printf("test_transmit_control: armed emergency failed\n");
		return 0;
	}
	if (cp_tx_control_arm(&control) != CP_TX_OK ||
	    cp_tx_control_request_transmit(&control) != CP_TX_OK ||
	    cp_tx_control_emergency_rx(&control) != CP_TX_OK ||
	    control.state != CP_TX_STATE_DISARMED ||
	    cp_tx_control_request_transmit(&control) != CP_TX_ERR_DISARMED) {
		printf("test_transmit_control: requested emergency failed\n");
		return 0;
	}
	if (cp_tx_control_arm(&control) != CP_TX_OK ||
	    cp_tx_control_request_transmit(&control) != CP_TX_OK ||
	    cp_tx_control_mock_step(&control) != CP_TX_OK ||
	    control.state != CP_TX_STATE_TX_ACTIVE ||
	    cp_tx_control_emergency_rx(&control) != CP_TX_OK ||
	    control.state != CP_TX_STATE_DISARMED ||
	    cp_tx_control_is_armed(&control) ||
	    cp_tx_control_is_tx_active(&control)) {
		printf("test_transmit_control: active emergency failed\n");
		return 0;
	}
	if (cp_tx_control_arm(&control) != CP_TX_OK ||
	    cp_tx_control_request_transmit(&control) != CP_TX_OK ||
	    cp_tx_control_request_rx(&control) != CP_TX_OK ||
	    control.state != CP_TX_STATE_RX_REQUESTED ||
	    cp_tx_control_emergency_rx(&control) != CP_TX_OK ||
	    control.state != CP_TX_STATE_DISARMED ||
	    cp_tx_control_request_transmit(&control) != CP_TX_ERR_DISARMED) {
		printf("test_transmit_control: rx emergency failed\n");
		return 0;
	}
	control.state = CP_TX_STATE_FAULT;
	if (cp_tx_control_emergency_rx(&control) != CP_TX_ERR_INVALID_STATE ||
	    cp_tx_control_is_tx_active(&control)) {
		printf("test_transmit_control: fault emergency failed\n");
		return 0;
	}
#endif

	return 1;
}

static int
test_guarded_state_machine(void)
{
#ifdef CP_WITH_TRANSMIT_CONTROL
	struct cp_tx_control control;

	cp_tx_control_init(&control);
	if (cp_tx_control_request_transmit(&control) != CP_TX_ERR_DISARMED ||
	    cp_tx_control_request_rx(&control) != CP_TX_ERR_DISARMED) {
		printf("test_transmit_control: disarmed request accepted\n");
		return 0;
	}
	if (cp_tx_control_arm(&control) != CP_TX_OK ||
	    control.state != CP_TX_STATE_ARMED_RX ||
	    !cp_tx_control_is_armed(&control) ||
	    cp_tx_control_is_tx_active(&control)) {
		printf("test_transmit_control: arm failed\n");
		return 0;
	}
	if (cp_tx_control_request_transmit(&control) != CP_TX_OK ||
	    control.state != CP_TX_STATE_TX_REQUESTED ||
	    !cp_tx_control_is_armed(&control) ||
	    cp_tx_control_is_tx_active(&control)) {
		printf("test_transmit_control: transmit request failed\n");
		return 0;
	}
	if (cp_tx_control_mock_step(&control) != CP_TX_OK ||
	    control.state != CP_TX_STATE_TX_ACTIVE ||
	    !cp_tx_control_is_tx_active(&control)) {
		printf("test_transmit_control: mock tx step failed\n");
		return 0;
	}
	if (cp_tx_control_request_rx(&control) != CP_TX_OK ||
	    control.state != CP_TX_STATE_RX_REQUESTED ||
	    cp_tx_control_is_tx_active(&control)) {
		printf("test_transmit_control: rx request failed\n");
		return 0;
	}
	if (cp_tx_control_mock_step(&control) != CP_TX_OK ||
	    control.state != CP_TX_STATE_ARMED_RX ||
	    !cp_tx_control_is_armed(&control) ||
	    cp_tx_control_is_tx_active(&control)) {
		printf("test_transmit_control: mock rx step failed\n");
		return 0;
	}
	if (cp_tx_control_request_transmit(&control) != CP_TX_OK ||
	    cp_tx_control_disarm(&control) != CP_TX_OK ||
	    control.state != CP_TX_STATE_DISARMED ||
	    cp_tx_control_is_armed(&control) ||
	    cp_tx_control_is_tx_active(&control)) {
		printf("test_transmit_control: disarm failed\n");
		return 0;
	}
#endif

	return 1;
}

static int
test_strings(void)
{
	const char *strings[] = {
		cp_tx_state_string(CP_TX_STATE_DISABLED),
		cp_tx_state_string(CP_TX_STATE_DISARMED),
		cp_tx_state_string(CP_TX_STATE_ARMED_RX),
		cp_tx_state_string(CP_TX_STATE_TX_REQUESTED),
		cp_tx_state_string(CP_TX_STATE_TX_ACTIVE),
		cp_tx_state_string(CP_TX_STATE_RX_REQUESTED),
		cp_tx_state_string(CP_TX_STATE_FAULT),
		cp_tx_state_string((enum cp_tx_state)99),
		cp_tx_status_string(CP_TX_OK),
		cp_tx_status_string(CP_TX_ERR_NULL),
		cp_tx_status_string(CP_TX_ERR_DISABLED),
		cp_tx_status_string(CP_TX_ERR_DISARMED),
		cp_tx_status_string(CP_TX_ERR_UNSUPPORTED),
		cp_tx_status_string(CP_TX_ERR_INVALID_STATE),
		cp_tx_status_string(99)
	};
	size_t index;

	if (strcmp(cp_tx_state_string(CP_TX_STATE_DISABLED),
	    "disabled") != 0 ||
	    strcmp(cp_tx_state_string(CP_TX_STATE_TX_ACTIVE),
	    "tx_active") != 0 ||
	    strcmp(cp_tx_status_string(CP_TX_ERR_UNSUPPORTED),
	    "unsupported") != 0 ||
	    strcmp(cp_tx_status_string(CP_TX_ERR_INVALID_STATE),
	    "invalid_state") != 0) {
		printf("test_transmit_control: string mapping failed\n");
		return 0;
	}
	for (index = 0; index < sizeof(strings) / sizeof(strings[0]);
	    index++) {
		if (text_has_backend_name(strings[index])) {
			printf("test_transmit_control: backend text leaked\n");
			return 0;
		}
	}

	return 1;
}

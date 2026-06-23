#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/scripts/operator-workflow-safety-audit.sh

set -eu

failed=0

fail()
{
	printf '%s\n' "$1" >&2
	failed=1
}

check_no_matches()
{
	description=$1
	pattern=$2
	shift 2

	if matches=$(grep -nE "$pattern" "$@" 2>/dev/null); then
		fail "operator-workflow-safety-audit: ${description}"
		printf '%s\n' "$matches" >&2
	fi
}

check_find_no_matches()
{
	description=$1
	pattern=$2
	shift 2

	if matches=$(find "$@" -type f -exec grep -nE "$pattern" {} + \
	    2>/dev/null); then
		fail "operator-workflow-safety-audit: ${description}"
		printf '%s\n' "$matches" >&2
	fi
}

check_example_scripts_no_matches()
{
	description=$1
	pattern=$2

	if matches=$(find examples -type f -name '*.sh' \
	    -exec grep -nE "$pattern" {} + 2>/dev/null); then
		fail "operator-workflow-safety-audit: ${description}"
		printf '%s\n' "$matches" >&2
	fi
}

check_no_matches \
	"ordinary GUI/TUI/control workflow exposes active transmit controls" \
	'(^|[^A-Za-z0-9_])(request_transmit|tx_active|tx_requested|TRANSMIT|PTT)([^A-Za-z0-9_]|$)|set_ptt|ptt_on|ptt_off|rig_set' \
	include/cp_control.h src/cp_control.c \
	include/cp_gui_workflow.h src/cp_gui_workflow.c \
	src/cp_gui_format.c src/cp_gui_sdl3.c src/cp_tui.c

check_no_matches \
	"operator display paths expose state-changing transmit-control calls" \
	'cp_tx_control_(arm|disarm|request_transmit|request_rx|emergency_rx|mock_step)[[:space:]]*\(' \
	include/cp_gui.h include/cp_tui.h include/cp_gui_format.h \
	src/cp_gui_format.c src/cp_gui_sdl3.c src/cp_tui.c

check_example_scripts_no_matches \
	"examples contain release publication, package install, or sudo actions" \
	'sudo|apt-get|apt install|dnf install|yum install|pacman|pkg_add|brew install|git tag|git push|gh release|upload-artifact'

check_example_scripts_no_matches \
	"examples contain transmit-control or backend control calls" \
	'cp_tx_control_|cp_transmit_control\.h|request_transmit|tx_active|tx_requested|set_ptt|ptt_on|ptt_off|rig_set_(ptt|freq|frequency|mode|vfo)|set_freq|set_frequency|set_mode'

if [ "$failed" -ne 0 ]; then
	exit 1
fi

printf 'operator-workflow-safety-audit: status=pass\n'

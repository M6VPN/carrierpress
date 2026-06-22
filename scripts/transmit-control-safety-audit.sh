#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/scripts/transmit-control-safety-audit.sh

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
		fail "transmit-control-safety-audit: ${description}"
		printf '%s\n' "$matches" >&2
	fi
}

check_find_no_matches()
{
	description=$1
	pattern=$2
	shift 2

	if matches=$(find "$@" -type f \( -name '*.c' -o -name '*.h' \) \
	    -exec grep -nE "$pattern" {} + 2>/dev/null); then
		fail "transmit-control-safety-audit: ${description}"
		printf '%s\n' "$matches" >&2
	fi
}

check_tx_symbols_outside_boundary()
{
	if matches=$(find include src tests examples -type f \
	    \( -name '*.c' -o -name '*.h' \) \
	    ! -path 'include/cp_transmit_control.h' \
	    ! -path 'src/cp_transmit_control.c' \
	    ! -path 'tests/test_transmit_control.c' \
	    -exec grep -nE 'cp_tx_control_|cp_transmit_control\.h' {} + \
	    2>/dev/null); then
		fail "transmit-control-safety-audit: transmit-control API used outside guarded boundary"
		printf '%s\n' "$matches" >&2
	fi
}

check_tx_symbols_outside_boundary

check_no_matches \
	"transmit-control implementation references backend/control names" \
	'hamlib|flrig|cat_ptt|rig_frequency|rig_mode|ptt_on|ptt_off|set_ptt|rig_set' \
	include/cp_transmit_control.h src/cp_transmit_control.c

check_no_matches \
	"ordinary GUI/TUI/control workflow exposes active transmit terms" \
	'cp_tx_control_|cp_transmit_control\.h|request_transmit|tx_active|tx_requested|TRANSMIT|PTT' \
	include/cp_control.h src/cp_control.c \
	include/cp_gui_workflow.h src/cp_gui_workflow.c \
	src/cp_gui_sdl3.c src/cp_tui.c

check_no_matches \
	"host/audio/report/batch/config/profile paths reference transmit-control API" \
	'cp_tx_control_|cp_transmit_control\.h|request_transmit|tx_active|tx_requested' \
	src/main.c src/cp_audio.c src/cp_playout.c src/cp_report.c \
	src/cp_batch.c src/cp_config_file.c src/cp_profile.c

check_find_no_matches \
	"CAT backend contains write-control style symbols" \
	'set_ptt|ptt_on|ptt_off|rig_set_(ptt|freq|frequency|mode|vfo)|set_freq|set_frequency|set_mode' \
	src include

if [ "$failed" -ne 0 ]; then
	exit 1
fi

printf 'transmit-control-safety-audit: status=pass\n'

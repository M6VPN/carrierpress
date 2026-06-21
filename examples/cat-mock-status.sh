#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/cat-mock-status.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make first.'
	exit 1
fi

printf '%s\n' 'CAT mock status is read-only. No PTT or rig-control command is sent.'
./carrierpress --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off --cat-status

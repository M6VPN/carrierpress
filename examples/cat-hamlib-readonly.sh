#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/cat-hamlib-readonly.sh

set -eu

CAT_RIG_MODEL=${CAT_RIG_MODEL:-1}

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make WITH_HAMLIB=1 first.'
	exit 1
fi

printf '%s\n' 'hamlib CAT status is read-only. No PTT or rig-control command is sent.'
printf 'using hamlib rig model: %s\n' "$CAT_RIG_MODEL"
./carrierpress --cat-backend hamlib --cat-rig-model "$CAT_RIG_MODEL" --cat-status

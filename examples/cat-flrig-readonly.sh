#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/cat-flrig-readonly.sh

set -eu

CAT_HOST=${CAT_HOST:-127.0.0.1}
CAT_PORT=${CAT_PORT:-12345}

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make WITH_FLRIG=1 first.'
	exit 1
fi

printf '%s\n' 'flrig CAT status is read-only. No PTT or rig-control command is sent.'
printf 'using flrig endpoint: %s:%s\n' "$CAT_HOST" "$CAT_PORT"
./carrierpress --cat-backend flrig --cat-host "$CAT_HOST" --cat-port "$CAT_PORT" --cat-status

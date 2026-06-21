#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/validate-config.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make first.'
	exit 1
fi

./carrierpress --validate-config configs/default.conf
./carrierpress --validate-config configs/live-pulse.conf
./carrierpress --validate-config configs/gui-demo.conf
./carrierpress --validate-config configs/playout.conf

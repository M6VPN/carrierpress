#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/config-gui-demo.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make WITH_GUI=1 first.'
	exit 1
fi

./carrierpress --config configs/gui-demo.conf --gui-demo \
	--cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB \
	--cat-ptt off

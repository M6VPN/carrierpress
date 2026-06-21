#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/print-effective-config.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make first.'
	exit 1
fi

./carrierpress --profile profiles/am-safe.profile --print-effective-config
./carrierpress --config configs/default.conf --print-effective-config
./carrierpress --config configs/default.conf \
	--profile profiles/ssb-speech.profile \
	--print-effective-config

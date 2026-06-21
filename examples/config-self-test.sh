#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/config-self-test.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make first.'
	exit 1
fi

./carrierpress --config configs/default.conf --self-test
./carrierpress --config configs/live-pulse.conf --self-test
./carrierpress --config configs/gui-demo.conf --self-test
./carrierpress --config configs/default.conf --sample-rate 44100 --self-test
./carrierpress --config configs/default.conf --profile profiles/ssb-speech.profile --self-test

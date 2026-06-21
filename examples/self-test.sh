#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/self-test.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make first.'
	exit 1
fi

./carrierpress --self-test
./carrierpress --self-test --dehummer --multiband --multiband-bands 3 --am --am-preset am-safe
./carrierpress --self-test --ssb --ssb-preset ssb-speech

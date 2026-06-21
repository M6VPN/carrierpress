#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/profile-self-test.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make first.'
	exit 1
fi

./carrierpress --profile profiles/am-safe.profile --self-test
./carrierpress --profile profiles/am-shortwave.profile --self-test
./carrierpress --profile profiles/ssb-speech.profile --self-test
./carrierpress --profile profiles/file-cleanup.profile --self-test
./carrierpress --profile profiles/am-safe.profile --am-preset am-voice --self-test
./carrierpress --profile profiles/ssb-speech.profile --ssb-preset ssb-narrow --self-test

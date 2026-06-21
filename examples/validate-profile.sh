#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/validate-profile.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make first.'
	exit 1
fi

./carrierpress --validate-profile profiles/am-safe.profile
./carrierpress --validate-profile profiles/am-shortwave.profile
./carrierpress --validate-profile profiles/ssb-speech.profile
./carrierpress --validate-profile profiles/file-cleanup.profile

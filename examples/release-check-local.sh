#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/release-check-local.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'note: ./carrierpress not found. make release-check will build it.'
fi

printf '%s\n' 'Running local validation only. No tag, push, or GitHub release is created.'
make release-check

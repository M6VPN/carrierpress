#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/report-summary.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf 'error: ./carrierpress not found. Run make first.\n' >&2
	exit 1
fi

if [ "$#" -ne 1 ]; then
	printf 'usage: %s report.json\n' "$0"
	exit 0
fi

./carrierpress --report-summary "$1"

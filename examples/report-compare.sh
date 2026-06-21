#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/report-compare.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf 'error: ./carrierpress not found. Run make first.\n' >&2
	exit 1
fi

if [ "$#" -ne 2 ] && [ "$#" -ne 3 ]; then
	printf 'usage: %s base.json new.json [tolerance]\n' "$0"
	exit 0
fi

if [ "$#" -eq 3 ]; then
	./carrierpress --report-compare "$1" "$2" --report-tolerance "$3"
else
	./carrierpress --report-compare "$1" "$2"
fi

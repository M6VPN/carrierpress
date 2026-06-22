#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/batch-report-compare.sh

set -eu

if [ "$#" -ne 2 ]; then
	echo "usage: $0 OLD_BATCH_SUMMARY.json NEW_BATCH_SUMMARY.json"
	exit 0
fi

if [ ! -x ./carrierpress ]; then
	echo "error: ./carrierpress not found; run make first" >&2
	exit 1
fi

./carrierpress --report-compare "$1" "$2"

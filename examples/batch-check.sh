#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/batch-check.sh

set -eu

if [ ! -x ./carrierpress ]; then
	echo "error: build ./carrierpress first"
	exit 1
fi

./carrierpress --batch-check examples/batch-list.txt \
	--batch-output-dir build/batch-out

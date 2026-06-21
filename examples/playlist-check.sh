#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/playlist-check.sh

set -eu

if [ ! -x ./carrierpress ]; then
	echo "error: build carrierpress first with: make"
	exit 1
fi

./carrierpress --playlist-check examples/playout-playlist.txt

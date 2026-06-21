#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/gui-cue-demo.sh

set -eu

if [ "$#" -ne 2 ]; then
	printf '%s\n' 'usage: ./examples/gui-cue-demo.sh cue.wav playlist.txt'
	exit 0
fi

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make WITH_GUI=1 first.'
	exit 1
fi

./carrierpress --gui-demo --gui-cue-wav "$1" --gui-cue-playlist "$2" \
	--cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB \
	--cat-ptt off

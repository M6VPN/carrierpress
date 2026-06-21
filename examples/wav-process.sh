#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/wav-process.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make WITH_SNDFILE=1 first.'
	exit 1
fi

if [ "$#" -ne 2 ]; then
	printf '%s\n' 'usage: ./examples/wav-process.sh input.wav output.wav'
	exit 1
fi

if [ ! -f "$1" ]; then
	printf 'error: input WAV not found: %s\n' "$1"
	exit 1
fi

./carrierpress --input "$1" --output "$2" --dehummer --multiband --multiband-bands 3

#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/profile-wav-process.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make WITH_SNDFILE=1 first.'
	exit 1
fi

if [ "$#" -ne 3 ]; then
	printf '%s\n' 'usage: ./examples/profile-wav-process.sh profile input.wav output.wav'
	exit 1
fi

if [ ! -f "$1" ]; then
	printf 'error: profile not found: %s\n' "$1"
	exit 1
fi

if [ ! -f "$2" ]; then
	printf 'error: input WAV not found: %s\n' "$2"
	exit 1
fi

./carrierpress --profile "$1" --input "$2" --output "$3"

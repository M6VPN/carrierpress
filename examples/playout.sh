#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/playout.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make WITH_SNDFILE=1 WITH_PORTAUDIO=1 first.'
	exit 1
fi

if [ "$#" -ne 1 ]; then
	printf '%s\n' 'usage: ./examples/playout.sh input.wav'
	exit 1
fi

if [ ! -f "$1" ]; then
	printf 'error: input WAV not found: %s\n' "$1"
	exit 1
fi

./carrierpress --play "$1" --meter-interval-ms 1000

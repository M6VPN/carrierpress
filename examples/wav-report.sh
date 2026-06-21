#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/wav-report.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make WITH_SNDFILE=1 first.'
	exit 1
fi

if [ "$#" -ne 3 ]; then
	printf '%s\n' 'usage: ./examples/wav-report.sh input.wav output.wav report.json'
	exit 1
fi

if [ ! -f "$1" ]; then
	printf 'error: input WAV not found: %s\n' "$1"
	exit 1
fi

if [ "$1" = "$2" ]; then
	printf '%s\n' 'error: output path must not match input path'
	exit 1
fi

./carrierpress --input "$1" --output "$2" --report "$3"

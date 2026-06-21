#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/playlist-workflow.sh

set -eu

usage()
{
	printf '%s\n' 'usage: ./examples/playlist-workflow.sh playlist.txt'
	printf '%s\n' 'checks a WAV-only playlist before playout'
}

if [ "$#" -eq 0 ]; then
	usage
	exit 0
fi

if [ "$#" -ne 1 ]; then
	usage
	exit 1
fi

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make first.'
	exit 1
fi

./carrierpress --playlist-check "$1"

printf '%s\n' 'build playout support with: make WITH_SNDFILE=1 WITH_PORTAUDIO=1'
printf 'play after checking with: ./carrierpress --playlist %s --meter-interval-ms 1000\n' "$1"

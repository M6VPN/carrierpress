#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/external-decode.sh

set -eu

usage()
{
	printf '%s\n' 'usage: ./examples/external-decode.sh input-media output.wav'
	printf '%s\n' 'example: ./examples/external-decode.sh input.mp3 wav-out/input.wav'
}

if [ "$#" -eq 0 ]; then
	usage
	exit 0
fi

if [ "$#" -ne 2 ]; then
	usage
	exit 1
fi

if ! command -v ffmpeg >/dev/null 2>&1; then
	printf '%s\n' 'error: ffmpeg not found. Install it manually to use this external decode example.'
	exit 1
fi

ffmpeg -i "$1" -ar 48000 -ac 2 "$2"

printf 'wrote WAV: %s\n' "$2"
printf '%s\n' 'next: ./carrierpress --playlist-check playlist.txt'

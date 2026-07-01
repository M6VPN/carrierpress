#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/ssb-bulletin-dry-run.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found; run make first'
	exit 1
fi

./carrierpress ssb-play bulletin.wav \
	--profile hf-ssb-voice \
	--audio-device "USB Audio CODEC" \
	--id M6VPN \
	--repeat 3 \
	--dry-run

./carrierpress carousel examples/ssb-carousel.toml \
	--profile hf-ssb-voice \
	--dry-run

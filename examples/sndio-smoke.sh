#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/sndio-smoke.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found. Run make first.'
	exit 1
fi

./carrierpress --self-test

printf '%s\n' ''
printf '%s\n' 'Manual sndio live-audio checks, after building WITH_SNDIO=1:'
printf '%s\n' './carrierpress --live --audio-backend sndio --device default'
printf '%s\n' './carrierpress --live --audio-backend sndio --device snd/0'
printf '%s\n' ''
printf '%s\n' 'These commands are not run by this script.'

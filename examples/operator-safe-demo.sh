#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/operator-safe-demo.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf 'operator-safe-demo: ./carrierpress not found; run make first\n' >&2
	exit 1
fi

printf 'operator-safe-demo: version\n'
./carrierpress --version

printf '\noperator-safe-demo: effective config\n'
./carrierpress --print-effective-config --profile profiles/am-safe.profile

printf '\noperator-safe-demo: playlist check\n'
./carrierpress --playlist-check examples/playout-playlist.txt

printf '\noperator-safe-demo: validation help\n'
make validation-help

printf '\noperator-safe-demo: test matrix help\n'
make test-matrix-help

printf '\noperator-safe-demo: transmit-control safety audit\n'
make transmit-control-safety-audit

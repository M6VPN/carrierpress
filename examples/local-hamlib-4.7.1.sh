#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/local-hamlib-4.7.1.sh

set -eu

HAMLIB_LOCAL=${HAMLIB_LOCAL:-/home/dgm/vendored/hamlib-4.7.1}

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'note: ./carrierpress not found. The Hamlib validation build will create it if dependencies are available.'
fi

if [ ! -d "$HAMLIB_LOCAL" ]; then
	printf 'error: local Hamlib tree not found: %s\n' "$HAMLIB_LOCAL"
	printf '%s\n' 'set HAMLIB_LOCAL=/path/to/hamlib and rerun.'
	exit 1
fi

printf '%s\n' 'local Hamlib validation only. No install, no radio, no PTT command.'
CPPFLAGS="-I$HAMLIB_LOCAL/include" \
LDFLAGS="-L$HAMLIB_LOCAL/src/.libs" \
LD_LIBRARY_PATH="$HAMLIB_LOCAL/src/.libs:${LD_LIBRARY_PATH:-}" \
	make WITH_HAMLIB=1 test

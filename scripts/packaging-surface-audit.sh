#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/scripts/packaging-surface-audit.sh

set -eu

failed=0

fail()
{
	printf '%s\n' "$1" >&2
	failed=1
}

require_match()
{
	description=$1
	pattern=$2
	file=$3

	if ! grep -Eq "$pattern" "$file"; then
		fail "packaging-surface-audit: missing ${description} in ${file}"
	fi
}

reject_match()
{
	description=$1
	pattern=$2
	file=$3

	if matches=$(grep -nEi "$pattern" "$file" 2>/dev/null); then
		fail "packaging-surface-audit: ${description} in ${file}"
		printf '%s\n' "$matches" >&2
	fi
}

if [ -d debian ]; then
	fail "packaging-surface-audit: real debian/ directory is present"
fi

if [ -d ports ]; then
	fail "packaging-surface-audit: real ports/ directory is present"
fi

require_match "pkg-config version placeholder" '^Version: @VERSION@$' \
	carrierpress.pc.in
require_match "base pkg-config libs" \
	'^Libs: -L\$\{libdir\} -lcarrierpress -lm$' carrierpress.pc.in
require_match "base pkg-config cflags" \
	'^Cflags: -I\$\{includedir\}/carrierpress$' carrierpress.pc.in
reject_match "optional dependency leaked into pkg-config metadata" \
	'sndfile|portaudio|sndio|sdl|fftw|hamlib|flrig|ncurses|mp3|flac|ogg|opus|aac|m4a|ffmpeg|libav' \
	carrierpress.pc.in

reject_match "optional backend include leaked into carrierpress_core.h" \
	'sndfile|portaudio|sndio|SDL|sdl|fftw|hamlib|flrig|ncurses|decoder|ffmpeg|libav|cp_transmit_control' \
	include/carrierpress_core.h
reject_match "optional backend include leaked into carrierpress_tooling.h" \
	'sndfile|portaudio|sndio|SDL|sdl|fftw|hamlib|flrig|ncurses|decoder|ffmpeg|libav|cp_transmit_control' \
	include/carrierpress_tooling.h

require_match "packaging post-install policy" \
	'No post-install scripts are required|Do not add post-install' \
	docs/packaging.md
require_match "packaging service policy" \
	'No daemon or service is installed|Do not add services' \
	docs/packaging.md
require_match "packaging privilege policy" \
	'No setuid or setcap is required|Do not add setuid' \
	docs/packaging.md
require_match "base pkg-config smoke documentation" \
	'make pkg-config-smoke' docs/packaging.md
require_match "packaging surface audit documentation" \
	'make packaging-surface-audit' docs/packaging.md

require_match "maintainer no post-install policy" \
	'Do not add post-install audio configuration' \
	docs/package-maintainer-checklist.md
require_match "maintainer no setuid policy" \
	'Do not add setuid, setcap, or privilege changes' \
	docs/package-maintainer-checklist.md
require_match "maintainer optional dependency policy" \
	'Optional dependencies must not leak into the base pkg-config file' \
	docs/package-maintainer-checklist.md
require_match "maintainer transmit-control policy" \
	'Do not add PTT control or CAT write/control behavior' \
	docs/package-maintainer-checklist.md

if [ "$failed" -ne 0 ]; then
	exit 1
fi

printf 'packaging-surface-audit: status=pass\n'

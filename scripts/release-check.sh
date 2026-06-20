#!/usr/bin/env bash
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/scripts/release-check.sh

set -eu

MAKE_CMD=${MAKE:-make}

enabled()
{
	if [ "${CP_CHECK_ALL_OPTIONAL:-0}" = "1" ]; then
		return 0
	fi
	if [ "${1:-0}" = "1" ]; then
		return 0
	fi
	return 1
}

run_make()
{
	printf '\n==> %s' "$MAKE_CMD"
	if [ "$#" -gt 0 ]; then
		printf ' %s' "$@"
	fi
	printf '\n'
	"$MAKE_CMD" "$@"
}

run_section()
{
	printf '\n== %s ==\n' "$1"
}

run_section "Required base validation"
run_make clean
run_make
run_make test
run_make -j test
run_make validate
run_make quality
run_make professional-check
run_make autodetect

run_section "Optional validation"

if enabled "${CP_CHECK_SNDFILE:-0}"; then
	run_make WITH_SNDFILE=1 test
else
	printf 'skip: libsndfile profile (set CP_CHECK_SNDFILE=1)\n'
fi

if enabled "${CP_CHECK_PORTAUDIO:-0}"; then
	run_make WITH_PORTAUDIO=1
else
	printf 'skip: PortAudio profile (set CP_CHECK_PORTAUDIO=1)\n'
fi

if enabled "${CP_CHECK_TUI:-0}"; then
	run_make WITH_TUI=1 test
else
	printf 'skip: ncurses TUI profile (set CP_CHECK_TUI=1)\n'
fi

if enabled "${CP_CHECK_GUI:-0}"; then
	run_make WITH_GUI=1 test
else
	printf 'skip: SDL3 GUI profile (set CP_CHECK_GUI=1)\n'
fi

if enabled "${CP_CHECK_FFTW:-0}"; then
	run_make WITH_FFTW=1 test
	if enabled "${CP_CHECK_GUI:-0}"; then
		run_make WITH_GUI=1 WITH_FFTW=1 test
	fi
else
	printf 'skip: FFTW spectrum profile (set CP_CHECK_FFTW=1)\n'
fi

if enabled "${CP_CHECK_FLRIG:-0}"; then
	run_make WITH_FLRIG=1 test
else
	printf 'skip: flrig CAT profile (set CP_CHECK_FLRIG=1)\n'
fi

if enabled "${CP_CHECK_HAMLIB:-0}"; then
	run_make WITH_HAMLIB=1 test
else
	printf 'skip: hamlib CAT profile (set CP_CHECK_HAMLIB=1)\n'
fi

if enabled "${CP_CHECK_SNDFILE:-0}" && enabled "${CP_CHECK_PORTAUDIO:-0}"; then
	run_make WITH_SNDFILE=1 WITH_PORTAUDIO=1 test
fi

if enabled "${CP_CHECK_PORTAUDIO:-0}" && enabled "${CP_CHECK_TUI:-0}"; then
	run_make WITH_PORTAUDIO=1 WITH_TUI=1 test
fi

if enabled "${CP_CHECK_GUI:-0}" && enabled "${CP_CHECK_PORTAUDIO:-0}" && \
	enabled "${CP_CHECK_FFTW:-0}"; then
	run_make WITH_GUI=1 WITH_PORTAUDIO=1 WITH_FFTW=1
fi

if enabled "${CP_CHECK_GUI:-0}" && enabled "${CP_CHECK_SNDFILE:-0}" && \
	enabled "${CP_CHECK_PORTAUDIO:-0}" && enabled "${CP_CHECK_FFTW:-0}"; then
	run_make WITH_GUI=1 WITH_SNDFILE=1 WITH_PORTAUDIO=1 WITH_FFTW=1
fi

run_section "Release validation complete"

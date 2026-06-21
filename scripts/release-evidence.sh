#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/scripts/release-evidence.sh

set -eu

printf 'CarrierPress release evidence helper\n'
printf 'No tag, push, GitHub release, package install, or PTT command is run.\n\n'

if [ -x ./carrierpress ]; then
	printf 'carrierpress version:\n'
	./carrierpress --version
else
	printf 'carrierpress version: unavailable; run make first\n'
fi

printf '\nmake version:\n'
make --version | sed -n '1p'

printf '\ngit status --short:\n'
git status --short

printf '\nlatest tag:\n'
git describe --tags --abbrev=0 2>/dev/null || printf 'none\n'

printf '\nquality JSON evidence:\n'
if [ -f build/quality-report.json ]; then
	printf 'build/quality-report.json\n'
else
	printf 'not present; run: make -s quality-json > build/quality-report.json\n'
fi

printf '\npublic API evidence:\n'
if [ -x build/examples/libcarrierpress-minimal ]; then
	printf 'build/examples/libcarrierpress-minimal\n'
else
	printf 'not present; run: make public-header-smoke && make example-libcarrierpress\n'
fi

printf '\nsource archive evidence:\n'
if [ -d build/dist ]; then
	find build/dist -type f -print | sort
else
	printf 'build/dist not present; run make dist-check\n'
fi

printf '\nstaged install files:\n'
if [ -d build/stage ]; then
	find build/stage -type f -print | sort
else
	printf 'build/stage not present; run make install-smoke or staged install first\n'
fi

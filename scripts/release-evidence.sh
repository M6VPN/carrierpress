#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/scripts/release-evidence.sh

set -eu

version=$(sed -n 's/^VERSION[[:space:]]*=[[:space:]]*//p' Makefile | sed -n '1p')

printf 'CarrierPress release evidence helper\n'
printf 'No tag, push, GitHub release, upload, sudo, package install, or transmit command is run.\n\n'

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

printf '\nvalidation target guide:\n'
make validation-help

printf '\ntest matrix guide:\n'
make test-matrix-help
printf 'full test matrix: docs/test-matrix.md\n'

printf '\npackaging surface guide:\n'
make packaging-help
printf 'surface guide: docs/api-packaging-surface.md\n'
printf 'audit: make packaging-surface-audit\n'
printf 'pkg-config smoke: make pkg-config-smoke\n'
printf 'staged install manifest, serial only: make install-manifest\n'

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

printf '\nbatch/report evidence:\n'
printf 'workflow guide: docs/report-evidence-workflow.md\n'
printf 'local demo: ./examples/report-evidence-demo.sh\n'
if [ -f build/quality-report.json ]; then
	printf 'summary: ./carrierpress --report-summary build/quality-report.json\n'
	printf 'compare: ./carrierpress --report-compare build/quality-report.json build/quality-report.json\n'
else
	printf 'not present; run: make -s quality-json > build/quality-report.json\n'
fi

printf '\ntransmit-control safety evidence:\n'
printf 'audit: make transmit-control-safety-audit\n'
printf 'operator workflow audit: make operator-workflow-safety-audit\n'
printf 'ordinary audit evidence: run make transmit-control-safety-audit\n'
printf 'guarded mock validation, serial only: make WITH_TRANSMIT_CONTROL=1 test\n'
printf 'mock wrapper, serial only because it runs make clean: make transmit-control-mock-test\n'
printf 'do not run clean-mutating targets concurrently with make -j test\n'

printf '\nsource archive evidence:\n'
if [ -n "$version" ] && [ -f "build/dist/carrierpress-$version.tar.gz" ]; then
	printf 'current tarball: build/dist/carrierpress-%s.tar.gz\n' "$version"
else
	printf 'current tarball missing; run after commit: make clean && make dist-check\n'
fi
if [ -n "$version" ] && [ -f "build/dist/carrierpress-$version.tar.gz.sha256" ]; then
	printf 'current checksum: build/dist/carrierpress-%s.tar.gz.sha256\n' "$version"
else
	printf 'current checksum missing; run after commit: make clean && make dist-check\n'
fi
if [ -d build/dist ]; then
	printf 'available dist files:\n'
	find build/dist -type f -print | sort
fi

printf '\nstaged install files:\n'
if [ -d build/stage ]; then
	if [ -x build/stage/usr/bin/carrierpress ] &&
	    [ -f build/stage/usr/lib/libcarrierpress.a ] &&
	    [ -f build/stage/usr/lib/pkgconfig/carrierpress.pc ] &&
	    [ -f build/stage/usr/share/man/man1/carrierpress.1 ] &&
	    [ -f build/stage/usr/include/carrierpress/carrierpress.h ]; then
		printf 'staged install appears complete\n'
	else
		printf 'staged install is incomplete; run make install-smoke serially\n'
	fi
	find build/stage -type f -print | sort
else
	printf 'build/stage not present; run make install-smoke or staged install first\n'
fi

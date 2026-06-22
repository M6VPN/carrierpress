#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/report-evidence-demo.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf 'error: ./carrierpress not found. Run make first.\n' >&2
	exit 1
fi

mkdir -p build

printf 'report-evidence-demo: writing build/quality-report.json\n'
make -s quality-json > build/quality-report.json

printf '\nreport-evidence-demo: summary\n'
./carrierpress --report-summary build/quality-report.json

printf '\nreport-evidence-demo: compare\n'
./carrierpress --report-compare \
	build/quality-report.json \
	build/quality-report.json

printf '\nreport-evidence-demo: reports are engineering evidence only\n'

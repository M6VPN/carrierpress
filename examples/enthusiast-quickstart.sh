#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/enthusiast-quickstart.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found; run make first' >&2
	exit 1
fi

mkdir -p build

printf '%s\n' 'CarrierPress enthusiast quick start'

printf '\n%s\n' 'Version:'
./carrierpress --version

printf '\n%s\n' 'Effective AM-safe profile:'
./carrierpress --print-effective-config --profile profiles/am-safe.profile

printf '\n%s\n' 'Quality report: build/quality-report.json'
make -s quality-json > build/quality-report.json
./carrierpress --report-summary build/quality-report.json
./carrierpress --report-compare build/quality-report.json build/quality-report.json

printf '\n%s\n' 'Safety audit:'
make transmit-control-safety-audit

printf '\n%s\n' 'Optional guarded mock validation, run serially when needed:'
printf '%s\n' 'make clean'
printf '%s\n' 'make WITH_TRANSMIT_CONTROL=1'
printf '%s\n' 'make WITH_TRANSMIT_CONTROL=1 test'
printf '%s\n' 'make transmit-control-safety-audit'
printf '%s\n' 'make transmit-control-mock-test'

printf '\n%s\n' 'Guide: docs/enthusiast-quickstart.md'

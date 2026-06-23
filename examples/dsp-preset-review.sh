#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/dsp-preset-review.sh

set -eu

if [ ! -x ./carrierpress ]; then
	printf '%s\n' 'error: ./carrierpress not found; run make first' >&2
	exit 1
fi

mkdir -p build

printf '%s\n' 'CarrierPress DSP preset review'
printf '%s\n' 'Profiles:'

for profile in \
	profiles/am-safe.profile \
	profiles/am-shortwave.profile \
	profiles/ssb-speech.profile \
	profiles/file-cleanup.profile
do
	printf '\n== %s ==\n' "$profile"
	./carrierpress --print-effective-config --profile "$profile"
done

printf '\n%s\n' 'Quality report evidence: build/quality-report.json'
make -s quality-json > build/quality-report.json
./carrierpress --report-summary build/quality-report.json

printf '\n%s\n' 'Listening notes template: docs/listening-notes-template.md'
printf '%s\n' 'Reports and notes are engineering evidence only, not compliance proof.'

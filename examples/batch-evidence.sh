#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/batch-evidence.sh

set -eu

if [ "$#" -lt 3 ]; then
	echo "usage: ./examples/batch-evidence.sh batch.txt output-dir evidence-dir [carrierpress options]"
	echo "example: ./examples/batch-evidence.sh examples/batch-list.txt build/batch-out build/batch-evidence --profile profiles/file-cleanup.profile"
	exit 0
fi

if [ ! -x ./carrierpress ]; then
	echo "error: build ./carrierpress first"
	exit 1
fi

batch_list=$1
output_dir=$2
evidence_dir=$3
shift 3

./carrierpress --batch-process "$batch_list" \
	--batch-output-dir "$output_dir" \
	--evidence-dir "$evidence_dir" "$@"

#!/usr/bin/env sh
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/examples/batch-process.sh

set -eu

if [ "$#" -lt 2 ]; then
	echo "usage: ./examples/batch-process.sh batch.txt output-dir [carrierpress options]"
	echo "example: ./examples/batch-process.sh examples/batch-list.txt build/batch-out --profile profiles/file-cleanup.profile"
	exit 0
fi

if [ ! -x ./carrierpress ]; then
	echo "error: build ./carrierpress first"
	exit 1
fi

batch_list=$1
output_dir=$2
shift 2

./carrierpress --batch-process "$batch_list" \
	--batch-output-dir "$output_dir" "$@"

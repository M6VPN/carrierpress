# Batch Offline WAV Workflow

CarrierPress batch mode is a safe offline WAV workflow. It validates a text
input list, plans output and report paths, detects unsupported formats, reports
overwrite risks, then can process WAV files sequentially when built with
libsndfile support.

## Batch List Format

Batch lists are UTF-8 text files with one input path per non-comment line:

```text
# batch.txt
audio/intro.wav
audio/program.wav
audio/outro.wav
```

Blank lines are allowed. Lines beginning with `#` after optional leading space
are comments. Inline comments are not supported in this slice.

Only WAV inputs are directly supported. Entries ending in `.mp3`, `.flac`,
`.ogg`, `.opus`, `.m4a`, or any other non-WAV extension are rejected with a
message to convert externally first.

## Dry-run Planning

Run a dry-run check with:

```sh
./carrierpress --batch-check batch.txt --batch-output-dir processed
```

For each WAV input, CarrierPress plans an output path and sidecar report path
from the input basename:

```text
input:  audio/intro.wav
output: processed/intro.wav
report: processed/intro.report.json
```

The dry-run reports:

- blank or comment lines
- accepted WAV entries
- unsupported input formats
- overlong lines or paths
- duplicate planned output or report paths
- existing output or report files

If a planned output or report already exists, the dry-run fails by default. Use
`--allow-overwrite` only to acknowledge overwrite risk in the dry-run planner:

```sh
./carrierpress --batch-check batch.txt --batch-output-dir processed --allow-overwrite
```

This does not write output files.

## Batch Processing

Actual batch processing requires a libsndfile build:

```sh
make WITH_SNDFILE=1
```

The output directory must already exist. CarrierPress does not create output
directories in this slice.

```sh
mkdir -p processed
./carrierpress --batch-check examples/batch-list.txt --batch-output-dir processed
./carrierpress --batch-process examples/batch-list.txt --batch-output-dir processed --profile profiles/file-cleanup.profile
```

Each accepted WAV entry is processed through the same offline WAV DSP path used
by `--input` and `--output`. For each input, CarrierPress writes:

```text
input:  audio/intro.wav
output: processed/intro.wav
report: processed/intro.report.json
```

The sidecar report uses the processed-file JSON format documented in
[`measurement-reports.md`](measurement-reports.md).

Batch processing performs the full dry-run preflight before processing any
file. If any planned output WAV or report already exists, processing fails
before the first file by default.

Use `--allow-overwrite` only when replacing existing output WAV and report
files is intentional:

```sh
./carrierpress --batch-process examples/batch-list.txt --batch-output-dir processed --allow-overwrite
```

Even with `--allow-overwrite`, duplicate planned output/report paths,
unsupported formats, malformed paths, and other plan errors still fail before
processing.

## Batch Summary Reports

Batch processing can write one batch-level JSON summary report:

```sh
./carrierpress --batch-process examples/batch-list.txt \
	--batch-output-dir processed \
	--batch-summary-report processed/batch-summary.json
```

The summary is written after processing finishes. It references the batch list,
output directory, profile metadata when present, planned/processed/failed
counts, and each per-file sidecar report path. It does not replace the per-file
processed-file reports.

If batch preflight fails before processing starts, CarrierPress does not write a
successful batch summary. If summary writing fails after processing, the command
exits non-zero.

## Evidence Directory Workflow

Use `--evidence-dir DIR` to write a standard summary name into an existing
directory:

```sh
mkdir -p build/evidence
./carrierpress --batch-process examples/batch-list.txt \
	--batch-output-dir processed \
	--evidence-dir build/evidence
```

This writes:

```text
build/evidence/batch-summary.json
```

The evidence directory must already exist. CarrierPress does not create parent
directories in this slice. If both `--batch-summary-report` and
`--evidence-dir` are supplied, the explicit `--batch-summary-report` path is
used for the batch summary.

Compare two batch summary reports with the report comparison helper:

```sh
./carrierpress --report-compare \
	build/evidence-old/batch-summary.json \
	build/evidence/batch-summary.json
```

Batch summary comparison is exact for stable count fields and ordered item
`input`, `output`, `report`, and `status` fields. It is intended for
engineering regression review and local evidence bundles only.

The full report evidence workflow is documented in
[`report-evidence-workflow.md`](report-evidence-workflow.md). It includes
quality report evidence, processed-file sidecar review, batch summary review,
and local comparison examples.

## Compressed Sources

CarrierPress stays WAV/PCM-native for this workflow. Convert compressed files
with an external tool before adding them to a batch list. See
[`external-decode-workflow.md`](external-decode-workflow.md) for examples.
Future optional decoder research is documented in
[`optional-decoder-architecture.md`](optional-decoder-architecture.md), but
batch mode remains WAV-only by default.

## Safety Boundary

Batch processing is an offline WAV workflow. It does not open audio devices,
control CAT, control PTT, key a radio, or make transmitter compliance claims.
Reports remain engineering metrics only.

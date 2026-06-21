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

## Compressed Sources

CarrierPress stays WAV/PCM-native for this workflow. Convert compressed files
with an external tool before adding them to a batch list. See
[`external-decode-workflow.md`](external-decode-workflow.md) for examples.

## Safety Boundary

Batch processing is an offline WAV workflow. It does not open audio devices,
control CAT, control PTT, key a radio, or make transmitter compliance claims.
Reports remain engineering metrics only.

# Batch Offline WAV Workflow

CarrierPress batch mode is planned as a safe offline WAV workflow. The first
slice is dry-run only: it validates a text input list, plans output and report
paths, detects unsupported formats, and reports overwrite risks. It does not
process audio, create output WAV files, write reports, or create directories.

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

This still does not write output files. Actual batch audio processing is planned
for a later M18 slice.

## Compressed Sources

CarrierPress stays WAV/PCM-native for this workflow. Convert compressed files
with an external tool before adding them to a batch list. See
[`external-decode-workflow.md`](external-decode-workflow.md) for examples.

## Safety Boundary

Batch dry-run validation is an offline file-planning tool. It does not open
audio devices, process audio, control CAT, control PTT, key a radio, or make
transmitter compliance claims. Reports remain engineering metrics only.

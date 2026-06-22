# CarrierPress Report Evidence Workflow

CarrierPress reports are local engineering evidence for regression review and
operator inspection. They are not RF measurements, transmitter compliance
proof, licence-compliance proof, occupied-bandwidth proof, regulatory
certification, or broadcast-quality proof.

## Quality Report Evidence

Generate a deterministic quality JSON report:

```sh
mkdir -p build
make -s quality-json > build/quality-report.json
```

Summarize the report:

```sh
./carrierpress --report-summary build/quality-report.json
```

Compare two quality reports:

```sh
./carrierpress --report-compare build/quality-report.json build/quality-report.json
```

Quality report comparison uses the documented numeric tolerance for known
metrics. The output is deterministic key-value text for local review.

## Processed-file Report Evidence

Processed-file sidecar reports require a `WITH_SNDFILE=1` build because they
come from offline WAV processing:

```sh
make WITH_SNDFILE=1
./carrierpress --input input.wav --output output.wav --report output.report.json
./carrierpress --report-summary output.report.json
```

The sidecar is written only after successful WAV processing. Report inspection
does not open audio devices.

## Batch Evidence Directory

Check a batch plan without processing audio:

```sh
mkdir -p build/batch-out
./carrierpress --batch-check examples/batch-list.txt --batch-output-dir build/batch-out
```

Batch processing and per-file sidecar reports require `WITH_SNDFILE=1`:

```sh
make WITH_SNDFILE=1
mkdir -p build/batch-out build/batch-evidence
./carrierpress --batch-process examples/batch-list.txt \
	--batch-output-dir build/batch-out \
	--evidence-dir build/batch-evidence \
	--profile profiles/file-cleanup.profile
```

The evidence directory must already exist. CarrierPress writes
`build/batch-evidence/batch-summary.json` and keeps per-file processed reports
in the batch output directory.

Summarize and compare batch summaries:

```sh
./carrierpress --report-summary build/batch-evidence/batch-summary.json
./carrierpress --report-compare \
	build/batch-evidence/batch-summary.json \
	build/batch-evidence/batch-summary.json
```

Batch summary comparison is exact for stable count fields and ordered item
fields. It is intended for regression checks and local evidence bundles.

## Local Evidence Helper

The release evidence helper prints local evidence status and recommended
commands. It does not tag, push, publish releases, install packages, use sudo,
or run transmit actions:

```sh
sh scripts/release-evidence.sh
```

For a compact report evidence smoke check:

```sh
./examples/report-evidence-demo.sh
```

## Validation Links

Related validation guides:

- [`validation-targets.md`](validation-targets.md)
- [`test-matrix.md`](test-matrix.md)
- [`measurement-reports.md`](measurement-reports.md)
- [`batch-workflow.md`](batch-workflow.md)

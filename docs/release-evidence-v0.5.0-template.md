# CarrierPress v0.5.0 Release Evidence Template

Use this template to record the final local validation run before deciding
whether to tag `v0.5.0`. This is a manual evidence log. It does not create
tags, push tags, publish a GitHub release, upload artifacts, install packages,
or run hardware transmit actions.

This evidence does not prove RF performance, transmitter compliance, licence
compliance, regulatory approval, legal bandwidth, occupied bandwidth, or
broadcast quality. Hardware TX backend support remains absent. Guarded
transmit-control support remains mock-only.

## Release Candidate

- Release candidate commit SHA:
- Date and time:
- Operator:
- Host OS:
- Compiler:
- `./carrierpress --version`:

## Base Build

| Command | Result | Notes |
| ------- | ------ | ----- |
| `make clean` | | |
| `make` | | |
| `make test` | | |
| `make -j test` | | |
| `make validate` | | |
| `make quality` | | |
| `make quality-json` | | |
| `make professional-check` | | |
| `make public-header-smoke` | | |
| `make example-libcarrierpress` | | |
| `./build/examples/libcarrierpress-minimal` | | |
| `make pkg-config-smoke` | | |
| `make packaging-help` | | |
| `make packaging-surface-audit` | | |
| `make transmit-control-safety-audit` | | |
| `make validation-help` | | |
| `make test-matrix-help` | | |
| `make operator-workflow-safety-audit` | | |
| `./examples/enthusiast-quickstart.sh` | | |
| `./examples/operator-safe-demo.sh` | | |
| `./examples/dsp-preset-review.sh` | | |
| `sh scripts/release-evidence.sh` | | |
| `make release-check` | | |

## Guarded Mock Validation

Run this block serially. `make transmit-control-mock-test` runs `make clean`
internally.

| Command | Result | Notes |
| ------- | ------ | ----- |
| `make clean` | | |
| `make WITH_TRANSMIT_CONTROL=1` | | |
| `make WITH_TRANSMIT_CONTROL=1 test` | | |
| `make transmit-control-safety-audit` | | |
| `make transmit-control-mock-test` | | |

## Optional Matrix

Record skipped dependencies clearly. Optional dependency failures must not block
the base build unless that optional profile is required for the release run.

| Command | Result | Notes |
| ------- | ------ | ----- |
| `make WITH_SNDFILE=1 test` | | |
| `make WITH_SNDFILE=1 WITH_PORTAUDIO=1 test` | | |
| `make WITH_GUI=1 test` | | |
| `make WITH_GUI=1 WITH_FFTW=1 test` | | |
| `make WITH_TUI=1 test` | | |
| `make WITH_SNDIO=1 test` | | |
| `make WITH_FLRIG=1 test` | | |
| `make WITH_HAMLIB=1 test` | | |
| `make WITH_TRANSMIT_CONTROL=1 WITH_GUI=1 test` | | |
| `make WITH_TRANSMIT_CONTROL=1 WITH_TUI=1 test` | | |

## Source Archive

- Dist tarball path:
- Dist tarball SHA256:
- `sha256sum -c build/dist/carrierpress-0.5.0.tar.gz.sha256`:
- `make dist-check` result:
- Tarball Makefile version inspection:
- Tarball release notes inspection:

## Safety Evidence

- Hardware TX backend absent:
- CAT write/control absent:
- hamlib/flrig PTT calls absent:
- serial/GPIO/VOX TX control absent:
- Frequency/mode setting absent:
- Guarded transmit-control remains mock-only:
- Ordinary builds remain non-transmit:

## Skipped Optional Dependencies

List optional dependencies or local profiles that were not available:

- 

## Reviewer Notes

- 

## Manual Tag Decision

- Decision:
- Reviewer:
- Date/time:
- Notes:

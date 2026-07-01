# CarrierPress v0.5.0 Release Readiness

This checklist prepares a local v0.5.0 release candidate. It does not create
tags, push tags, publish a GitHub release, upload artifacts, install packages,
or run hardware transmit actions.

R12B adds manual release evidence and final tag checklist documents only:

- [`release-evidence-v0.5.0-template.md`](release-evidence-v0.5.0-template.md)
- [`release-evidence-v0.5.0-summary.md`](release-evidence-v0.5.0-summary.md)
- [`manual-release-checklist-v0.5.0.md`](manual-release-checklist-v0.5.0.md)

Publication remains manual.

R12C locks the v0.5.0 release date and records that final local validation was
completed before the manual tag decision. Base validation passed, guarded mock
validation passed, and tarball/checksum inspection passed after regenerating
the dist archive.

## Required Validation

```sh
make clean
make
make test
make -j test
make validate
make quality
make quality-json
make professional-check
make public-header-smoke
make example-libcarrierpress
./build/examples/libcarrierpress-minimal
make pkg-config-smoke
make packaging-help
make packaging-surface-audit
make transmit-control-safety-audit
make validation-help
make test-matrix-help
make operator-workflow-safety-audit
./examples/enthusiast-quickstart.sh
./examples/operator-safe-demo.sh
./examples/dsp-preset-review.sh
sh scripts/release-evidence.sh
make release-check
```

## Guarded Mock Validation

Run this profile serially:

```sh
make clean
make WITH_TRANSMIT_CONTROL=1
make WITH_TRANSMIT_CONTROL=1 test
make transmit-control-safety-audit
make transmit-control-mock-test
```

`make transmit-control-mock-test` runs `make clean` internally. Do not run it
concurrently with other build or test targets.

## Optional Matrix

Run these only where the matching optional development libraries are present:

```sh
make WITH_SNDFILE=1 test
make WITH_SNDFILE=1 WITH_PORTAUDIO=1 test
make WITH_GUI=1 test
make WITH_GUI=1 WITH_FFTW=1 test
make WITH_TUI=1 test
make WITH_SNDIO=1 test
make WITH_FLRIG=1 test
make WITH_HAMLIB=1 test
make WITH_TRANSMIT_CONTROL=1 WITH_GUI=1 test
make WITH_TRANSMIT_CONTROL=1 WITH_TUI=1 test
```

Optional dependency failures must not affect the base build.

## Release Archive

After committing the intended release-prep tree:

```sh
make clean
make dist
make dist-check
sha256sum -c build/dist/carrierpress-0.5.0.tar.gz.sha256
tar -xOf build/dist/carrierpress-0.5.0.tar.gz carrierpress-0.5.0/include/cp_version.h | grep '0.5.0'
```

`make dist-check` archives committed `HEAD`, not uncommitted working-tree
content.

`make clean` removes `build/dist`, and guarded mock validation may clean the
build tree. Inspect the tarball immediately after `make dist` or
`make dist-check`. If `build/dist/carrierpress-0.5.0.tar.gz` is missing,
regenerate it with:

```sh
make clean
make dist
make dist-check
```

## Manual Final Actions

These are operator actions only. They are not run by project scripts or this
patch.

- Inspect the dist tarball.
- Review `docs/release-notes-v0.5.0.md`.
- Review `CHANGELOG.md`.
- Create a signed or annotated tag manually, if desired.
- Publish a GitHub release manually, if desired.

## Safety Checks

- Ordinary builds remain non-transmit.
- `WITH_TRANSMIT_CONTROL=1` remains mock-only.
- Hardware TX backend remains absent.
- CAT write/control remains absent.
- hamlib/flrig PTT calls remain absent.
- serial/GPIO/VOX TX control remains absent.
- Frequency and mode setting remain absent.
- Native compressed-audio decoding remains absent.
- Release publication remains manual.

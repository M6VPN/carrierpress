# CarrierPress v0.5.0 Release Notes

Suggested tag: `v0.5.0`

## Summary

CarrierPress v0.5.0 is an enthusiast-facing product-readiness release. It
completes the product-roadmap pivot after v0.4.1 by pairing the existing
portable DSP core with clearer preset guidance, operator workflow polish,
interactive selector foundations, safe local demos, and guarded mock-only
transmit-control UI validation.

## Highlights

- DSP preset UX and listening/regression notes for AM, SSB, and cleanup
  profiles.
- Professional TUI/GUI dashboard grouping for Processing, Meters, Playout,
  Selectors, Device, Workflow, and Safety surfaces.
- Interactive selector foundations for output-device choices, WAV audio-file
  cue/load candidates, and playlist cue/load candidates.
- Enthusiast quick-start guide and safe local demo scripts.
- Guarded mock-only TX operator controls under `WITH_TRANSMIT_CONTROL=1`.
- Hardware TX backend safety design and validation template for future work
  only.

## Safety Boundaries

- Ordinary builds remain non-transmit.
- `WITH_TRANSMIT_CONTROL=1` remains mock-only.
- No hardware TX backend exists.
- No CAT write/control backend exists.
- No hamlib or flrig PTT calls exist.
- No serial, GPIO, or VOX TX control exists.
- No frequency or mode setting exists.
- T7 hardware backend work remains future and deferred.

CarrierPress is not an RF generator, transmitter compliance tool,
licence-compliance proof, legal-bandwidth proof, regulatory certification
path, or broadcast-quality proof.

## Validation Commands

The release-readiness checklist, manual evidence template, and final manual
release checklist are:

- [`release-readiness-v0.5.0.md`](release-readiness-v0.5.0.md)
- [`release-evidence-v0.5.0-template.md`](release-evidence-v0.5.0-template.md)
- [`release-evidence-v0.5.0-summary.md`](release-evidence-v0.5.0-summary.md)
- [`manual-release-checklist-v0.5.0.md`](manual-release-checklist-v0.5.0.md)

Final local validation was completed before the manual tag decision. Base
validation passed, guarded mock validation passed, and tarball/checksum
inspection passed after regenerating the dist archive. Tagging and publication
remain manual.

Run the ordinary release-readiness profile:

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
make dist
make dist-check
make release-check
```

Run guarded mock validation serially:

```sh
make clean
make WITH_TRANSMIT_CONTROL=1
make WITH_TRANSMIT_CONTROL=1 test
make transmit-control-safety-audit
make transmit-control-mock-test
```

## Optional Dependency Notes

Optional dependencies remain manual and opt-in. CarrierPress project scripts do
not install packages, use `sudo`, create tags, push tags, upload artifacts, or
publish releases.

Optional local profiles include libsndfile, PortAudio, SDL3, FFTW, ncurses,
sndio, read-only flrig CAT status, read-only hamlib CAT status, and guarded
mock transmit-control validation. Missing optional dependencies must not affect
the base build.

## Known Deferred Work

- Native compressed-audio decoding.
- Native file dialogs.
- Hardware TX backend implementation.
- T7B and later backend interface or validation follow-ups.
- Future DSP refinements only with tests and documented regression evidence.

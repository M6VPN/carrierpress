# CarrierPress v0.3.0 Release Notes

Suggested tag: `v0.3.0`

CarrierPress v0.3.0 is the operator-workflow and inspectability release after
`v0.2.0`. It keeps DSP behavior intact and adds effective config/profile
inspection, safer batch WAV processing, schema-versioned report tools, TUI/GUI
operator polish, public API examples, packaging maintainer notes, and deferred
GUI workflow requests.

## Effective Config and Profile Inspection

- Added `--validate-profile PATH`.
- Added `--validate-config PATH`.
- Added `--print-effective-config`.
- Displayed config and profile source paths in inspection output.
- Documented override precedence after defaults, config files, profiles, and
  command-line options.
- Kept CAT, PTT, rig-control, transmit-state, and station-control settings out
  of ordinary inspection output.

## Batch Offline WAV Workflow

- Added a strict batch input-list format for offline WAV jobs.
- Added `--batch-check` for dry-run planning and validation.
- Added safe output-directory planning.
- Added duplicate and overwrite-risk detection.
- Added `--batch-process` for sequential WAV processing when built with
  `WITH_SNDFILE=1`.
- Added one JSON sidecar report per processed batch output.
- Kept MP3, FLAC, OGG, Opus, and M4A decoding external.

## Report Schema and Compare Tools

- Added an explicit JSON report schema version field.
- Documented stable schema-v1 fields for scripts.
- Added `--report-summary REPORT.json`.
- Added `--report-compare BASE.json NEW.json`.
- Kept report summaries and comparisons as engineering regression metrics only.

## TUI and GUI Operator Polish

- Added active config, profile, report, and cue context to operator-facing
  formatting.
- Kept GUI text bounded inside monitor panels.
- Added GUI keyboard parity for safe TUI-style audio-chain controls.
- Improved level and spectrum labels.
- Added screenshot/evidence metadata.
- Kept the CLI fully usable without TUI or GUI support.

## Library and API Examples

- Added `carrierpress_core.h` as the recommended stable core umbrella header.
- Added `carrierpress_tooling.h` for dependency-light tooling APIs.
- Kept `carrierpress.h` as the broad compatibility umbrella.
- Added a minimal static-library example.
- Added public-header smoke targets.
- Documented stable, tooling, experimental, host-specific, and compatibility
  API areas.

## Packaging Maintainer Notes

- Added a package-maintainer checklist.
- Added Debian-style packaging notes.
- Added OpenBSD ports-style packaging notes.
- Added portable/source packaging notes.
- Kept the local tarball and checksum workflow manual.
- Kept optional dependencies optional and out of the base pkg-config metadata.

## GUI Workflow Requests

- Added a dependency-light GUI workflow request model.
- Added GUI-safe WAV cue requests.
- Added GUI-safe playlist cue requests.
- Added GUI output-device selection request and display.
- Added live PortAudio output-device restart outside SDL and audio callbacks.
- Added one fallback attempt to the previous live PortAudio output device when
  a GUI-selected device cannot open or start.
- Kept GUI playout and sndio output-device restart deferred unless selected in
  a future milestone.
- Kept any GUI TRANSMIT or CAT control toggle deferred to the T5 safety gate.

## Explicit Non-goals

- No PTT control is implemented.
- No active TRANSMIT control is implemented.
- No CAT write/control commands are implemented.
- CarrierPress does not key a radio.
- CarrierPress does not generate RF.
- CarrierPress does not certify transmitter compliance, licence compliance,
  regulatory approval, legal bandwidth, or broadcast quality.
- Native MP3, FLAC, OGG, Opus, and M4A playout is not implemented.

## Validation

Before tagging `v0.3.0`, run:

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
make install-smoke
make install-manifest
make dist-check
make release-check
./carrierpress --version
```

Also run the safe example smoke tests:

```sh
./examples/self-test.sh
./examples/profile-self-test.sh
./examples/config-self-test.sh
./examples/print-effective-config.sh
./examples/validate-profile.sh
./examples/validate-config.sh
./examples/playlist-check.sh
./examples/batch-check.sh
make -s quality-json > build/quality-report.json
./examples/report-summary.sh build/quality-report.json
./examples/report-compare.sh build/quality-report.json build/quality-report.json
./examples/release-check-local.sh
```

Run optional build profiles only on hosts with the matching development
libraries.

## Known Deferred Work

- PTT control remains deferred behind the T5 safety checklist.
- GUI playout output-device restart remains deferred unless selected later.
- sndio GUI output-device restart remains deferred unless selected later.
- Native compressed-audio playout remains deferred.
- STM32H753 and CMSIS-DSP support remain planned.
- RF generation, transmitter control, NRSC certification, C-QUAM, and
  regulatory compliance tooling are not part of this release.

# CarrierPress v0.2.0 Release Notes

Suggested tag: `v0.2.0`

CarrierPress v0.2.0 is the first audio-workflow release after `v0.1.1`. It
keeps the existing DSP behavior intact and adds repeatable profile/config
workflows, playout diagnostics, engineering reports, OpenBSD/sndio validation
notes, and packaging polish.

## Profile Workflow

- Added a strict dependency-free audio profile parser.
- Added `--profile PATH` runtime loading.
- Added example profiles for AM-safe, AM-shortwave, SSB speech, and file
  cleanup workflows.
- Documented command-line override precedence.
- Rejected CAT, PTT, rig frequency, rig mode, transmit-state, flrig, hamlib,
  and station-control settings from profiles.

## Config Workflow

- Added a strict dependency-free ordinary config parser for host and workflow
  defaults.
- Added `--config PATH` runtime loading.
- Added profile loading from config files.
- Documented default-search-path planning and runtime command-line precedence.
- Rejected PTT, transmit, rig-control, station-control, flrig, hamlib, CAT
  backend, and DSP profile keys from ordinary config files.

## Playout Workflow

- Added `--playlist-check PATH` for dependency-light playlist dry-run
  validation.
- Improved playlist diagnostics with file and line context.
- Added concise cue/status output for playout and playlist transitions.
- Documented the external decode workflow for MP3, FLAC, OGG, Opus, and M4A.
- Kept compressed audio formats outside native playout support.

## Measurement Reports

- Added JSON quality report output through `make quality-json`.
- Added optional processed WAV sidecar JSON reports through `--report PATH`.
- Documented report fields as engineering metrics only.
- Kept reports free of transmitter-compliance, regulatory-certification,
  legal-bandwidth, and broadcast-quality claims.

## OpenBSD and sndio Validation

- Added OpenBSD build notes.
- Added a manual sndio validation checklist.
- Added sndio device notes.
- Kept the Linux PortAudio path unchanged.

## Packaging and Release Polish

- Added distro packaging notes.
- Added staged install manifest review.
- Added a local source tarball and checksum workflow with `make dist-check`.
- Kept optional dependencies optional and out of the base pkg-config metadata.
- Kept release publication manual and local.

## Explicit Non-goals

- No PTT control is implemented.
- No CAT write/control commands are implemented.
- CarrierPress does not key a radio.
- CarrierPress does not generate RF.
- CarrierPress does not certify transmitter compliance, licence compliance,
  regulatory approval, legal bandwidth, or broadcast quality.
- Native MP3, FLAC, OGG, Opus, and M4A playout is not implemented.

## Validation

Before tagging `v0.2.0`, run:

```sh
make clean
make
make test
make -j test
make validate
make quality
make quality-json
make professional-check
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
./examples/playlist-check.sh
./examples/release-check-local.sh
```

Run optional build profiles only on hosts with the matching development
libraries.

## Known Deferred Work

- PTT control remains deferred behind the T5 safety checklist.
- Native compressed-audio playout remains deferred.
- OpenBSD/sndio hardware validation remains manual unless a stable runner is
  added.
- STM32H753 and CMSIS-DSP support remain planned.
- RF generation, transmitter control, NRSC certification, C-QUAM, and
  regulatory compliance tooling are not part of this release.

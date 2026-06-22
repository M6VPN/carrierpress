# CarrierPress v0.4.0 Release Notes

Suggested tag: `v0.4.0`

CarrierPress v0.4.0 is the GUI workflow, evidence bundle, API, packaging, and
decoder-research release after `v0.3.0`. It keeps DSP behavior intact and does
not add decoder libraries, native compressed-audio support, PTT control,
TRANSMIT controls, or CAT write/control commands.

## GUI Playout Workflow Parity

- Added GUI playout output-device restart.
- Kept restart outside GUI callbacks.
- Kept restart outside real-time audio callbacks.
- Added previous-device fallback when a GUI-selected output device cannot open
  or start.
- Preserved CLI-only playout behavior.
- Preserved TUI playout controls.

## sndio GUI and Device Workflow Evaluation

- Documented sndio GUI device workflow constraints.
- Kept sndio optional.
- Kept the Linux PortAudio GUI device-switching path unchanged.
- Kept OpenBSD and sndio validation manual unless CI support appears later.

## GUI Workflow Display Polish

- Improved queued WAV and playlist display.
- Improved pending request status display.
- Improved rejected request error display.
- Displayed PortAudio output-device choices where enumeration is available.
- Kept GUI text restrained to panels.

## Batch and Report Evidence Bundles

- Added a batch-level summary report.
- Added an evidence directory workflow.
- Documented batch summary and evidence bundle fields.
- Added batch summary report comparison.
- Kept reports as engineering metrics only.

## API and Packaging Hardening

- Reviewed `carrierpress_core.h` after example use.
- Added stronger public-header smoke examples.
- Reviewed package-maintainer notes after v0.3.
- Kept optional dependencies out of base pkg-config metadata.

## Optional Decoder Architecture Research

- Documented possible optional decoder interface options.
- Kept compressed audio formats external by default.
- Added no decoder libraries.
- Kept the base build WAV/PCM-native.

## Explicit Non-goals

- No PTT control is implemented.
- No active TRANSMIT control is implemented.
- No CAT write/control commands are implemented.
- CarrierPress does not key a radio.
- CarrierPress does not generate RF.
- CarrierPress does not certify transmitter compliance, licence compliance,
  regulatory approval, legal bandwidth, or broadcast quality.
- Native MP3, FLAC, OGG, Opus, and M4A decoding is not implemented.

## Validation

Before tagging `v0.4.0`, run:

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
- sndio GUI output-device restart remains deferred unless selected later.
- Native compressed-audio decoding remains deferred.
- STM32H753 and CMSIS-DSP support remain planned.
- RF generation, transmitter control, NRSC certification, C-QUAM, and
  regulatory compliance tooling are not part of this release.

# CarrierPress Examples

These examples are safe command wrappers for local CarrierPress testing. They
do not install packages, use sudo, publish releases, create tags, key a radio,
or send CAT write/control commands.

Run examples from the repository root after building `./carrierpress`.

## Base Examples

```sh
make
./examples/self-test.sh
./examples/validate-profile.sh
./examples/validate-config.sh
./examples/print-effective-config.sh
./examples/profile-self-test.sh
./examples/cat-mock-status.sh
./examples/release-check-local.sh
```

`self-test.sh` runs the built-in tone through the default chain plus AM, SSB,
and dehummer/multiband examples.

`profile-self-test.sh` runs each bundled profile through self-test and then
checks AM and SSB command-line profile overrides.

`validate-profile.sh`, `validate-config.sh`, and
`print-effective-config.sh` inspect profile/config files without opening audio
devices.

## Config Files

Config examples live in `configs/` and are documented in
`docs/config-file-format.md`. They are parsed and validated by the base test
suite and can be loaded explicitly with `--config PATH`.

Config files are host workflow defaults only. They do not configure DSP chain
presets, CAT, PTT, rig frequency, rig mode, transmit state, or station control.

```sh
./examples/config-self-test.sh
./examples/validate-config.sh
./examples/print-effective-config.sh
```

GUI config testing requires `WITH_GUI=1`:

```sh
make WITH_GUI=1
./examples/config-gui-demo.sh
```

## WAV Processing

WAV processing requires a `WITH_SNDFILE=1` build:

```sh
make WITH_SNDFILE=1
./examples/wav-process.sh input.wav output.wav
./examples/wav-report.sh input.wav output.wav output.report.json
./examples/profile-wav-process.sh profiles/file-cleanup.profile input.wav output.wav
```

The wrapper processes a WAV file through the current chain with dehummer and a
three-band multiband stage. It does not overwrite the input file.
The profile wrapper loads one safe audio profile before processing.
The report wrapper writes a processed-file JSON sidecar with engineering
metrics only.

## Report Tools

Summarize or compare CarrierPress JSON reports without opening audio devices:

```sh
make -s quality-json > build/quality-report.json
./examples/report-summary.sh build/quality-report.json
./examples/report-compare.sh build/quality-report.json build/quality-report.json
```

Report comparisons are regression metrics only. They do not prove listening
quality, RF bandwidth, transmitter compliance, regulatory approval, legal
station operation, or licence compliance.

## Playout

WAV playout requires a `WITH_SNDFILE=1 WITH_PORTAUDIO=1` build and a usable
PortAudio output device:

```sh
make WITH_SNDFILE=1 WITH_PORTAUDIO=1
./examples/playout.sh input.wav
```

Check playlist syntax without PortAudio or playback:

```sh
./examples/playlist-check.sh
./examples/playlist-workflow.sh examples/playout-playlist.txt
```

Check a future batch WAV plan without processing audio or writing outputs:

```sh
./examples/batch-check.sh
```

Process a checked WAV batch with a `WITH_SNDFILE=1` build:

```sh
make WITH_SNDFILE=1
mkdir -p build/batch-out
./examples/batch-process.sh examples/batch-list.txt build/batch-out --profile profiles/file-cleanup.profile
```

Batch processing writes output WAV files and one `.report.json` sidecar per
file. It does not create the output directory and does not overwrite existing
outputs unless `--allow-overwrite` is supplied. Batch workflow details are
documented in `docs/batch-workflow.md`.

To play the playlist, edit `examples/playout-playlist.txt` so it points at local
WAV files, then run:

```sh
./carrierpress --playlist examples/playout-playlist.txt --meter-interval-ms 1000
```

Text playout prints concise cue/status lines for file starts, playlist cues,
stops, and completion. TUI and GUI playout keep cue/status information inside
the monitor display instead of printing extra text lines.

When TUI or GUI monitoring is enabled, the monitor also shows the active config,
profile, report, batch, and cue context where those values exist. Use
`./examples/print-effective-config.sh` for script-friendly resolved settings.

CarrierPress does not play MP3, FLAC, OGG, Opus, or M4A natively in this
milestone. Convert those files to WAV first:

```sh
./examples/external-decode.sh input.mp3 wav-out/input.wav
```

See `docs/external-decode-workflow.md` for the full external decode workflow.

## OpenBSD and sndio

The sndio smoke helper runs self-test and prints manual live-audio commands. It
does not open a live stream or change system audio settings:

```sh
./examples/sndio-smoke.sh
```

Build with `WITH_SNDIO=1` before trying the printed live-audio commands. See
`docs/openbsd-sndio.md` for the full manual checklist.

## GUI Demo

The GUI monitor requires `WITH_GUI=1`. The spectrum panel requires
`WITH_FFTW=1`:

```sh
make WITH_GUI=1 WITH_FFTW=1
./examples/gui-demo.sh
./examples/gui-screenshot.sh
```

The screenshot example writes `build/gui-demo.bmp` and
`build/gui-demo.bmp.txt`.

## Read-only CAT

CAT examples are read-only. They do not key PTT, change frequency, change
mode, or send transmit commands.

```sh
./examples/cat-mock-status.sh
./examples/cat-flrig-readonly.sh
./examples/cat-hamlib-readonly.sh
```

`cat-flrig-readonly.sh` expects flrig XML-RPC on `127.0.0.1:12345`. If flrig is
not running, an unavailable or error status is expected. `cat-hamlib-readonly.sh`
uses hamlib model `1`, the hamlib dummy backend documented in the main README.

## Local Hamlib 4.7.1

Local Hamlib validation is optional and local-only:

```sh
./examples/local-hamlib-4.7.1.sh
```

The script defaults to `/home/dgm/vendored/hamlib-4.7.1`, does not install
Hamlib, does not require a radio, and does not make CarrierPress depend on the
local vendored path. Override with:

```sh
HAMLIB_LOCAL=/path/to/hamlib ./examples/local-hamlib-4.7.1.sh
```

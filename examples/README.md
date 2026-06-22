# CarrierPress Examples

These examples are safe command wrappers for local CarrierPress testing. They
do not install packages, use sudo, publish releases, create tags, key a radio,
or send CAT write/control commands.

Run examples from the repository root after building `./carrierpress`.

## Example Safety Index

Most examples are local inspection wrappers. Some write files under `build/`
or need optional libraries. The table below summarizes the expected boundary.

| Area | Mutates build output | Optional dependencies | Serial-only | Hardware access |
| ---- | -------------------- | --------------------- | ----------- | --------------- |
| Base validation and config/profile examples | no | no | no | no |
| `operator-safe-demo.sh` | no | no | no | no |
| Library and public-header smoke examples | writes `build/examples` | no | no | no |
| WAV processing and batch processing | writes requested outputs | libsndfile | no | no |
| Playout examples | no by default | libsndfile and PortAudio | no | output audio device |
| GUI examples | may write `build/gui-demo.bmp` | SDL3, optional FFTW | no | GUI display |
| sndio smoke helper | no | sndio for manual commands | no | manual only |
| Read-only CAT examples | no | flrig or hamlib where selected | no | read-only status only |
| Release and evidence helpers | no | no | no | no |
| Packaging surface checks | no | no | no | no |
| Report evidence demo | writes `build/quality-report.json` | no | no | no |
| Batch evidence workflow | writes requested `build/` outputs | libsndfile for processing | no | no |
| Guarded mock transmit-control validation | writes build outputs | no | yes for mock wrapper | no |

Examples must not install packages, use `sudo`, create tags, push tags,
publish releases, upload artifacts, key a radio, or send CAT write/control
commands. Operator workflow boundaries are documented in
`docs/operator-workflow.md`. Selector workflows are documented in
`docs/selector-workflow.md`; output-device selector display uses the shared
state model where backend choices are available. Audio-file selector display
uses explicit WAV candidates or recent cue slots and keeps compressed formats
disabled for external conversion. Playlist selector display uses explicit
`.txt` and `.playlist` candidates and existing playlist validation before
deferred requests are accepted. The selector model is not a file dialog,
device opener, decoder, playlist auto-discovery tool, or transmit-control UI.

## Base Examples

```sh
make
./examples/self-test.sh
./examples/operator-safe-demo.sh
./examples/validate-profile.sh
./examples/validate-config.sh
./examples/print-effective-config.sh
./examples/profile-self-test.sh
./examples/cat-mock-status.sh
make validation-help
make test-matrix-help
make packaging-help
./examples/release-check-local.sh
```

`self-test.sh` runs the built-in tone through the default chain plus AM, SSB,
and dehummer/multiband examples.

`profile-self-test.sh` runs each bundled profile through self-test and then
checks AM and SSB command-line profile overrides.

`validate-profile.sh`, `validate-config.sh`, and
`print-effective-config.sh` inspect profile/config files without opening audio
devices.

`make validation-help` prints the local validation target classes.
`make test-matrix-help` prints the ordinary, guarded mock, and optional
dependency test matrix. `make packaging-help` prints the local packaging and
pkg-config surface checks. Release evidence is reported by
`scripts/release-evidence.sh`; it does not tag, push, publish, install
packages, or run transmit actions.

`operator-safe-demo.sh` runs local inspection, playlist checking, validation
help, test matrix help, and the transmit-control safety audit. It does not run
`make clean`, open audio devices, require optional dependencies, access
hardware, or publish anything.

## Library Example

The minimal library example links against the in-tree static library and uses
only `carrierpress_core.h` and the dependency-light block-processing API:

```sh
make example-libcarrierpress
./build/examples/libcarrierpress-minimal
```

Additional public-header smoke examples check the recommended core, tooling,
and compatibility umbrellas:

```sh
make public-core-header-smoke
make public-tooling-header-smoke
make public-compat-header-smoke
make public-header-smoke
make pkg-config-smoke
make packaging-surface-audit
```

After installing CarrierPress, the same example can be built with pkg-config:

```sh
cc -std=c17 -Wall -Wextra -o libcarrierpress-minimal \
  examples/libcarrierpress-minimal.c \
  $(pkg-config --cflags --libs carrierpress)
```

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
./examples/report-evidence-demo.sh
```

Report comparisons are regression metrics only. They do not prove listening
quality, RF bandwidth, transmitter compliance, regulatory approval, legal
station operation, or licence compliance.

The report evidence demo writes `build/quality-report.json`, summarizes it, and
compares it with itself. It does not run `make clean`, use optional
dependencies, open audio devices, publish releases, or run transmit actions.
The full workflow is documented in `docs/report-evidence-workflow.md`.

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

Write a batch-level summary into an existing evidence directory:

```sh
mkdir -p build/batch-evidence
./examples/batch-evidence.sh examples/batch-list.txt build/batch-out build/batch-evidence --profile profiles/file-cleanup.profile
```

The script writes `build/batch-evidence/batch-summary.json` through
`--evidence-dir` and leaves per-file sidecar reports in the output directory.
Batch processing requires a `WITH_SNDFILE=1` build; dry-run batch checks do not.
Compare two batch summary reports without processing audio:

```sh
./examples/batch-report-compare.sh build/batch-evidence/batch-summary.json build/batch-evidence/batch-summary.json
```

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
`docs/openbsd-sndio.md` for the full manual checklist and
`docs/sndio-gui-device-workflow.md` for the GUI device workflow evaluation.

## GUI Demo

The GUI monitor requires `WITH_GUI=1`. The spectrum panel requires
`WITH_FFTW=1`:

```sh
make WITH_GUI=1 WITH_FFTW=1
./examples/gui-demo.sh
./examples/gui-screenshot.sh
./examples/gui-cue-demo.sh audio/program.wav examples/playout-playlist.txt
```

The screenshot example writes `build/gui-demo.bmp` and
`build/gui-demo.bmp.txt`.

The current GUI is a monitor and safe-control demo. Preconfigured WAV and
playlist cue slots can be requested from the GUI with `l` and `p`, then
validated outside SDL callbacks. The GUI status panel shows queued cue paths,
the latest request, and pending, valid, or error status with bounded text.
Deferred output-device selection requests can be created with `o` and `O`. In
live PortAudio GUI mode, those requests are consumed by the host loop and
applied by restarting the stream outside SDL and audio callbacks. If the
requested device cannot open or start, CarrierPress tries once to fall back to
the previous output device. GUI WAV playout uses the same deferred request
pattern and reopens the blocking PortAudio output stream between processed
blocks. Where PortAudio enumeration is available, the GUI status panel shows a
compact output-choice line. Future file dialogs and sndio switching are
documented in
`docs/gui-workflow.md` and
`docs/sndio-gui-device-workflow.md`.

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

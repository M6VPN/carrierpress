# CarrierPress Enthusiast Quick Start

CarrierPress is a local baseband audio processor and monitor for radio and
audio enthusiasts. It provides AM, SSB, and cleanup profiles, optional GUI and
TUI operator surfaces, output-device and cue selectors, report tools, and safe
demo flows for local review.

For automated SSB voice bulletin and audio playout workflows, start with
[`ssb-bulletin-workflow.md`](ssb-bulletin-workflow.md). CarrierPress sends
audio content for people to hear or record; it is not a file-transfer modem.

The v0.5.0 release notes are in
[`release-notes-v0.5.0.md`](release-notes-v0.5.0.md), and the local
release-readiness checklist is in
[`release-readiness-v0.5.0.md`](release-readiness-v0.5.0.md).

CarrierPress is not an RF generator, transmitter compliance tool, legal
bandwidth proof, licence-compliance proof, regulatory certification path, or
operational hardware PTT controller. No hardware TX backend exists.

## Base Build and First Run

The base build is dependency-light and does not require libsndfile, PortAudio,
sndio, SDL3, FFTW, ncurses, hamlib, flrig, or decoder libraries:

```sh
make
./carrierpress --version
./carrierpress --print-effective-config --profile profiles/am-safe.profile
```

Run the first-run helper after building:

```sh
./examples/enthusiast-quickstart.sh
```

The helper writes only under `build/`, does not open audio devices, and does
not require optional dependencies.

## Profile and Report Review

Generate and inspect a local quality report:

```sh
mkdir -p build
make -s quality-json > build/quality-report.json
./carrierpress --report-summary build/quality-report.json
./carrierpress --report-compare build/quality-report.json build/quality-report.json
```

Review shipped DSP profile settings:

```sh
./examples/dsp-preset-review.sh
```

DSP preset notes are in `docs/dsp-product-presets.md`. Listening note fields
are in `docs/listening-notes-template.md`. Reports and notes are engineering
and regression evidence only.

## Safe Operator Demos

Run the local operator demo:

```sh
./examples/operator-safe-demo.sh
```

The ordinary operator workflow includes CLI inspection, optional TUI and GUI
monitoring, output-device selectors, explicit WAV cue/load selectors, playlist
selectors, and report inspection. Ordinary builds do not include active TX
controls.

Operator workflow details are in:

- `docs/operator-workflow.md`
- `docs/gui-workflow.md`
- `docs/selector-workflow.md`
- `examples/README.md`

## Optional GUI, TUI, and Audio Paths

Optional dependencies are installed manually by the user or system package
manager. CarrierPress scripts do not install packages.

- `WITH_SNDFILE=1` enables WAV file processing and batch WAV tests.
- `WITH_PORTAUDIO=1` enables PortAudio host paths.
- `WITH_SNDFILE=1 WITH_PORTAUDIO=1` enables WAV playout paths.
- `WITH_TUI=1` enables the ncurses TUI when ncurses is available.
- `WITH_GUI=1` enables the SDL3 GUI when SDL3 is available.
- `WITH_GUI=1 WITH_FFTW=1` enables the GUI spectrum path when FFTW is
  available.
- `WITH_SNDIO=1` enables optional sndio build tests. OpenBSD validation remains
  manual.
- `WITH_FLRIG=1` and `WITH_HAMLIB=1` are read-only CAT status paths.

Compressed audio remains an external-conversion workflow by default. CarrierPress
does not add native MP3, FLAC, OGG, Opus, M4A, AAC, or container decoding.

## Guarded Mock TX Workflow

`WITH_TRANSMIT_CONTROL=1` is a mock-only state-machine and UI validation path.
It can show mock TX status and guarded mock arm, disarm, mock TX request, and
emergency RX/drop controls. It does not call hardware, CAT write/control,
hamlib or flrig PTT, serial, GPIO, VOX, or rig frequency/mode controls.

Run guarded mock validation serially:

```sh
make clean
make WITH_TRANSMIT_CONTROL=1
make WITH_TRANSMIT_CONTROL=1 test
make transmit-control-safety-audit
make transmit-control-mock-test
```

`make transmit-control-mock-test` runs `make clean` internally. Do not run it
in parallel with other build or test targets.

Hardware TX backend work is deferred to T7 and requires a separate safety
milestone before implementation.

T7A is design-only. It documents future hardware backend gates in
[`hardware-tx-backend-safety-design.md`](hardware-tx-backend-safety-design.md)
and a future manual evidence template in
[`hardware-tx-validation-template.md`](hardware-tx-validation-template.md).
No hardware backend exists, and guarded builds remain mock-only.

## Recommended Validation

Ordinary enthusiast validation:

```sh
make test
make validate
make quality-json
make operator-workflow-safety-audit
make transmit-control-safety-audit
```

For target scheduling and optional build paths, see:

- `docs/validation-targets.md`
- `docs/test-matrix.md`

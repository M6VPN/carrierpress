# CarrierPress

[![CI](https://github.com/M6VPN/carrierpress/actions/workflows/ci.yml/badge.svg)](https://github.com/M6VPN/carrierpress/actions/workflows/ci.yml)

CarrierPress is a portable C DSP skeleton for real-time and offline AM and SSB audio processing. v0.4 provides a clean-room core with block processing, float32 samples, a DC blocker, RMS and peak meters, a gated input AGC, optional dehummer, optional natural dynamics and low-level boost stages, optional multiband compressor foundations, optional static bass EQ, optional restoration and auto EQ analysis taps, an optional conservative declipper research stage, a safe peak limiter, strict profile/config workflows, batch WAV processing, report tools, operator UI polish, library/API examples, packaging hardening, and optional decoder architecture research.

The long-term goal is AM/SSB audio processing for legal transmitters and test loads. Users are responsible for complying with radio regulations, transmitter licence limits, occupied bandwidth limits, and local operating rules.

Offline WAV processing is available as an optional M1 foundation when built with libsndfile. Experimental live sound-card I/O is available as an optional M2 foundation when built with PortAudio. Optional sndio support exists as an OpenBSD-style foundation, but remaining sndio work is deferred while Linux-host core processing quality is the active focus. Optional WAV playout to a sound-card output is available when both libsndfile and PortAudio are enabled. The ncurses TUI and SDL3 GUI monitor are optional host interfaces. AM output-chain shaping is available as an M6 foundation. SSB output-chain shaping is available as an M7 foundation. Static bass EQ is available as an M8.1 foundation. M9.4 adds optional conservative natural dynamics and low-level boost stages before AGC. M10.2 adds an optional second conservative multiband polish stage after bass EQ and before AM or SSB shaping. M10.3 adds an optional analysis-only auto EQ tap for tonal-balance diagnostics. M10.4 adds bounded bass EQ recommendations from the auto EQ analyzer without applying them automatically. MP3 playout and STM32H753 support are planned but are not part of v0.1. Unsupported playlist entries are reported with line details. CarrierPress stays WAV/PCM-native internally for this milestone.

## Table of Contents

- [Requirements](#requirements)
- [Setup](#setup)
- [Examples](#examples)
- [Enthusiast Quick Start](#enthusiast-quick-start)
- [Profiles](#profiles)
- [Config Files](#config-files)
- [Usage](#usage)
- [Development](#development)
- [Roadmap](#roadmap)
- [License](#license)

## Requirements

- C17 compiler such as `cc`, `clang`, or `gcc`
- `make`
- Standard C math library
- Optional [libsndfile](https://libsndfile.github.io/libsndfile/) development package for WAV processing
- Optional [PortAudio](https://www.portaudio.com/) development package for live USB sound-card processing
- Optional [sndio](https://sndio.org/) development package for OpenBSD-style live audio
- Optional [ncurses](https://invisible-island.net/ncurses/) development package for the live TUI monitor
- Optional [SDL3](https://wiki.libsdl.org/SDL3/FrontPage) development package for the GUI monitor
- Optional [FFTW](https://www.fftw.org/) single-precision development package for the GUI spectrum monitor
- Optional [hamlib](https://hamlib.github.io/) development package for read-only CAT status

The DSP core builds without PortAudio, libsndfile, sndio, ncurses, SDL3, or
FFTW, or CAT control libraries. CAT includes a dependency-free read-only mock
backend and an optional read-only flrig XML-RPC backend plus an optional
read-only hamlib backend. If your system is missing optional
WAV support, install the libsndfile development package manually. Common
package names are `libsndfile1-dev`, `libsndfile-devel`, or `libsndfile`. If
your system is missing optional PortAudio support, install the PortAudio
development package manually. Common package names are `portaudio19-dev`,
`portaudio-devel`, or `portaudio`. If your system is missing optional sndio
support, install the sndio development package manually. Common package names
are `sndio`, `sndio-devel`, or `libsndio-dev`. If your system is missing
optional TUI support, install the ncurses development package manually. Common
package names are `libncurses-dev`, `ncurses-devel`, or `ncurses`.
If your system is missing optional GUI support, install the SDL3 development
package manually. Common package names are `libsdl3-dev`, `SDL3-devel`,
`SDL3`, or `sdl3`.
If your system is missing optional FFTW support, install the single-precision
FFTW development package manually. Common package names are `libfftw3-dev`,
`fftw-devel`, `fftw3`, or `fftw3f`.
If your system is missing optional hamlib support, install the hamlib
development package manually. Common package names are `hamlib`, `libhamlib`,
`libhamlib-dev`, or `hamlib-devel`.

## Setup

Build the CLI and core objects without optional host dependencies:

```sh
make
```

Build with autodetected Linux-host optional features:

```sh
make autodetect
```

`make autodetect` compiles small dependency probes for libsndfile, PortAudio,
ncurses, SDL3, FFTW, and sndio. It prints a feature summary, prints missing
package or library names, and then builds with available libsndfile, PortAudio,
and ncurses support. SDL3 and FFTW are detected and reported, but they remain
manual GUI-monitor options. sndio is detected and reported, but it is not
auto-enabled on the active Linux path because remaining sndio work is deferred.

Explicit feature overrides still work and remain the preferred command when
you need a reproducible build profile:

```sh
make WITH_SNDFILE=1
make WITH_PORTAUDIO=1
make WITH_PORTAUDIO=1 WITH_TUI=1
make WITH_GUI=1
make WITH_GUI=1 WITH_TUI=1
make WITH_GUI=1 WITH_PORTAUDIO=1
make WITH_GUI=1 WITH_SNDFILE=1 WITH_PORTAUDIO=1
make WITH_GUI=1 WITH_FFTW=1
make WITH_GUI=1 WITH_PORTAUDIO=1 WITH_FFTW=1
make WITH_GUI=1 WITH_SNDFILE=1 WITH_PORTAUDIO=1 WITH_FFTW=1
make WITH_SNDFILE=1 WITH_PORTAUDIO=1
make WITH_FLRIG=1
make WITH_FLRIG=1 WITH_TUI=1
make WITH_FLRIG=1 WITH_PORTAUDIO=1 WITH_TUI=1
make WITH_HAMLIB=1
make WITH_HAMLIB=1 WITH_TUI=1
make WITH_HAMLIB=1 WITH_PORTAUDIO=1 WITH_TUI=1
```

If an explicit optional dependency is missing, the build fails with the package
or library name to install manually. CarrierPress does not run package managers
or use `sudo`.

Build and run the normal test set:

```sh
make test
```

Run the test target with parallel jobs:

```sh
make -j test
```

Run deterministic DSP validation fixtures and print a compact report:

```sh
make validate
```

Run deterministic audio QA fixtures and print a human-readable measurement
report:

```sh
make quality
```

Run the same quality fixtures as JSON:

```sh
make quality-json
```

Run the stricter professional validation gate:

```sh
make professional-check
```

Run the release validation profile:

```sh
make release-check
```

Print the local validation target guide:

```sh
make validation-help
```

Print the ordinary, guarded mock, and optional dependency test matrix:

```sh
make test-matrix-help
```

Run the operator workflow safety audit:

```sh
make operator-workflow-safety-audit
```

Release validation and manual tag checks are documented in
[`docs/release-checklist.md`](docs/release-checklist.md).
Validation target scheduling is documented in
[`docs/validation-targets.md`](docs/validation-targets.md).
The build and test matrix is documented in
[`docs/test-matrix.md`](docs/test-matrix.md).
Ordinary CLI, TUI, GUI, CAT status, and example workflows are documented in
[`docs/operator-workflow.md`](docs/operator-workflow.md).
GitHub Actions CI coverage is documented in [`docs/ci.md`](docs/ci.md).
Release notes, the manual GitHub release procedure, and optional local release
asset guidance are documented in
[`docs/release-notes-v0.1.md`](docs/release-notes-v0.1.md),
[`docs/release-notes-v0.1.1.md`](docs/release-notes-v0.1.1.md),
[`docs/release-notes-v0.2.0.md`](docs/release-notes-v0.2.0.md),
[`docs/release-notes-v0.3.0.md`](docs/release-notes-v0.3.0.md),
[`docs/release-notes-v0.4.0.md`](docs/release-notes-v0.4.0.md),
[`docs/release-notes-v0.4.1.md`](docs/release-notes-v0.4.1.md),
[`docs/release-procedure.md`](docs/release-procedure.md), and
[`docs/release-assets-v0.1.md`](docs/release-assets-v0.1.md).
The current release notes are in
[`docs/release-notes-v0.4.1.md`](docs/release-notes-v0.4.1.md). Safe example
wrappers are documented in [`examples/README.md`](examples/README.md). The
local safe operator demo is `./examples/operator-safe-demo.sh`. Manual release
commands remain documented in
[`docs/release-procedure.md`](docs/release-procedure.md).
The product roadmap is documented in
[`docs/roadmap-product.md`](docs/roadmap-product.md). The first product
foundation adds shared dashboard section labels and bounded selector state.
TUI and GUI dashboards group Processing, Meters, Playout, Selectors, Device,
Workflow, and Safety displays. Output-device selector display now uses that
model where backend choices are already available. Audio-file selector display
now uses explicit WAV candidates and recent cue slots; compressed formats
remain disabled external-conversion workflows. Playlist selector display now
uses explicit `.txt` and `.playlist` candidates and existing playlist
validation before deferred requests are accepted. Selector workflow notes are in
[`docs/selector-workflow.md`](docs/selector-workflow.md). TX operator controls
remain future, guarded, and mock-only unless a later safety milestone selects
more work.

## Enthusiast Quick Start

Start with [`docs/enthusiast-quickstart.md`](docs/enthusiast-quickstart.md)
and the local first-run helper:

```sh
make
./examples/enthusiast-quickstart.sh
```

The guide covers base DSP/profile review, report comparison, safe operator
demos, optional GUI/TUI/audio builds, and guarded mock TX validation. Optional
dependencies are manual. Ordinary builds cannot transmit, and no hardware TX
backend exists.

Build with optional WAV support:

```sh
make WITH_SNDFILE=1
```

Build with optional PortAudio live audio support:

```sh
make WITH_PORTAUDIO=1
```

Build with optional sndio live audio support. Remaining sndio work is deferred:

```sh
make WITH_SNDIO=1
```

Build with optional PortAudio live audio and the ncurses TUI monitor:

```sh
make WITH_PORTAUDIO=1 WITH_TUI=1
```

Build with the optional SDL3 GUI monitor:

```sh
make WITH_GUI=1
make WITH_GUI=1 WITH_TUI=1
make WITH_GUI=1 WITH_PORTAUDIO=1
make WITH_GUI=1 WITH_SNDFILE=1 WITH_PORTAUDIO=1
```

Build with optional FFTW spectrum monitor support:

```sh
make WITH_FFTW=1
make WITH_GUI=1 WITH_FFTW=1
make WITH_GUI=1 WITH_PORTAUDIO=1 WITH_FFTW=1
make WITH_GUI=1 WITH_SNDFILE=1 WITH_PORTAUDIO=1 WITH_FFTW=1
```

The GUI monitor is separate from the ncurses TUI. SDL3 rendering and event
polling run in the foreground host loop, not in the real-time audio callback.
The GUI waveform panel shows a monitor-only preview of processed output audio.
With `WITH_FFTW=1`, the GUI spectrum panel shows a monitor-only processed-output
spectrum preview. These displays do not alter audio samples.
Future GUI file workflows are documented in
[`docs/gui-workflow.md`](docs/gui-workflow.md). They use deferred requests and
are not active file dialogs. The GUI can request preconfigured WAV and playlist
cue slots with `l` and `p`. The GUI workflow panel shows queued cue paths, the
last workflow request, and pending, valid, or error status with bounded text.
In live PortAudio GUI mode and GUI WAV playout, `o` and `O` create a deferred
output-device request that is consumed by the host loop and applied by
restarting the stream outside SDL and audio callbacks. If the requested device
cannot open or start, CarrierPress tries once to fall back to the previous
output device. Where PortAudio enumeration is available, the GUI device panel
shows a compact output-choice line. sndio GUI output-device restart is deferred
because sndio uses named devices, not PortAudio numeric device indices. See
[`docs/sndio-gui-device-workflow.md`](docs/sndio-gui-device-workflow.md):

```sh
./carrierpress --gui-demo --gui-cue-wav audio/program.wav --gui-cue-playlist playlist.txt
```

Build with both optional WAV and PortAudio support:

```sh
make WITH_SNDFILE=1 WITH_PORTAUDIO=1
```

Build all current optional host backends, including deferred sndio:

```sh
make WITH_SNDFILE=1 WITH_PORTAUDIO=1 WITH_SNDIO=1
```

Build with optional WAV playout support:

```sh
make WITH_SNDFILE=1 WITH_PORTAUDIO=1
```

Build and run tests with optional WAV support:

```sh
make WITH_SNDFILE=1 test
```

Build and run tests with optional WAV playout support:

```sh
make WITH_SNDFILE=1 WITH_PORTAUDIO=1 test
```

Print the installed version:

```sh
./carrierpress --version
```

Install under a local prefix:

```sh
make
make PREFIX="$HOME/.local" install
```

Stage an install for packaging review:

```sh
make DESTDIR=build/stage PREFIX=/usr install
make install-smoke
make install-manifest
```

Uninstall from the same prefix:

```sh
make PREFIX="$HOME/.local" uninstall
```

The install target copies the `carrierpress` binary, public headers,
`libcarrierpress.a`, `carrierpress.pc`, and `carrierpress(1)`. If your shell
cannot find the staged tools, set `PATH`, `PKG_CONFIG_PATH`, or `MANPATH` for
your chosen prefix:

```sh
pkg-config --cflags --libs carrierpress
man carrierpress
```

Packaging notes, staged install review, and the local source archive workflow
are documented in [docs/packaging.md](docs/packaging.md):

```sh
make dist-check
ls -lh build/dist/
```

The API and packaging surface guide is in
[`docs/api-packaging-surface.md`](docs/api-packaging-surface.md):

```sh
make packaging-help
make packaging-surface-audit
```

Package maintainer checklists and example distro packaging notes are in
[`docs/package-maintainer-checklist.md`](docs/package-maintainer-checklist.md).
These notes are documentation only and do not add real distro packaging files.

## Library API

CarrierPress installs public headers and `libcarrierpress.a` for dependency-light
C programs that use the core baseband audio processor. New library users should
include `carrierpress_core.h` for stable in-memory DSP use. The API status and
stable/experimental header areas are documented in [docs/api.md](docs/api.md).

Build the minimal in-tree example with:

```sh
make example-libcarrierpress
./build/examples/libcarrierpress-minimal
```

Check that the umbrella public header links without optional dependencies:

```sh
make public-core-header-smoke
make public-tooling-header-smoke
make public-compat-header-smoke
make public-header-smoke
make pkg-config-smoke
make packaging-surface-audit
```

The active Linux-host validation path is:

```sh
make clean
make
make test
make -j test
./carrierpress --self-test
make validate
make quality
make professional-check
make WITH_SNDFILE=1 WITH_PORTAUDIO=1 WITH_TUI=1
make WITH_SNDFILE=1 WITH_PORTAUDIO=1 test
```

## Examples

Safe example wrappers are in [`examples/`](examples/). They cover self-test,
WAV processing, WAV playout, GUI demo, GUI screenshot capture, mock CAT,
read-only flrig CAT, read-only hamlib CAT, local Hamlib 4.7.1 validation, and
local release validation. `./examples/dsp-preset-review.sh` prints resolved
settings for the shipped DSP profiles and creates local quality report evidence.

The examples do not use `sudo`, install packages, create tags, publish GitHub
releases, key PTT, change rig frequency, change rig mode, or send CAT
write/control commands. Optional examples require matching build flags:
`WITH_SNDFILE=1` for WAV processing, `WITH_SNDFILE=1 WITH_PORTAUDIO=1` for
playout, `WITH_GUI=1` for GUI examples, `WITH_FFTW=1` for GUI spectrum, and
`WITH_FLRIG=1` or `WITH_HAMLIB=1` for read-only CAT backend examples.

Start with:

```sh
make
./examples/self-test.sh
./examples/validate-profile.sh
./examples/validate-config.sh
./examples/print-effective-config.sh
./examples/cat-mock-status.sh
```

## Profiles

Profile files are documented in
[`docs/profile-format.md`](docs/profile-format.md). Safe starting profiles are
in [`profiles/`](profiles/), including AM-safe, AM-shortwave, SSB-speech, and
file-cleanup examples. Enthusiast-facing preset notes, listening workflow, and
A/B comparison guidance are documented in
[`docs/dsp-product-presets.md`](docs/dsp-product-presets.md). A subjective
listening notes template is available in
[`docs/listening-notes-template.md`](docs/listening-notes-template.md).

Profiles are audio-chain settings only. They do not control CAT, PTT, rig
frequency, rig mode, transmit state, flrig, hamlib, or station-control state.

Load a profile with `--profile PATH`:

```sh
./carrierpress --profile profiles/am-safe.profile --self-test
./carrierpress --profile profiles/ssb-speech.profile --self-test
./carrierpress --profile profiles/file-cleanup.profile --input in.wav --output out.wav
```

Defaults are created first, the profile is applied when `--profile` appears,
and later command-line options override profile values. For example:

```sh
./carrierpress --profile profiles/am-safe.profile --am-preset am-voice --self-test
./carrierpress --profile profiles/ssb-speech.profile --ssb-preset ssb-narrow --self-test
```

Options before `--profile` may be overwritten by the loaded profile. Only one
profile may be loaded in this M11 slice.

Validate a profile without running DSP:

```sh
./carrierpress --validate-profile profiles/am-safe.profile
```

Review the shipped profile descriptions and deterministic quality evidence:

```sh
./examples/dsp-preset-review.sh
```

## Config Files

Config files are documented in
[`docs/config-file-format.md`](docs/config-file-format.md). Safe examples are
in [`configs/`](configs/), including default, live Pulse/PipeWire, GUI demo,
and playout workflow defaults.

Config files are host and workflow defaults only. Audio-chain settings belong
in profiles. Config files do not control CAT, PTT, rig frequency, rig mode,
transmit state, flrig, hamlib, or station-control state.

Use `--config PATH` to load a config file explicitly:

```sh
./carrierpress --config configs/default.conf --self-test
./carrierpress --config configs/live-pulse.conf --live
./carrierpress --config configs/gui-demo.conf --gui-demo
```

Config loading is order-sensitive. CarrierPress starts with built-in defaults,
applies `--config PATH` where it appears, loads any `profile = PATH` named by
that config, and then lets later command-line options override those values.
Options before `--config` may be overwritten by the config.

Validate a config and inspect final resolved settings without opening audio
devices:

```sh
./carrierpress --validate-config configs/default.conf
./carrierpress --config configs/default.conf --profile profiles/ssb-speech.profile --print-effective-config
```

Effective config inspection is documented in
[`docs/effective-config.md`](docs/effective-config.md).

## Usage

Run the built-in synthetic tone through the v0.1 chain:

```sh
./carrierpress --self-test
```

The command prints input and output meter values plus AGC gain, gain dB, and gate state.

Print a one-shot read-only mock CAT status:

```sh
./carrierpress --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off --cat-status
./carrierpress --cat-backend none --cat-status
```

Build the read-only flrig CAT backend:

```sh
make WITH_FLRIG=1
make WITH_FLRIG=1 WITH_TUI=1
make WITH_FLRIG=1 WITH_PORTAUDIO=1 WITH_TUI=1
```

Print a one-shot read-only flrig CAT status:

```sh
./carrierpress --cat-backend flrig --cat-host 127.0.0.1 --cat-port 12345 --cat-status
```

Run live or playout TUI mode with read-only flrig status:

```sh
./carrierpress --live --tui --cat-backend flrig --cat-host 127.0.0.1 --cat-port 12345
./carrierpress --play input.wav --tui --cat-backend flrig --cat-host 127.0.0.1 --cat-port 12345
```

The flrig backend uses XML-RPC read calls for frequency, mode, and PTT readback
only. If flrig is not running or the host/port is unavailable, CarrierPress
prints or displays `unavailable` or `error` CAT status and audio processing
continues. The default host is `127.0.0.1`, the default port is `12345`, and
the default timeout is short enough for foreground status polling. Use
`--cat-host`, `--cat-port`, and `--cat-timeout-ms` to override them.

CAT support does not send transmit commands, set PTT, set frequency, set mode,
or key a radio.

### CAT PTT Safety

CAT support is read-only in the current code. flrig and hamlib PTT state is
readback only, and no PTT control exists. Any future PTT-control feature must be
compiled explicitly, enabled explicitly at runtime, and follow
[`docs/cat-ptt-safety.md`](docs/cat-ptt-safety.md),
[`docs/transmit-control-architecture.md`](docs/transmit-control-architecture.md),
and [`docs/transmit-control-checklist.md`](docs/transmit-control-checklist.md)
before any transmitter control is allowed. T5 documents future safety gates
only. Ordinary CarrierPress remains baseband audio processing with read-only
CAT status.

`WITH_TRANSMIT_CONTROL=1` currently builds a mock-only runtime-arming state
machine for tests. Ordinary builds still report transmit control unavailable.
The guarded mock path does not add PTT, CAT write/control, hardware backends,
GUI TRANSMIT controls, TUI TRANSMIT controls, CLI transmit options, or
station-control behavior. T5D adds mock-only emergency RX/drop tests in that
guarded state machine. T5E adds static callback and path isolation audit
coverage through `make transmit-control-safety-audit`.

T6A adds a guarded mock-only status display for GUI/TUI safety surfaces when
`WITH_TRANSMIT_CONTROL=1` is built. It shows mock state only. It does not add
arm/disarm controls, transmit-request controls, emergency controls, hardware
backends, or CAT write/control.

T6B adds guarded mock-only `r` arm and `u` disarm controls when
`WITH_TRANSMIT_CONTROL=1` is built. They affect only the mock state machine.
Ordinary builds still have no TX controls, and no transmit-request, emergency
RX/drop, CAT write/control, or hardware backend is added.

T6C adds guarded mock-only `t` TX request and `x` emergency RX/drop controls
when `WITH_TRANSMIT_CONTROL=1` is built. They affect only the mock state
machine. Ordinary builds still have no TX controls, and no CAT write/control
or hardware backend is added.

Build the read-only hamlib CAT backend:

```sh
make WITH_HAMLIB=1
make WITH_HAMLIB=1 WITH_TUI=1
make WITH_HAMLIB=1 WITH_PORTAUDIO=1 WITH_TUI=1
```

Print a one-shot read-only hamlib CAT status:

```sh
./carrierpress --cat-backend hamlib --cat-rig-model N --cat-rig-path PATH --cat-status
```

Run live or playout TUI mode with read-only hamlib status:

```sh
./carrierpress --live --tui --cat-backend hamlib --cat-rig-model N --cat-rig-path PATH
./carrierpress --play input.wav --tui --cat-backend hamlib --cat-rig-model N --cat-rig-path PATH
```

The hamlib backend reads frequency, mode, and PTT readback only. It requires
`--cat-rig-model` for real hardware so CarrierPress does not guess a radio.
`--cat-rig-path` and `--cat-rig-speed` are optional and should match the rig
connection. If hamlib is not built, no rig model is supplied, or the rig cannot
be opened, CarrierPress prints or displays `unavailable` or `error` CAT status
and audio processing continues. Hamlib model `1` is the hamlib dummy backend
and can be used for a hardware-free read-only smoke test.

Show mock CAT status in the TUI while running live or playout mode:

```sh
./carrierpress --live --tui --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off
./carrierpress --play input.wav --tui --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off
```

CAT status is read outside the real-time audio callback. Users remain
responsible for licence limits, station-control requirements, and transmitter
operating rules.

Build the optional SDL3 GUI monitor:

```sh
make WITH_GUI=1
make WITH_GUI=1 WITH_TUI=1
make WITH_GUI=1 WITH_PORTAUDIO=1
make WITH_GUI=1 WITH_SNDFILE=1 WITH_PORTAUDIO=1
```

Run the GUI demo without audio hardware:

```sh
./carrierpress --gui-demo --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off
```

Save one deterministic GUI demo screenshot without audio hardware:

```sh
./carrierpress --gui-demo-screenshot build/gui-demo.bmp --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off
```

Run live or playout monitoring with the SDL3 GUI:

```sh
./carrierpress --live --gui
./carrierpress --play input.wav --gui
./carrierpress --playlist playlist.txt --gui
```

`--gui` and `--tui` are mutually exclusive. Without a `WITH_GUI=1` build,
GUI commands exit with:

```text
GUI support not enabled. Rebuild with WITH_GUI=1.
```

The GUI monitor groups display text into Processing, Meters, Playout,
Selectors, Device, Workflow, and Safety panels. It shows transport state, input
and output peak/RMS meters, AGC state, stream flags, processing-chain state,
read-only CAT status, a processed-output waveform preview, and an optional FFTW
spectrum preview. The waveform and spectrum previews are monitor-only. GUI text
is clipped inside the monitor panels, and safe keyboard controls mirror the
ncurses TUI:
`q`/Escape stop, `n` playlist next, `d` dehummer, `m` multiband 1, `b`
multiband 2, `a` AM bank, `s` SSB bank, `0` off, and `1` to `4` presets.
Preconfigured GUI cue slots use `l` for WAV, `p` for playlist, and `c` for
the current playlist item when playlist context exists.
Live capture uses preallocated storage and does not allocate, print, lock, call
FFTW planning, call SDL, or do blocking work from the audio callback. FFTW
planning and execution run outside the callback. For stereo audio, the waveform
preview draws channel 1 and the spectrum preview averages channels for display.
It is an engineering monitor, not a calibrated spectrum analyser, and does not
replace the ncurses TUI.
Screenshot and visual-regression checks are manual or optional. They are not
required by base builds or normal tests. `--gui-demo-screenshot` writes a BMP
and a `.txt` metadata sidecar with version, mode, profile/config, and report
context when available. See
[`docs/gui-visual-regression.md`](docs/gui-visual-regression.md).

Run the same self-test with the M9.3 restoration analyzer enabled:

```sh
./carrierpress --self-test --analyze
```

The analyzer reports clipping, low-ceiling, transient, source-profile, and
high-frequency-loss indicators.

Run the self-test with the conservative declipper research stage enabled:

```sh
./carrierpress --self-test --analyze --declipper
```

`--declipper` enables analysis automatically. The declipper is disabled by
default and only repairs analysis-confident hard-clipping or low-ceiling
clipping blocks. Transient-like blocks and low-confidence blocks are bypassed.

Run the self-test with M9.4 natural dynamics and low-level boost enabled:

```sh
./carrierpress --self-test --natural-dynamics --low-level-boost
```

Natural dynamics is a conservative wideband stage that gently reduces loud
blocks before AGC. Low-level boost raises quiet but non-silent program material
within a configured limit and leaves silence gated. Both stages are disabled by
default.

Useful starting controls are:

```sh
./carrierpress --self-test --natural-dynamics --natural-threshold-db -18 --natural-ratio 1.5 --natural-max-reduction-db 4
./carrierpress --self-test --low-level-boost --low-level-target-rms 0.12 --low-level-max-boost-db 6
```

Run the same self-test with the dehummer enabled:

```sh
./carrierpress --self-test --dehummer --hum-frequency 50 --hum-harmonics 4
```

Run the self-test with the M5 multiband compressor enabled:

```sh
./carrierpress --self-test --multiband --multiband-bands 3 --multiband-preset speech
```

Run the self-test with the M10.2 second multiband polish stage enabled:

```sh
./carrierpress --self-test --multiband2 --multiband2-bands 3 --multiband2-preset speech
./carrierpress --self-test --multiband --bass-eq --multiband2 --multiband2-bands 3 --multiband2-preset music
```

The second multiband stage is disabled by default. It reuses the current 2 to 4
band splitter and compressor, but uses gentler final-polish settings than the
first multiband stage. It is not final broadcast loudness processing.

Run the self-test with the M10.3 auto EQ analyzer enabled:

```sh
./carrierpress --self-test --auto-eq-analyze
./carrierpress --self-test --bass-eq-recommend
```

The auto EQ analyzer is disabled by default. It measures fixed tonal bands,
relative band levels, spectral tilt, and a source hint. It does not change
audio samples and does not implement adaptive EQ.

Run the self-test with the M8.1 static bass EQ enabled:

```sh
./carrierpress --self-test --bass-eq --bass-eq-preset music
./carrierpress --self-test --bass-eq --bass-gain-db 2.0 --bass-frequency 120 --presence-gain-db 0.5
```

Run the self-test with the M6 AM output chain enabled:

```sh
./carrierpress --self-test --am --am-preset am-safe
./carrierpress --self-test --am --am-preset am-shortwave
./carrierpress --self-test --dehummer --multiband --multiband-bands 3 --am --am-preset am-shortwave
```

Run the self-test with the M7 SSB output chain enabled:

```sh
./carrierpress --self-test --ssb --ssb-preset ssb-speech
./carrierpress --self-test --dehummer --multiband --multiband-bands 3 --ssb --ssb-preset ssb-narrow
```

Process a mono or stereo WAV file through the current chain:

```sh
./carrierpress --input input.wav --output output.wav
```

Write a processed-file JSON sidecar report:

```sh
make WITH_SNDFILE=1
./carrierpress --input input.wav --output output.wav --report output.report.json
```

The sidecar report contains engineering metrics for the input and processed
output. It does not prove RF bandwidth, transmitter compliance, regulatory
approval, legal operation, or broadcast quality. See
[docs/measurement-reports.md](docs/measurement-reports.md).

Process a WAV file and print source analysis metrics:

```sh
./carrierpress --input input.wav --output output.wav --analyze
```

Process a WAV file with the conservative declipper research stage enabled:

```sh
./carrierpress --input input.wav --output output.wav --declipper
./carrierpress --input input.wav --output output.wav --declipper --declipper-strength 0.35
```

Process a WAV file with 60 Hz hum notches and four harmonics:

```sh
./carrierpress --input input.wav --output output.wav --dehummer --hum-frequency 60 --hum-harmonics 4
```

Process a WAV file with static bass EQ:

```sh
./carrierpress --input input.wav --output output.wav --bass-eq --bass-eq-preset warm
```

WAV support requires a `WITH_SNDFILE=1` build. Without it, the command exits with:

```text
WAV support not enabled. Rebuild with WITH_SNDFILE=1.
```

Play a WAV file through CarrierPress and send the processed result to the default output device:

```sh
./carrierpress --play input.wav
```

Play mode uses the same DSP chain controls as live mode. Dehummer, multiband,
second multiband, auto EQ analysis, natural dynamics, low-level boost, AM
shaping, AGC, limiter, and metering are applied to fixed-size WAV blocks before
they are sent to the output device.

Choose an output device for WAV playout:

```sh
./carrierpress --play input.wav --output-device 3
./carrierpress --play input.wav --device pulse
```

Play a simple playlist:

```sh
./carrierpress --playlist playlist.txt --device pulse
```

Text playout prints concise cue/status lines when a file starts, a playlist
item is cued, a file ends, playback stops, or a playlist completes. TUI and GUI
playout suppress text cue lines so the monitor display stays readable.
In GUI playout, `o` and `O` request an output-device change. The playout host
loop consumes the request after GUI event handling, reopens the blocking
PortAudio output stream between processed blocks, and falls back once to the
previous output device if the requested device fails.

Run playout with the ncurses monitor:

```sh
./carrierpress --play input.wav --device pulse --tui
./carrierpress --playlist playlist.txt --device pulse --tui
```

In playout TUI mode, press `n` to skip to the next playlist item and `q` to
stop. The same AM/SSB bank and preset keys used by live mode are available
during playout.

Print live-style meters once per second while playing:

```sh
./carrierpress --play input.wav --meter-interval-ms 1000
```

Print analysis metrics while playing:

```sh
./carrierpress --play input.wav --analyze --meter-interval-ms 1000
./carrierpress --play input.wav --auto-eq-analyze --meter-interval-ms 1000
./carrierpress --play input.wav --bass-eq-recommend --meter-interval-ms 1000
```

Run playout through the conservative declipper research stage:

```sh
./carrierpress --play input.wav --declipper --meter-interval-ms 1000
```

Run playout through static bass EQ:

```sh
./carrierpress --play input.wav --bass-eq --bass-eq-preset music
```

Stop play mode with `Ctrl-C`. CarrierPress stops cleanly between processed
blocks. WAV playout tries the WAV file sample rate first. If the selected output
device cannot open that rate and no explicit sample rate was requested,
CarrierPress retries the output device default rate and uses the internal linear
resampler.

Force a playout output rate:

```sh
./carrierpress --play input.wav --sample-rate 48000
./carrierpress --playlist playlist.txt --device pulse --sample-rate 48000
```

The M7.3 resampler is a utility-quality linear converter for playout
compatibility. It is not a mastering-grade sample-rate converter.

Playlist files are plain text. Blank lines and lines beginning with `#` are ignored. Each other line is a WAV path:

```text
# playlist.txt
first.wav
second.wav
```

Check playlist syntax without opening PortAudio or playing audio:

```sh
./carrierpress --playlist-check playlist.txt
```

`--playlist-check` is available in the base build. It reports file and line
diagnostics, accepts `.wav` entries case-insensitively, skips blank/comment
lines, and reports unsupported entries such as MP3, FLAC, OGG, Opus, and M4A as
formats to convert to WAV first. It checks playlist syntax and supported
extensions only; it does not require the listed files to exist.

Batch dry-run validation plans offline WAV batch outputs without processing
audio or writing output files:

```sh
./carrierpress --batch-check examples/batch-list.txt --batch-output-dir processed
```

`--batch-check` validates one WAV input path per non-comment line, plans output
and `.report.json` sidecar paths from the input basename, detects duplicate
planned paths, reports existing output/report files, and rejects compressed
formats with a convert-to-WAV-first message. `--allow-overwrite` only changes
dry-run overwrite diagnostics.

Actual batch processing requires `WITH_SNDFILE=1`, an existing output
directory, and WAV inputs:

```sh
make WITH_SNDFILE=1
mkdir -p processed
./carrierpress --batch-process examples/batch-list.txt --batch-output-dir processed --profile profiles/file-cleanup.profile
```

`--batch-process` runs the same offline WAV DSP path as `--input` and
`--output`, writes one output WAV per item, and writes one `.report.json`
sidecar per output. It fails before processing if planned output/report files
already exist unless `--allow-overwrite` is supplied. See
[docs/batch-workflow.md](docs/batch-workflow.md).

Batch runs can also write an aggregate summary report:

```sh
./carrierpress --batch-process examples/batch-list.txt --batch-output-dir processed --batch-summary-report processed/batch-summary.json
```

Use `--evidence-dir DIR` to write `DIR/batch-summary.json` into an existing
local evidence directory. See
[docs/measurement-reports.md](docs/measurement-reports.md).
Batch summary reports can also be compared with `--report-compare` for local
regression review.

WAV playout requires a `WITH_SNDFILE=1 WITH_PORTAUDIO=1` build. Without it, `--play` and `--playlist` exit with:

```text
Playout support not enabled. Rebuild with WITH_SNDFILE=1 WITH_PORTAUDIO=1.
```

MP3, FLAC, OGG, Opus, and M4A playout are not implemented in this milestone.
Convert them to WAV with an external tool before using `--play` or
`--playlist`.

See [docs/external-decode-workflow.md](docs/external-decode-workflow.md) for
the full workflow and
[docs/optional-decoder-architecture.md](docs/optional-decoder-architecture.md)
for future optional decoder research. Basic examples:

```sh
ffmpeg -i input.mp3 -ar 48000 -ac 2 output.wav
ffmpeg -i input.flac -ar 48000 -ac 2 output.wav
ffmpeg -i input.ogg -ar 48000 -ac 2 output.wav
ffmpeg -i input.opus -ar 48000 -ac 2 output.wav
ffmpeg -i input.m4a -ar 48000 -ac 2 output.wav
```

Then play the converted WAV:

```sh
./carrierpress --play output.wav
```

List PortAudio devices:

```sh
./carrierpress --list-devices
```

Run experimental live processing with automatic backend selection:

```sh
./carrierpress --live
```

On Linux, automatic selection prefers usable JACK devices, then PipeWire or
Pulse-compatible full-duplex devices visible through PortAudio, then PortAudio
defaults.

Choose a backend policy:

```sh
./carrierpress --live --audio-backend auto
./carrierpress --live --audio-backend jack
./carrierpress --live --audio-backend alsa
./carrierpress --live --audio-backend pulse
./carrierpress --live --audio-backend sndio
./carrierpress --live --audio-backend default
```

On OpenBSD or systems using sndio, build with `WITH_SNDIO=1` and select sndio
explicitly. See [docs/openbsd-sndio.md](docs/openbsd-sndio.md) for OpenBSD
build notes and the manual sndio validation checklist. See
[docs/sndio-gui-device-workflow.md](docs/sndio-gui-device-workflow.md) for the
GUI device workflow evaluation:

```sh
./carrierpress --live --audio-backend sndio
./carrierpress --live --audio-backend sndio --device default
```

The sndio backend uses named sndio devices through `--device NAME`. Numeric
`--input-device` and `--output-device` values are PortAudio-only.

Choose a full-duplex device by name substring:

```sh
./carrierpress --live --device pulse --channels 2
./carrierpress --live --device default --channels 2
```

Run live processing with selected USB sound-card devices:

```sh
./carrierpress --live --input-device 2 --output-device 3
```

Run live processing with explicit stream settings:

```sh
./carrierpress --live --sample-rate 48000 --channels 2 --block-size 256
```

Run live processing with conservative 50 Hz dehumming:

```sh
./carrierpress --live --dehummer --hum-frequency 50 --hum-harmonics 4 --hum-q 35
```

Run live processing with the simple speech multiband preset:

```sh
./carrierpress --live --multiband --multiband-bands 3 --multiband-preset speech
```

Run live processing with the second multiband polish stage:

```sh
./carrierpress --live --multiband --bass-eq --multiband2 --multiband2-preset music
```

Run live processing with static bass EQ:

```sh
./carrierpress --live --bass-eq --bass-eq-preset music
```

Run live processing with analysis metrics:

```sh
./carrierpress --live --analyze --meter-interval-ms 1000
./carrierpress --live --auto-eq-analyze --meter-interval-ms 1000
./carrierpress --live --bass-eq-recommend --meter-interval-ms 1000
```

Run live processing with the M9.4 pre-AGC dynamics stages:

```sh
./carrierpress --live --natural-dynamics --low-level-boost
```

Run live processing with AM-safe output shaping:

```sh
./carrierpress --live --am --am-preset am-safe
```

Run live processing with the ncurses monitor:

```sh
./carrierpress --live --tui
```

Run live AM processing with the ncurses monitor and preset keys:

```sh
./carrierpress --live --tui --am
```

The live TUI supports safe AM and SSB preset switching while audio is running.
Press `d` to toggle the dehummer. Press `m` to cycle multiband mode through off,
speech, and music. Press `b` to cycle the second multiband polish stage through
off, speech, and music. Press `a` for the AM control bank or `s` for the SSB
control bank. In the AM bank, press `0` for AM off, `1` for `am-safe`, `2` for
`am-shortwave`, `3` for `am-wide`, and `4` for `am-voice`. In the SSB bank,
press `0` for SSB off, `1` for `ssb-speech`, `2` for `ssb-narrow`, `3` for
`ssb-wide`, and `4` for `ssb-gentle`. Press `q` to stop. Preset changes are
validated and applied at audio block boundaries.

The TUI is a plain ASCII operator panel. It is readable on monochrome terminals
and does not depend on colour. The screen is split into stable regions for
transport, active audio-chain mode, selected control bank, device/config
details, input/output meters, AGC state, stream flags, chain-order processing
state, detailed status, and a compact key footer. The active audio-chain mode is
shown as `NEUTRAL`, `AM`, or `SSB`. The selected control bank is shown
separately as `AM BANK` or `SSB BANK`. When AM mode is active, SSB controls are
shown locked unless the SSB bank is explicitly selected. When SSB mode is
active, AM controls are shown locked unless the AM bank is explicitly selected.

The key footer changes by context. Live mode shows stop, bank selection,
dehummer, multiband, and preset keys. Single-file play mode shows the same
processing keys for file playout. Playlist mode adds `n next` when another item
is available. The detailed status area includes read-only CAT status when a CAT
snapshot is available. With the current mock backend this can show frequency,
mode, PTT readback state, stale/error state, or disabled state without real
hardware.

Print meters once per second:

```sh
./carrierpress --live --meter-interval-ms 1000
```

PortAudio support requires a `WITH_PORTAUDIO=1` build. Without it, live and device-list commands exit with:

```text
PortAudio support not enabled. Rebuild with WITH_PORTAUDIO=1.
```

sndio support requires a `WITH_SNDIO=1` build. Without it, sndio live mode exits with:

```text
sndio support not enabled. Rebuild with WITH_SNDIO=1.
```

TUI support requires a `WITH_TUI=1` build. Without it, `--tui` exits with:

```text
TUI support not enabled. Rebuild with WITH_TUI=1.
```

Live mode is experimental. It is a sound-card backend for testing the existing DC blocker, dehummer, analysis taps, dynamics, AGC, multiband, AM/SSB shaping, limiter, and metering chain. It is not a broadcast-quality processor.

PortAudio may print ALSA or JACK probe warnings while listing or opening devices.
Those warnings are not always fatal. Use the final CarrierPress status line and
meter output to confirm whether a stream opened.

The sndio backend is a blocking full-duplex foundation. It does not provide a
device listing command or TUI controls in this milestone. Linux-host PortAudio
live mode, WAV processing, WAV playout, and the ncurses TUI are the active host
targets for current development.

## Manual Live-Audio Test

Build with PortAudio:

```sh
make WITH_PORTAUDIO=1
```

Build with PortAudio and the TUI monitor:

```sh
make WITH_PORTAUDIO=1 WITH_TUI=1
```

List devices:

```sh
./carrierpress --list-devices
```

The listing shows host API names, default markers, live candidates, and a
recommended command when a full-duplex device is found.

For a laptop using PipeWire or pipewire-pulse, try:

```sh
./carrierpress --live --device pulse --sample-rate 44100 --channels 2
./carrierpress --live --device default --sample-rate 44100 --channels 2
```

Run live passthrough processing with the chosen devices:

```sh
./carrierpress --live --input-device 2 --output-device 3 --sample-rate 48000 --channels 2 --block-size 256
```

Check that meter lines update while audio is present. Stop with `Ctrl-C`.

If input meters stay at zero, select the correct capture source in a PipeWire or
Pulse mixer, unmute the microphone, and check that the capture level is raised.
For monitor-source testing, select the monitor of the output sink in the mixer.

Run the TUI monitor:

```sh
./carrierpress --live --tui --input-device 2 --output-device 3 --sample-rate 48000 --channels 2 --block-size 256
```

Check that input/output meters, AGC state, stream flags, multiband meters, analysis metrics when enabled, and AM/SSB settings update. Press `q` or `Ctrl-C` to stop.

## Manual TUI Smoke Test

Build with TUI support:

```sh
make WITH_PORTAUDIO=1 WITH_TUI=1
make WITH_SNDFILE=1 WITH_PORTAUDIO=1 WITH_TUI=1
```

Check an 80x24 terminal:

```sh
stty cols 80 rows 24
./carrierpress --live --tui --device pulse
./carrierpress --play input.wav --tui --device pulse
./carrierpress --playlist playlist.txt --tui --device pulse
```

Check a wider terminal:

```sh
stty cols 120 rows 32
./carrierpress --live --tui --device pulse
./carrierpress --play input.wav --tui --device pulse
./carrierpress --playlist playlist.txt --tui --device pulse
```

For each run, verify that the footer is visible, the screen does not overflow,
the active mode reads `NEUTRAL`, `AM`, or `SSB`, and the selected control bank
reads `AM BANK` or `SSB BANK`. Press `a` and `s` to switch banks. In AM mode,
confirm that SSB controls are shown locked until the SSB bank is selected. In
SSB mode, confirm that AM controls are shown locked until the AM bank is
selected. In playlist mode, confirm that `n next` appears only when another
playlist item is available.

For live control testing, press `d` to toggle dehummer and `m` to cycle
multiband off, speech, and music. For AM control testing, add `--am`, press
`a`, and press `0` through `4` to switch AM off or select one of the validated
AM presets. For SSB control testing, add `--ssb`, press `s`, and press `0`
through `4` to switch SSB off or select one of the validated SSB presets.

## Manual GUI Smoke Test

Build without GUI support and confirm GUI commands fail clearly:

```sh
make clean
make
./carrierpress --gui-demo
```

Build with GUI support:

```sh
make WITH_GUI=1
make WITH_GUI=1 WITH_FFTW=1
make WITH_GUI=1 WITH_PORTAUDIO=1 WITH_FFTW=1
make WITH_GUI=1 WITH_SNDFILE=1 WITH_PORTAUDIO=1 WITH_FFTW=1
```

Save a deterministic BMP screenshot and inspect it before running interactive
checks:

```sh
./carrierpress --gui-demo-screenshot build/gui-demo.bmp --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off
```

The screenshot command also writes `build/gui-demo.bmp.txt` with operator
context metadata.

Run the hardware-free GUI demo and confirm the window opens, mock CAT appears,
the processed-output waveform moves, the optional FFTW spectrum moves, `q`
exits cleanly, Escape exits cleanly, and closing the window exits cleanly:

```sh
./carrierpress --gui-demo --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off
```

If PortAudio is available, run live GUI monitoring:

```sh
./carrierpress --live --gui
```

If libsndfile and PortAudio are available, run playout GUI monitoring:

```sh
./carrierpress --play input.wav --gui
./carrierpress --playlist playlist.txt --gui
```

## OpenBSD and sndio Manual Validation

OpenBSD and sndio validation is manual unless a stable OpenBSD runner is added.
Linux PortAudio remains the active live-audio path. Build sndio explicitly:

```sh
make WITH_SNDIO=1
```

Run the safe local smoke helper:

```sh
./examples/sndio-smoke.sh
```

Then follow the checklist in [docs/openbsd-sndio.md](docs/openbsd-sndio.md).
The sndio backend uses named devices. GUI output-device restart remains
PortAudio-first and is evaluated in
[docs/sndio-gui-device-workflow.md](docs/sndio-gui-device-workflow.md):

```sh
./carrierpress --live --audio-backend sndio --device default
./carrierpress --live --audio-backend sndio --device snd/0
```

## Development

The core library has no optional audio backend dependency. WAV support lives in `cp_wav.c`, PortAudio support lives in `cp_portaudio.c`, sndio support lives in `cp_sndio.c`, WAV playout lives in `cp_playout.c`, the ncurses monitor lives in `cp_tui.c`, and read-only CAT status lives in `cp_cat.c`. The SDL3 GUI monitor lives in `cp_gui_sdl3.c` and is compiled only with `WITH_GUI=1`. GUI text formatting lives in dependency-free formatter helpers so it can be tested without SDL3. Processed-output waveform capture lives in dependency-free fixed-size helpers and feeds only the monitor path. The flrig XML-RPC client lives in `cp_cat_flrig.c` and is compiled only with `WITH_FLRIG=1`. The hamlib client lives in `cp_cat_hamlib.c` and is compiled only with `WITH_HAMLIB=1`. CAT mock status is dependency-free and host-side. Process functions use caller-owned buffers and explicit state structs so real-time callbacks can remain malloc-free and deterministic.

The current development baseline is Linux-host first. That means normal builds,
WAV processing, PortAudio live mode, WAV playout, TUI monitoring, and the
deterministic validation gates are checked before new DSP features are added.
The current processing-quality target is documented in
[docs/core-processing-quality.md](docs/core-processing-quality.md).

WAV playout reads fixed-size blocks from libsndfile, processes them through the normal CarrierPress chain, reports live-style meters from the processor state, and writes processed float32 blocks to a PortAudio output stream. It does not load the whole file into memory. This milestone uses blocking PortAudio output for file playout; live input mode still uses the callback backend.

Live mode and playout use the same host-to-DSP config mapping for dehummer,
natural dynamics, low-level boost, multiband, AM, SSB, and sample-rate-dependent
settings. Their monitor views use the same processor snapshot fields before
live mode hands them to atomics.

When playout resampling is active, CarrierPress runs the DSP chain at the output
stream rate so AGC and filter timing match playback. Offline `--input` and
`--output` WAV processing still preserves the input file sample rate.

`make validate` runs deterministic synthetic fixtures through default, dehummer,
multiband plus bass EQ, second multiband, AM, and SSB chain profiles. It checks
finite output, bounded peaks, AM/SSB peak limits, silence stability, DC
reduction, hum reduction, low-pass rejection, stereo stability, and AGC gain
limits. This is an engineering gate for regressions, not a claim of broadcast
processor quality. See [docs/validation.md](docs/validation.md).

`make quality` runs a fixed measurement pass over silence, speech-like steps,
music-like harmonic content, clipped sine, DC offset, 50 Hz hum, 60 Hz hum,
burst transients, high-frequency content, and stereo imbalance. It prints RMS,
peak, crest factor, DC, hum-bin, stereo-balance, clipping indicator, and
high-frequency-loss indicator measurements for the current default, dehummer,
dynamics, multiband plus bass EQ, second multiband, AM, and SSB profiles. This
is a repeatable QA report for tuning and regression review, not a listening
test, spectrum
measurement, restoration-quality claim, or compliance claim.

`make quality-json` runs the same quality fixtures and emits a single JSON
document for local evidence or tooling. The JSON report is documented in
[docs/measurement-reports.md](docs/measurement-reports.md), including the
stable schema-version-1 fields intended for scripts. It is an engineering
metrics export only and does not prove RF bandwidth, transmitter compliance, or
legal operation.

Summarize or compare CarrierPress JSON reports:

```sh
./carrierpress --report-summary build/quality-report.json
./carrierpress --report-compare old.report.json new.report.json
```

Local report and batch evidence workflows are documented in
[docs/report-evidence-workflow.md](docs/report-evidence-workflow.md). The
safe local demo is:

```sh
./examples/report-evidence-demo.sh
```

`make professional-check` is the stricter M9.5 gate. It runs deterministic
fixtures through default, dehummer, declipper, natural dynamics plus low-level
boost, multiband plus bass EQ, second multiband, AM, and SSB profiles. It fails
on non-finite output, limiter violations, AM/SSB peak-limit failures, silence
instability, weak DC or hum reduction, filter regressions, stereo instability,
restoration analysis regressions, declipper gating regressions, and missing
dynamics-stage activity. See [docs/professional-validation.md](docs/professional-validation.md).

TUI control is preset-only for DSP changes in this milestone. It can switch the
dehummer on or off, cycle the first and second multiband stages between off,
speech, and music, switch the AM and SSB output chains between validated
presets, and skip to the next track during playlist playout. AM and SSB controls
are split into explicit banks so AM preset keys do not affect SSB mode, and SSB
preset keys do not affect AM mode. It does not expose arbitrary DSP parameter
editing.

The optional TUI and SDL3 GUI monitor show the active config path, profile path,
profile name, report path, batch path, and current cue/playlist position where
that context exists. This is an operator-facing summary only. The authoritative
plain-text inspection path remains `--print-effective-config`.

CAT status is a host-side read-only boundary in this milestone slice. The mock
backend is used for tests and TUI display without requiring a radio. The flrig
backend reads frequency, mode, and PTT state through XML-RPC when built with
`WITH_FLRIG=1`. The hamlib backend reads frequency, mode, and PTT state through
hamlib when built with `WITH_HAMLIB=1`. PTT control remains deferred.

## Restoration Analysis And Declipper

The M9.3 analyzer is disabled by default. Enable it with `--analyze`. It
observes the post-DC-blocker and post-dehummer signal before AGC and before the
optional declipper, then reports:

| Metric                                  | Meaning                                      |
| --------------------------------------- | -------------------------------------------- |
| `analysis_profile`                      | Wideband or limited-band source hint         |
| `analysis_reason_flags`                 | Bit field for active analyzer reasons        |
| `analysis_clip_ratio`                   | Samples near the configured clipping ceiling |
| `analysis_hf_ratio`                     | Simple high-frequency activity indicator     |
| `analysis_clip_confidence`              | Conservative hard-clipping suspicion score   |
| `analysis_low_ceiling_confidence`       | Flattened low-ceiling suspicion score        |
| `analysis_transient_confidence`         | Short near-peak event suspicion score        |
| `analysis_lossy_confidence`             | High-frequency-loss suspicion score          |
| `analysis_peak`                         | Highest absolute sample observed             |
| `analysis_crest`                        | Observed peak divided by RMS                 |
| `flat_runs`                             | Repeated near-peak flat sample runs          |
| `peak_repeats`                          | Repeated near-peak samples                   |

The analyzer does not modify audio by itself. A high score is not proof of
source damage, and a low score is not proof that a source is clean.

The optional M9.3 declipper is enabled with `--declipper`. It is disabled by
default. When enabled, it uses the analyzer metrics to decide whether a block is
safe to repair. It bypasses silence, non-finite analysis, low-confidence blocks,
and transient-like blocks. It reports:

| Metric                      | Meaning                                  |
| --------------------------- | ---------------------------------------- |
| `declipper`                 | Whether the repair stage is enabled      |
| `repaired_samples`          | Samples changed in the latest block      |
| `repaired_runs`             | Short clipped runs repaired              |
| `max_delta`                 | Largest sample change in the block       |
| `bypass`                    | Reason repair was bypassed               |
| `finite`                    | Whether output remained finite           |

The M9.3 declipper is a bounded clean-room research prototype for clear hard
clipping and low-ceiling clipping. It does not restore all clipped audio,
repair codecs, replace missing detail, remove noise, or implement a
delossifier.

`make quality` includes M9.3 fixtures for hard clipping, low-ceiling clipping,
short bursts, AM-limited bandwidth, SSB or voice-limited bandwidth, and
declipper repair gating. These fixtures are regression checks, not restoration
or compliance proof.

## AGC Controls

The M3 AGC is a single linked gain rider for mono or stereo blocks. It measures RMS, applies one shared gain value across channels, uses fast attack for sudden loud input, waits through hold time before release, and freezes gain during gated or silent input so silence does not push gain to max.

| Control              | Purpose                                      |
| -------------------- | -------------------------------------------- |
| `target_rms`         | Desired working RMS level                    |
| `min_gain`           | Lowest allowed linear gain                   |
| `max_gain`           | Highest allowed linear gain                  |
| `attack_ms`          | Normal gain reduction timing                 |
| `release_ms`         | Gain recovery timing                         |
| `fast_attack_ms`     | Faster reduction for sudden loud input       |
| `hold_ms`            | Delay before release after gain reduction    |
| `gate_threshold_db`  | Below this, gain movement is held            |
| `silence_threshold_db` | Below this, input is treated as silence     |
| `max_gain_step_db`   | Largest gain change per processed block      |
| `sample_rate`        | Timing reference for millisecond controls    |

Practical starting points for later tuning:

| Preset               | target_rms | min_gain | max_gain | attack_ms | release_ms | fast_attack_ms | hold_ms | gate_db | silence_db | max_step_db |
| -------------------- | ---------- | -------- | -------- | --------- | ---------- | -------------- | ------- | ------- | ---------- | ----------- |
| AM music             | 0.20       | 0.125    | 6.0      | 80        | 1800       | 8              | 250     | -45     | -70        | 4           |
| AM speech            | 0.18       | 0.125    | 8.0      | 40        | 1200       | 5              | 200     | -50     | -75        | 5           |
| SSB speech           | 0.16       | 0.125    | 10.0     | 30        | 900        | 4              | 150     | -55     | -78        | 6           |
| Gentle file levelling | 0.18      | 0.25     | 4.0      | 120       | 2500       | 20             | 300     | -50     | -75        | 2           |

## Dehummer

The M4 dehummer is an optional fixed-frequency hum reducer placed after the DC blocker and before AGC. Removing steady mains hum before AGC keeps the gain rider from reacting to hum energy.

Use `--hum-frequency 50` in regions with 50 Hz mains power, and `--hum-frequency 60` in regions with 60 Hz mains power. `--hum-harmonics N` adds matching notches at integer multiples of the base frequency, for example 50, 100, 150, and 200 Hz when `N` is 4.

`--hum-q Q` controls notch width. Higher Q values are narrower and usually safer for program audio. Lower Q values remove a wider band but can damage nearby wanted low-frequency content. CarrierPress defaults to conservative narrow notches and does not claim to remove all noise or provide forensic-quality restoration.

## Multiband

The M5 multiband mode is an optional first compressor foundation. It splits mono or stereo audio into 2 to 4 bands, applies simple linked per-band compression, meters each band, and recombines before the final limiter. The structs are sized so 2 to 9 bands can be added later, but v0.1 accepts only 2, 3, or 4 active bands.

Enable it with `--multiband`. The `speech` preset uses slightly stronger conservative compression for spoken audio. The `music` preset uses gentler ratios and slower timing. These are practical starting points, not final AM or SSB broadcast presets.

```sh
./carrierpress --input input.wav --output output.wav --multiband --multiband-bands 2 --multiband-preset speech
./carrierpress --input input.wav --output output.wav --multiband --multiband-bands 3 --multiband-preset music
```

M5 uses cascaded 2nd-order low-pass sections with subtractive band creation for the crossover scaffold. It is bounded and deterministic, but it is not final broadcast processing.

The M10.2 second multiband stage runs after bass EQ and before AM or SSB output
shaping. It uses the same 2 to 4 band structure as M5, but its presets use
gentler final-polish compression. Enable it with `--multiband2`.

```sh
./carrierpress --input input.wav --output output.wav --multiband --bass-eq --multiband2 --multiband2-bands 3 --multiband2-preset speech
./carrierpress --input input.wav --output output.wav --multiband2 --multiband2-bands 3 --multiband2-preset music
```

The second multiband stage is disabled by default and does not add 5 to 9 band
support yet.

## Auto EQ Analysis

The M10.3 auto EQ analysis tap runs after restoration and declipper analysis
and before dynamics, low-level boost, AGC, and compression. It measures fixed
tonal bands for bass, low midrange, midrange, presence, and high frequencies.
It reports total RMS, per-band RMS, relative band dB, spectral tilt, broad
weights, and a source hint such as silence, bass-heavy, thin, dark, bright,
balanced, or limited-band.

Enable it with `--auto-eq-analyze`:

```sh
./carrierpress --input input.wav --output output.wav --auto-eq-analyze
./carrierpress --live --auto-eq-analyze --meter-interval-ms 1000
./carrierpress --play input.wav --auto-eq-analyze --meter-interval-ms 1000
```

M10.3 is analysis-only. It does not apply automatic EQ, change bass EQ
settings, or claim tonal correction quality.

`--bass-eq-recommend` enables the same analyzer and prints a bounded bass EQ
recommendation. The recommendation includes a preset, bass shelf gain, presence
shelf gain, output gain, confidence, and source hint. It is operator guidance
only. CarrierPress does not apply the recommendation automatically.

## Bass EQ

The M8.1 bass EQ mode is a static two-shelf tone-shaping foundation. It runs after the first multiband compressor and before the optional second multiband stage. It is disabled by default and leaves audio unchanged unless `--bass-eq` is selected.

Enable it with `--bass-eq`. Presets are conservative starting points:

| Preset   | Purpose                                      |
| -------- | -------------------------------------------- |
| `flat`   | No shelf gain changes                        |
| `speech` | Slight low reduction and presence lift       |
| `music`  | Modest bass lift with small presence lift    |
| `warm`   | Mild low lift with softer presence           |

```sh
./carrierpress --input input.wav --output output.wav --bass-eq --bass-eq-preset music
./carrierpress --input input.wav --output output.wav --bass-eq --bass-gain-db 2.0 --bass-frequency 120 --presence-gain-db 0.5
```

M8.1 does not implement automatic EQ, immersive bass, true bass, or subharmonic synthesis. It is a static, bounded bass and presence EQ stage for later tuning work.

M10.4 adds recommendation output for the static bass EQ stage. Recommendations
are based on the auto EQ analyzer and are clamped to conservative values. They
are visible in self-test, WAV mode, live meters, playout meters, the TUI,
`make quality`, and `make professional-check`.

## AM Mode

The M6 AM mode is audio-chain processing for legal AM transmitters and dummy-load testing. It is not an RF exciter, modulator, transmitter controller, or certified compliance tool. Users must obey their licence terms, transmitter limits, occupied bandwidth limits, and local radio regulations.

Enable AM mode with `--am`. The AM chain runs after AGC, multiband, bass EQ, and the optional second multiband stage, then before the final limiter. It provides high-pass filtering, low-pass audio bandwidth limiting, optional phase rotation, positive and negative peak control, and explicitly configured positive asymmetry.

```sh
./carrierpress --input input.wav --output output.wav --am --am-preset am-safe
./carrierpress --input input.wav --output output.wav --am --am-preset am-shortwave
./carrierpress --input input.wav --output output.wav --am --am-lowpass 4500 --am-highpass 80
```

AM presets:

| Preset         | Purpose                                      |
| -------------- | -------------------------------------------- |
| `am-safe`      | Conservative mono-safe starting point        |
| `am-shortwave` | 5-7 MHz shortwave-style starting bandwidth   |
| `am-wide`      | Wider lab and dummy-load testing preset      |
| `am-voice`     | Narrower voice-focused preset                |

Use `--am-asymmetry FLOAT` only when positive asymmetry is explicitly wanted. The module allows up to 200 percent positive capability for experiments, but that is not a default and is not a compliance claim. Negative peaks remain strictly limited by `--am-negative-peak`.

## SSB Mode

The M7 SSB mode is audio-chain processing for legal SSB transmitters and dummy-load testing. It is not an RF exciter, USB/LSB modulator, transmitter controller, or certified compliance tool. Users must obey their licence terms, transmitter limits, occupied bandwidth limits, and local radio regulations.

Enable SSB mode with `--ssb`. The SSB chain runs after AGC, multiband, bass EQ, and the optional second multiband stage, then before the final limiter. It provides speech-oriented high-pass filtering, low-pass bandwidth limiting, optional phase rotation, and symmetric peak control. AM and SSB modes are mutually exclusive.

```sh
./carrierpress --input input.wav --output output.wav --ssb --ssb-preset ssb-speech
./carrierpress --input input.wav --output output.wav --ssb --ssb-preset ssb-narrow
./carrierpress --input input.wav --output output.wav --ssb --ssb-lowpass 2800 --ssb-highpass 150
```

SSB presets:

| Preset       | Purpose                                      |
| ------------ | -------------------------------------------- |
| `ssb-speech` | General voice intelligibility starting point |
| `ssb-narrow` | Tighter voice bandwidth                      |
| `ssb-wide`   | Wider lab and local testing preset           |
| `ssb-gentle` | Lighter shaping for already processed files  |

SSB mode does not add carrier generation, sideband modulation, VOX, CAT control, or transmitter keying. It shapes baseband audio only.

| Area       | v0.2 status                           |
| ---------- | ------------------------------------- |
| Core DSP   | Portable C17 float32 block API        |
| Offline IO | Optional WAV processing and reporting |
| Live audio | Optional PortAudio foundation         |
| OpenBSD IO | Optional sndio foundation, manual validation |
| MCU port   | Planned STM32H753/CMSIS-DSP, deferred |

## Roadmap

The v0.3 roadmap is documented in
[`docs/roadmap-v0.3.md`](docs/roadmap-v0.3.md). v0.3 focuses on operator
polish, effective profile/config inspection, safer batch offline WAV workflow,
report schema polish, GUI/TUI status clarity, library examples, and packaging
maintainer notes. PTT control remains deferred to the separate T5 safety gate.

The v0.4 roadmap is documented in
[`docs/roadmap-v0.4.md`](docs/roadmap-v0.4.md). v0.4 planning focuses on GUI
workflow parity, safer playout and device operations, report automation, API
and packaging hardening, and optional decoder research without adding decoder
libraries to the base build.

The v0.4.1 release notes are in
[`docs/release-notes-v0.4.1.md`](docs/release-notes-v0.4.1.md).
The v0.5 planning roadmap is documented in
[`docs/roadmap-v0.5.md`](docs/roadmap-v0.5.md). v0.5 planning focuses on
hardening, maintainability, operator workflow polish, and optional-feature
boundaries. T5 remains mock-only safety-gate scaffolding, and no hardware PTT
backend exists.
The product roadmap is documented in
[`docs/roadmap-product.md`](docs/roadmap-product.md). It tracks DSP product
polish, professional TUI/GUI layout work, interactive selectors, mock-only TX
operator-control planning, and future hardware TX as a separate safety track.

## License

CarrierPress is released under the ISC License.


###### Mirrors:

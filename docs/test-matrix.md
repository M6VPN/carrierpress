# CarrierPress Test Matrix

This guide documents ordinary and optional build/test coverage after v0.4.1.
It complements `docs/validation-targets.md`, which remains the source of
truth for serial versus parallel scheduling.

## Base Build Matrix

The base build is the required dependency-light path:

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
make transmit-control-safety-audit
make operator-workflow-safety-audit
make packaging-surface-audit
```

Base validation must not require libsndfile, PortAudio, sndio, SDL3, FFTW,
hamlib, flrig, ncurses, or decoder libraries. Base validation must not require
audio hardware.

The base build cannot transmit. It has no hardware PTT backend, CAT
write/control backend, hamlib or flrig PTT call, active TRANSMIT UI, or
profile/config/report/batch/playlist transmit arming path.

`make operator-workflow-safety-audit` checks ordinary operator-facing source
and examples for active transmit controls, release publication, package
installation, and backend control calls.

`make packaging-surface-audit` checks base pkg-config metadata,
dependency-light header tiers, and packaging policy docs.

## Guarded Mock Transmit-Control Matrix

Guarded transmit-control validation is mock-only and must be run serially:

```sh
make clean
make WITH_TRANSMIT_CONTROL=1
make WITH_TRANSMIT_CONTROL=1 test
make transmit-control-safety-audit
make transmit-control-mock-test
```

`WITH_TRANSMIT_CONTROL=1` remains a guarded mock state-machine build. It does
not add a hardware backend, CAT write/control backend, hamlib or flrig PTT
call, GUI/TUI/CLI active TRANSMIT control, or profile/config/report/batch/
playlist arming path.

`make transmit-control-mock-test` runs `make clean` internally. Do not run it
in parallel with any other build or test target.

## Optional Dependency Matrix

Optional feature flags stay opt-in. Missing optional dependencies should fail
with a clear package or library name and must not affect the base build.

| Feature flag                            | Dependency | Validation path |
| --------------------------------------- | ---------- | --------------- |
| `WITH_SNDFILE=1`                        | libsndfile | WAV processing and batch WAV tests |
| `WITH_PORTAUDIO=1`                      | PortAudio  | Live audio backend build path |
| `WITH_SNDFILE=1 WITH_PORTAUDIO=1`       | libsndfile and PortAudio | WAV playout tests |
| `WITH_TUI=1`                            | ncurses    | TUI formatting and control tests |
| `WITH_GUI=1`                            | SDL3       | GUI formatting, workflow, and monitor build tests |
| `WITH_GUI=1 WITH_FFTW=1`                | SDL3 and FFTW | GUI spectrum build and tests |
| `WITH_SNDIO=1`                          | sndio      | Optional sndio build tests, manual OpenBSD validation |
| `WITH_FLRIG=1`                          | POSIX socket headers | Read-only XML-RPC CAT tests |
| `WITH_HAMLIB=1`                         | hamlib     | Read-only hamlib CAT tests |
| `WITH_TRANSMIT_CONTROL=1`               | none beyond base | Mock-only transmit-control tests |

`WITH_FLRIG=1` and `WITH_HAMLIB=1` remain read-only CAT paths. They must not
add CAT write/control or PTT behavior.

`WITH_SNDIO=1` remains optional. OpenBSD and sndio device validation are manual
unless a stable CI runner is selected later.

Optional dependencies must not leak into base `carrierpress.pc` metadata. Use
`make pkg-config-smoke` and `make packaging-surface-audit` to check base
pkg-config output and source/docs package-surface policy.

## Optional Build Validation Commands

Run these only where the matching development libraries are installed:

```sh
make WITH_SNDFILE=1 test
make WITH_SNDFILE=1 WITH_PORTAUDIO=1 test
make WITH_GUI=1 test
make WITH_GUI=1 WITH_FFTW=1 test
make WITH_TUI=1 test
make WITH_SNDIO=1 test
make WITH_FLRIG=1 test
make WITH_HAMLIB=1 test
```

These commands are not required for ordinary base validation. They should stay
local/manual when dependencies are not available.

## CI and Local Expectations

GitHub Actions currently covers:

- base build, tests, validation, staged install, and release check
- libsndfile profile
- PortAudio profile
- ncurses TUI profile
- libsndfile plus PortAudio playout profile
- FFTW profile
- read-only flrig CAT profile
- read-only hamlib CAT profile
- packaging smoke and source archive checksum
- packaging surface audit
- SDL3 GUI profile when runner packages are available

The SDL3 GUI job is allowed to skip when runner packages are unavailable. Full
GUI and audio-device checks remain local/manual.

Project scripts must not install packages outside CI setup, use `sudo` outside
CI setup, create tags, push tags, publish releases, upload artifacts, access
hardware, or perform transmit/CAT write-control actions.

## Help Target

Print a concise matrix without building or mutating the tree:

```sh
make test-matrix-help
make packaging-help
```

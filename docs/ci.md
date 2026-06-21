# CarrierPress CI

CarrierPress uses GitHub Actions for build, test, validation, and staged
install checks. The workflow is a safety check only. It does not create tags,
push tags, publish releases, key radios, or run transmitter-control commands.

## Workflow

The workflow lives at `.github/workflows/ci.yml` and runs on pushes, pull
requests, and manual dispatch.

## Base Validation

The base job runs dependency-light validation:

```sh
make clean
make
make test
make -j test
make validate
make quality
make professional-check
make install-smoke
./carrierpress --version
make release-check
```

The base job does not require libsndfile, PortAudio, ncurses, SDL3, FFTW,
hamlib, flrig, sndio, local radio hardware, or local vendored dependencies.

## Optional Dependency Profiles

The optional job installs development packages through GitHub Actions only and
runs selected optional profiles:

```sh
make autodetect
make WITH_SNDFILE=1 test
make WITH_PORTAUDIO=1
make WITH_TUI=1 test
make WITH_SNDFILE=1 WITH_PORTAUDIO=1 test
make WITH_FFTW=1 test
make WITH_FLRIG=1 test
make WITH_HAMLIB=1 test
```

These profiles validate optional build paths. The project Makefile and scripts
still do not install packages.

## GUI Profile

SDL3 package availability can vary across Ubuntu runners. The GUI job tries to
install `libsdl3-dev` and runs:

```sh
make WITH_GUI=1 test
make WITH_GUI=1 WITH_FFTW=1 test
```

If SDL3 is unavailable on the runner, the GUI job prints a local validation
note instead of blocking the rest of CI. Local GUI validation remains:

```sh
make WITH_GUI=1 WITH_FFTW=1 test
./carrierpress --gui-demo-screenshot build/gui-demo.bmp --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off
```

Visual checks remain manual. CI does not do pixel comparison.

## OpenBSD and sndio

OpenBSD and sndio validation is manual unless a stable OpenBSD runner is added.
The Linux CI path does not make sndio mandatory and does not use sndio as the
active Linux audio backend. Use `docs/openbsd-sndio.md` for the manual build
and hardware checklist.

## Packaging Smoke

The packaging job checks a staged install and uninstall:

```sh
make clean
make
make install-smoke
make install-manifest
make DESTDIR=build/stage PREFIX=/usr install
make DESTDIR=build/stage PREFIX=/usr uninstall
make dist-check
```

It verifies the staged binary, static library, pkg-config file, man page, and
public header directory.

## Release Boundary

CI does not publish releases. Release tags and GitHub releases remain manual
per `docs/release-procedure.md`.

The pushed `v0.1.0` tag was created before the R3 packaging polish. R3, R4,
and R5 are post-v0.1.0 hardening and are prepared for the `v0.1.1` patch
release.

## CAT and PTT Boundary

CAT validation in CI is read-only. flrig and hamlib profiles validate status
readback code and parser paths only. No PTT control is implemented, and no CAT
write/control commands are run in CI.

The local Hamlib 4.7.1 path at `/home/dgm/vendored/hamlib-4.7.1` is for local
validation only and is not used by CI.

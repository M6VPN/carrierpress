# Packaging Notes

CarrierPress packaging should keep the base DSP build dependency-light and keep
all host backends optional. Packaging must not publish releases, create tags,
install system packages, change audio system configuration, key a radio, or add
transmitter-control behavior.

CarrierPress is baseband audio processing software. It is not an RF generator,
transmitter controller, certified compliance tool, licence-compliance tool, or
legal-bandwidth proof.

## Installed Files

A staged install with `DESTDIR="$PWD/build/stage" PREFIX=/usr` installs:

```text
/usr/bin/carrierpress
/usr/include/carrierpress/*.h
/usr/lib/libcarrierpress.a
/usr/lib/pkgconfig/carrierpress.pc
/usr/share/man/man1/carrierpress.1
```

`carrierpress_core.h` is the recommended installed header for new in-memory DSP
library users. `carrierpress_tooling.h` is the recommended installed header for
profile, config, batch, and report tooling. `carrierpress.h` remains installed
as the broad compatibility umbrella.

Review the exact staged file list with:

```sh
make install-manifest
```

## Build Options

Base package build:

```sh
make
```

Optional feature builds:

```sh
make WITH_SNDFILE=1
make WITH_SNDFILE=1 WITH_PORTAUDIO=1
make WITH_TUI=1
make WITH_SNDIO=1
make WITH_GUI=1 WITH_FFTW=1
make WITH_HAMLIB=1
make WITH_FLRIG=1
```

Optional features must stay optional. A base package should not require
libsndfile, PortAudio, sndio, ncurses, SDL3, FFTW, hamlib, or flrig.

Packagers may split optional host interfaces into separate packages, but the
base package should remain usable for the dependency-light CLI and DSP tests.

## DESTDIR and PREFIX

Use staged installs for packaging review:

```sh
make clean
make
make DESTDIR="$PWD/build/stage" PREFIX=/usr install
make DESTDIR="$PWD/build/stage" PREFIX=/usr uninstall
```

The install target does not use `sudo` and does not require root when `DESTDIR`
points inside the repository.

## pkg-config

After a staged install, test pkg-config metadata with:

```sh
PKG_CONFIG_PATH="$PWD/build/stage/usr/lib/pkgconfig" pkg-config --modversion carrierpress
PKG_CONFIG_PATH="$PWD/build/stage/usr/lib/pkgconfig" pkg-config --cflags --libs carrierpress
```

The base `carrierpress.pc` file should expose the installed headers, static
library, and `-lm` only. Optional host dependencies should not be added to the
base pkg-config metadata.

## Man Page

Check the staged man page with:

```sh
man -l build/stage/usr/share/man/man1/carrierpress.1
```

## Source Archive and Checksum

Create and verify a local source archive:

```sh
make dist-check
ls -lh build/dist/
```

`make dist` uses `git archive` from `HEAD`, writes
`build/dist/carrierpress-0.2.0.tar.gz`, and writes a `.sha256` file. It does
not create tags, push tags, call GitHub tools, or publish a release.

Commit intended release changes before using the archive as a release source,
because `git archive` reads committed `HEAD`, not uncommitted working-tree
files.

On platforms without `sha256sum`, use the platform checksum tool manually, such
as OpenBSD `sha256`, and record the result with the release evidence.

## Packaging Policy

- No post-install scripts are required.
- No daemon or service is installed.
- No udev, sndio, ALSA, PipeWire, PulseAudio, or JACK configuration is changed.
- No setuid or setcap is required.
- No release publication is automated.
- No RF, transmitter, licence, regulatory, legal-bandwidth, or broadcast-quality
  claims are made by package metadata.

## Maintainer Notes

Package maintainer checklists and example packaging notes are split into
focused documents:

- [`package-maintainer-checklist.md`](package-maintainer-checklist.md) covers
  source input, staged install review, pkg-config checks, optional split
  package ideas, and safety policy checks.
- [`packaging-debian-notes.md`](packaging-debian-notes.md) gives Debian-style
  packaging notes without adding a real `debian/` directory.
- [`packaging-openbsd-notes.md`](packaging-openbsd-notes.md) gives OpenBSD
  ports-style notes without adding a real ports tree.
- [`packaging-portable-notes.md`](packaging-portable-notes.md) covers portable
  source builds, local tarballs, and checksum review.

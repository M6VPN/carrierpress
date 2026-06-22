# OpenBSD Ports-Style Packaging Notes

These notes are documentation only. They are not an OpenBSD ports tree and
they do not publish releases.

CarrierPress is baseband audio processing software. Package descriptions must
not claim RF generation, transmitter compliance, licence compliance,
regulatory approval, legal bandwidth, or broadcast-quality proof. CAT support
is read-only and PTT control is not implemented.

## Build Notes

The base build should remain dependency-light:

```sh
make clean
make
make test
```

If the packaging environment requires a specific make implementation, document
that in the package recipe. Keep the upstream project build path unchanged.

## sndio Option

sndio support is optional:

```sh
make WITH_SNDIO=1
make WITH_SNDIO=1 test
```

Use named sndio devices with `--device NAME`. See
[`openbsd-sndio.md`](openbsd-sndio.md) for manual validation notes.

## Staged Install

Use `DESTDIR` and `PREFIX`:

```sh
make DESTDIR="$PWD/build/stage" PREFIX=/usr/local install
make install-manifest
make public-header-smoke
make pkg-config-smoke
```

Do not add scripts that change system audio configuration. Do not add
services, daemons, setuid, setcap, or post-install behavior.

The base pkg-config metadata should expose only `-lcarrierpress -lm`.
`carrierpress_core.h` is the recommended dependency-light DSP header, and
`carrierpress_tooling.h` is the recommended dependency-light tooling header.

## Source Archive and Checksum

Source archives are created locally with:

```sh
make dist-check
```

On OpenBSD, `sha256` can be used manually if `sha256sum` is not available:

```sh
sha256 build/dist/carrierpress-0.4.1.tar.gz
```

The source archive workflow is local and manual. It does not create tags, push
tags, upload artifacts, or publish releases.

## Optional Dependencies

Optional package flavors may map to libsndfile, sndio, ncurses, SDL3, FFTW,
hamlib, or read-only flrig CAT support. Keep the base package independent from
those libraries unless the package maintainer deliberately selects a split or
flavor.

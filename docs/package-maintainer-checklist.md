# Package Maintainer Checklist

This checklist is for maintainers preparing local packages from CarrierPress
source releases or committed source trees. It is not a distro packaging recipe
and it does not publish releases.

CarrierPress is baseband audio processing software. Package metadata must not
claim RF generation, transmitter compliance, licence compliance, regulatory
approval, legal bandwidth, or broadcast-quality proof. CAT support is read-only
and PTT control is not implemented.

## Source Input

- Start from a signed or reviewed tag, release archive, or committed source
  tree.
- Verify the source archive checksum before building.
- Use `make dist-check` only as a local/manual source archive check.
- Note that `make dist-check` uses committed `HEAD`.
- Do not rely on uncommitted working-tree files for release archives.
- Do not create tags, push tags, upload artifacts, or publish releases from
  packaging scripts.

## Base Build

Run the dependency-light build and validation profile:

```sh
make clean
make
make test
make validate
make quality
make quality-json
make professional-check
```

These checks validate software behavior and regression fixtures. They are not
transmitter, RF, or regulatory compliance evidence.

## Install Layout

Review the staged install:

```sh
make DESTDIR="$PWD/build/stage" PREFIX=/usr install
make install-manifest
```

Expected staged files:

```text
/usr/bin/carrierpress
/usr/include/carrierpress/*.h
/usr/lib/libcarrierpress.a
/usr/lib/pkgconfig/carrierpress.pc
/usr/share/man/man1/carrierpress.1
```

The Makefile install target must be run with `DESTDIR` for package staging. It
does not require root and must not use `sudo`.

## pkg-config

Check the staged pkg-config metadata:

```sh
PKG_CONFIG_PATH="$PWD/build/stage/usr/lib/pkgconfig" pkg-config --modversion carrierpress
PKG_CONFIG_PATH="$PWD/build/stage/usr/lib/pkgconfig" pkg-config --cflags --libs carrierpress
```

The base `carrierpress.pc` metadata should expose only the base static library
and math library:

```text
Libs: -L${libdir} -lcarrierpress -lm
```

Optional dependencies must not leak into the base pkg-config file.

## Optional Feature Packages

Maintainers may choose split packages. These names are examples only:

- `carrierpress`: base CLI and static library.
- `carrierpress-wav`: optional libsndfile WAV processing build.
- `carrierpress-playout`: optional libsndfile plus PortAudio playout build.
- `carrierpress-tui`: optional ncurses operator interface.
- `carrierpress-gui`: optional SDL3 GUI monitor, with FFTW if selected.
- `carrierpress-sndio`: optional sndio validation build.
- `carrierpress-cat-readonly`: optional hamlib/flrig read-only CAT support.

Optional features must remain optional. A base package should not require
libsndfile, PortAudio, sndio, ncurses, SDL3, FFTW, hamlib, or flrig.

## Safety and Policy Checks

- Do not add services, daemons, timers, or background jobs.
- Do not add post-install audio configuration.
- Do not add udev, ALSA, PipeWire, PulseAudio, JACK, or sndio system changes.
- Do not add setuid, setcap, or privilege changes.
- Do not add PTT control or CAT write/control behavior.
- Do not add release publication to package scripts.
- Do not add package metadata that claims RF generation, transmitter
  compliance, licence compliance, regulatory approval, legal bandwidth, or
  broadcast-quality proof.

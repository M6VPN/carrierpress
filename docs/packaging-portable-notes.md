# Portable Source Packaging Notes

These notes cover source builds outside a distro package recipe. They do not
publish releases or install system packages.

CarrierPress is baseband audio processing software. Source packages and local
install notes must not claim RF generation, transmitter compliance, licence
compliance, regulatory approval, legal bandwidth, or broadcast-quality proof.
CAT support is read-only and PTT control is not implemented.

## Source Tarball and Checksum

Create and verify a local source archive from committed `HEAD`:

```sh
make dist-check
```

Verify the checksum with GNU coreutils:

```sh
sha256sum -c build/dist/carrierpress-0.4.1.tar.gz.sha256
```

On OpenBSD or systems without `sha256sum`, use the platform checksum tool:

```sh
sha256 build/dist/carrierpress-0.4.1.tar.gz
```

The archive workflow is local and manual. It does not create tags, push tags,
upload artifacts, call GitHub tools, or publish releases.

## Build and Test

Build the base tree:

```sh
make clean
make
make test
make validate
make quality
make quality-json
make professional-check
make public-header-smoke
make pkg-config-smoke
make packaging-surface-audit
```

Optional features remain explicit:

```sh
make WITH_SNDFILE=1
make WITH_PORTAUDIO=1
make WITH_TUI=1
make WITH_SNDIO=1
make WITH_GUI=1 WITH_FFTW=1
make WITH_HAMLIB=1
make WITH_FLRIG=1
```

If an optional dependency is missing, install it manually with the platform
tools. CarrierPress project scripts do not install packages.

The base pkg-config metadata should expose only `-lcarrierpress -lm`.
New library examples should prefer `carrierpress_core.h` for in-memory DSP
processing and `carrierpress_tooling.h` for profile, config, batch, and report
tooling.
`cp_transmit_control.h` is guarded mock-only T5 scaffolding, not an operational
PTT API.

## Staged Install

Use a local staged install before packaging or copying files:

```sh
make DESTDIR="$PWD/build/stage" PREFIX=/usr install
make install-manifest
```

Review `build/stage` before copying files into a system prefix. Do not use
post-install scripts to change audio devices, services, privileges, or radio
control behavior.

## Systems Without GNU Tools

Use the platform equivalents for:

- `sha256sum`, such as OpenBSD `sha256`.
- `tar` listing and extraction checks.
- `make`, if the platform separates BSD make and GNU make.

Keep any local adaptations outside the upstream source tree unless they are
portable and explicitly selected for implementation.

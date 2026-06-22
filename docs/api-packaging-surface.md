# API and Packaging Surface

This guide records the v0.4.1 public API and package surface that packagers and
library users should expect. It is an inspection guide, not a packaging recipe.

CarrierPress is baseband audio processing software. It does not generate RF,
provide transmitter control, certify transmitter behavior, prove licence
compliance, prove legal bandwidth, or provide regulatory approval.

## Public Header Tiers

`carrierpress_core.h` is the dependency-light DSP umbrella for in-memory
processing. It must not include optional host backend, GUI/TUI, decoder,
CAT write/control, or hardware transmit headers. New library users should
prefer this header when they need block processing, presets, meters, and monitor
snapshots.

`carrierpress_tooling.h` is the dependency-light tooling umbrella for profiles,
config files, batch planning, and report helpers. It must not require optional
host backends.

`carrierpress.h` is the broad compatibility umbrella. It remains installed for
existing source code and includes more public headers than most new
applications need. New code should prefer `carrierpress_core.h` or
`carrierpress_tooling.h` where practical.

`cp_transmit_control.h` is the T5 guarded, mock-only API surface. It is not an
operational PTT API. Ordinary builds report transmit control unavailable, and
`WITH_TRANSMIT_CONTROL=1` remains a mock state-machine test path. No hardware
backend, CAT write/control backend, GUI/TUI/CLI TRANSMIT control, or
profile/config/report/batch/playlist arming path exists.

## Installed Files

A staged install with `DESTDIR="$PWD/build/stage" PREFIX=/usr` should contain:

```text
/usr/bin/carrierpress
/usr/include/carrierpress/*.h
/usr/lib/libcarrierpress.a
/usr/lib/pkgconfig/carrierpress.pc
/usr/share/man/man1/carrierpress.1
```

Review the exact staged file list with:

```sh
make install-manifest
```

`make install-manifest` depends on `make install-smoke` and rewrites
`build/stage`, so run it serially.

## Base pkg-config Contract

The base `carrierpress.pc` metadata should expose the static library and math
library only:

```text
Libs: -L${libdir} -lcarrierpress -lm
Cflags: -I${includedir}/carrierpress
```

The base metadata must not expose optional dependencies:

- libsndfile
- PortAudio
- sndio
- SDL3
- FFTW
- hamlib
- flrig
- ncurses
- decoder libraries

Check this with:

```sh
make pkg-config-smoke
make packaging-surface-audit
```

## Optional Dependency Policy

Optional features remain opt-in. Missing optional dependencies may fail the
matching optional build with a clear package or library name. Missing optional
dependencies must not affect the base build, public-header smoke tests, or base
pkg-config metadata.

Packagers may split optional host interfaces into separate packages or flavors
when they choose. The base package should remain dependency-light.

## Packaging Safety Policy

CarrierPress packaging should not add:

- post-install scripts
- daemons, services, timers, or background jobs
- udev, ALSA, PipeWire, PulseAudio, JACK, or sndio configuration changes
- setuid or setcap requirements
- package release publication
- hardware transmit behavior
- CAT write/control behavior
- RF, transmitter, licence, regulatory, legal-bandwidth, or broadcast-quality
  claims

Use `make packaging-help` for a concise local packaging-check guide.

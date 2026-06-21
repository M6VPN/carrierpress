# Debian-Style Packaging Notes

These notes are documentation only. They are not a `debian/` packaging
directory and they do not publish releases.

CarrierPress is baseband audio processing software. Package descriptions must
not claim RF generation, transmitter compliance, licence compliance,
regulatory approval, legal bandwidth, or broadcast-quality proof. CAT support
is read-only and PTT control is not implemented.

## Base Build Dependencies

Likely base build requirements:

- C compiler.
- `make`.
- libc development environment.
- system math library.

The base package should not require optional audio, GUI, spectrum, CAT, or file
I/O development libraries.

## Optional Build Dependencies

Optional profiles may need:

- libsndfile development package for WAV processing.
- PortAudio development package for live audio and playout.
- ncurses development package for the TUI.
- SDL3 development package for the GUI monitor.
- FFTW development package for the GUI spectrum monitor.
- hamlib development package for read-only hamlib CAT.

flrig support uses the small built-in read-only XML-RPC client and does not
require a linked flrig library.

## Staged Install Pattern

Use a staged install:

```sh
make clean
make
make DESTDIR="$PWD/debian/tmp" PREFIX=/usr install
```

Package scripts should not install packages, use `sudo`, publish releases, or
modify system audio configuration.

## Package Metadata Wording

Prefer wording such as:

```text
baseband audio DSP processor for AM/SSB audio-chain experiments
```

Avoid wording such as:

- transmitter compliance.
- broadcast certified.
- legal bandwidth.
- regulatory approval.
- RF generator.

## Split Package Ideas

These are optional naming ideas only:

- base CLI/static library package.
- optional WAV/libsndfile package.
- optional PortAudio playout package.
- optional ncurses TUI package.
- optional SDL3/FFTW GUI package.
- optional read-only CAT package.

CarrierPress currently installs a static library. Shared-library packaging
should wait until shared-library support and ABI policy are explicitly added.

No daemon, service, post-install script, udev rule, setuid bit, or setcap rule
is required.

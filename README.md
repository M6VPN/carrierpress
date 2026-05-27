# CarrierPress

CarrierPress is a portable C DSP skeleton for real-time and offline AM and SSB audio processing. v0.1 provides a clean-room core with block processing, float32 samples, a DC blocker, RMS and peak meters, a slow AGC, and a safe peak limiter.

The long-term goal is AM/SSB audio processing for legal transmitters and test loads. Users are responsible for complying with radio regulations, transmitter licence limits, occupied bandwidth limits, and local operating rules.

Offline WAV processing is available as an optional M1 foundation when built with libsndfile. Live sound-card I/O, AM presets, SSB shaping, and STM32H753 support are planned but are not part of v0.1.

## Table of Contents

- [Requirements](#requirements)
- [Setup](#setup)
- [Usage](#usage)
- [Development](#development)
- [License](#license)

## Requirements

- C17 compiler such as `cc`, `clang`, or `gcc`
- `make`
- Standard C math library
- Optional [libsndfile](https://libsndfile.github.io/libsndfile/) development package for WAV processing

The DSP core builds without PortAudio or libsndfile. If your system is missing optional WAV support, install the libsndfile development package manually. Common package names are `libsndfile1-dev`, `libsndfile-devel`, or `libsndfile`.

## Setup

Build the CLI and core objects:

```sh
make
```

Build and run the normal test set:

```sh
make test
```

Run the test target with parallel jobs:

```sh
make -j test
```

Build with optional WAV support:

```sh
make WITH_SNDFILE=1
```

Build and run tests with optional WAV support:

```sh
make WITH_SNDFILE=1 test
```

## Usage

Run the built-in synthetic tone through the v0.1 chain:

```sh
./carrierpress --self-test
```

The command prints input and output meter values plus the current AGC gain.

Process a mono or stereo WAV file through the current chain:

```sh
./carrierpress --input input.wav --output output.wav
```

WAV support requires a `WITH_SNDFILE=1` build. Without it, the command exits with:

```text
WAV support not enabled. Rebuild with WITH_SNDFILE=1.
```

## Development

The core library has no optional audio backend dependency. WAV support lives in `cp_wav.c` and is compiled only when requested. Process functions use caller-owned buffers and explicit state structs so real-time callbacks can remain malloc-free and deterministic.

| Area       | v0.1 status                          |
| ---------- | ------------------------------------ |
| Core DSP   | Portable C17 float32 block API       |
| Offline IO | Optional WAV foundation in progress  |
| Live audio | Planned PortAudio backend for M2     |
| OpenBSD IO | Planned sndio backend for M10        |
| MCU port   | Planned STM32H753/CMSIS-DSP for M11  |

## License

CarrierPress is released under the ISC License.


###### Mirrors:

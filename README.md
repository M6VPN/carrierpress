# CarrierPress

CarrierPress is a portable C DSP skeleton for real-time and offline AM and SSB audio processing. v0.1 provides a clean-room core with block processing, float32 samples, a DC blocker, RMS and peak meters, a slow AGC, and a safe peak limiter.

The long-term goal is AM/SSB audio processing for legal transmitters and test loads. Users are responsible for complying with radio regulations, transmitter licence limits, occupied bandwidth limits, and local operating rules.

Live sound-card I/O, AM presets, SSB shaping, and STM32H753 support are planned but are not part of v0.1.

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

No PortAudio or libsndfile dependency is used in v0.1. If your system is missing the build tools, install the compiler and make package for your operating system manually.

## Setup

Build the CLI and core objects:

```sh
make
```

Run unit tests:

```sh
make test
```

## Usage

Run the built-in synthetic tone through the v0.1 chain:

```sh
./carrierpress --self-test
```

The command prints input and output meter values plus the current AGC gain.

## Development

The core library has no optional audio backend dependency. Process functions use caller-owned buffers and explicit state structs so real-time callbacks can remain malloc-free and deterministic.

| Area       | v0.1 status                         |
| ---------- | ----------------------------------- |
| Core DSP   | Portable C17 float32 block API      |
| Offline IO | Planned for M1                      |
| Live audio | Planned PortAudio backend for M2    |
| OpenBSD IO | Planned sndio backend for M10       |
| MCU port   | Planned STM32H753/CMSIS-DSP for M11 |

## License

CarrierPress is released under the ISC License.


###### Mirrors:

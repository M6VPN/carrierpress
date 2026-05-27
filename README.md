# CarrierPress

CarrierPress is a portable C DSP skeleton for real-time and offline AM and SSB audio processing. v0.1 provides a clean-room core with block processing, float32 samples, a DC blocker, RMS and peak meters, a slow AGC, and a safe peak limiter.

The long-term goal is AM/SSB audio processing for legal transmitters and test loads. Users are responsible for complying with radio regulations, transmitter licence limits, occupied bandwidth limits, and local operating rules.

Offline WAV processing is available as an optional M1 foundation when built with libsndfile. Experimental live sound-card I/O is available as an optional M2 foundation when built with PortAudio. AM presets, SSB shaping, and STM32H753 support are planned but are not part of v0.1.

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
- Optional [PortAudio](https://www.portaudio.com/) development package for live USB sound-card processing

The DSP core builds without PortAudio or libsndfile. If your system is missing optional WAV support, install the libsndfile development package manually. Common package names are `libsndfile1-dev`, `libsndfile-devel`, or `libsndfile`. If your system is missing optional live audio support, install the PortAudio development package manually. Common package names are `portaudio19-dev`, `portaudio-devel`, or `portaudio`.

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

Build with optional PortAudio live audio support:

```sh
make WITH_PORTAUDIO=1
```

Build with both optional WAV and PortAudio support:

```sh
make WITH_SNDFILE=1 WITH_PORTAUDIO=1
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

List PortAudio devices:

```sh
./carrierpress --list-devices
```

Run experimental live processing with default devices:

```sh
./carrierpress --live
```

Run live processing with selected USB sound-card devices:

```sh
./carrierpress --live --input-device 2 --output-device 3
```

Run live processing with explicit stream settings:

```sh
./carrierpress --live --sample-rate 48000 --channels 2 --block-size 256
```

Print meters once per second:

```sh
./carrierpress --live --meter-interval-ms 1000
```

PortAudio support requires a `WITH_PORTAUDIO=1` build. Without it, live and device-list commands exit with:

```text
PortAudio support not enabled. Rebuild with WITH_PORTAUDIO=1.
```

Live mode is experimental. It is a USB sound-card backend for testing the existing DC blocker, AGC, limiter, and metering chain. It is not a broadcast-quality processor.

## Manual Live-Audio Test

Build with PortAudio:

```sh
make WITH_PORTAUDIO=1
```

List devices:

```sh
./carrierpress --list-devices
```

Run live passthrough processing with the chosen devices:

```sh
./carrierpress --live --input-device 2 --output-device 3 --sample-rate 48000 --channels 2 --block-size 256
```

Check that meter lines update while audio is present. Stop with `Ctrl-C`.

## Development

The core library has no optional audio backend dependency. WAV support lives in `cp_wav.c`, and PortAudio support lives in `cp_portaudio.c`. Both are compiled only when requested. Process functions use caller-owned buffers and explicit state structs so real-time callbacks can remain malloc-free and deterministic.

| Area       | v0.1 status                          |
| ---------- | ------------------------------------ |
| Core DSP   | Portable C17 float32 block API       |
| Offline IO | Optional WAV foundation in progress  |
| Live audio | Optional PortAudio foundation        |
| OpenBSD IO | Planned sndio backend for M10        |
| MCU port   | Planned STM32H753/CMSIS-DSP for M11  |

## License

CarrierPress is released under the ISC License.


###### Mirrors:

# CarrierPress

CarrierPress is a portable C DSP skeleton for real-time and offline AM and SSB audio processing. v0.1 provides a clean-room core with block processing, float32 samples, a DC blocker, RMS and peak meters, a gated input AGC, an optional dehummer, a simple optional multiband compressor foundation, and a safe peak limiter.

The long-term goal is AM/SSB audio processing for legal transmitters and test loads. Users are responsible for complying with radio regulations, transmitter licence limits, occupied bandwidth limits, and local operating rules.

Offline WAV processing is available as an optional M1 foundation when built with libsndfile. Experimental live sound-card I/O is available as an optional M2 foundation when built with PortAudio. AM output-chain shaping is available as an M6 foundation. SSB shaping and STM32H753 support are planned but are not part of v0.1.

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
- Optional [ncurses](https://invisible-island.net/ncurses/) development package for the live TUI monitor

The DSP core builds without PortAudio, libsndfile, or ncurses. If your system is missing optional WAV support, install the libsndfile development package manually. Common package names are `libsndfile1-dev`, `libsndfile-devel`, or `libsndfile`. If your system is missing optional live audio support, install the PortAudio development package manually. Common package names are `portaudio19-dev`, `portaudio-devel`, or `portaudio`. If your system is missing optional TUI support, install the ncurses development package manually. Common package names are `libncurses-dev`, `ncurses-devel`, or `ncurses`.

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

Build with optional PortAudio live audio and the ncurses TUI monitor:

```sh
make WITH_PORTAUDIO=1 WITH_TUI=1
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

The command prints input and output meter values plus AGC gain, gain dB, and gate state.

Run the same self-test with the dehummer enabled:

```sh
./carrierpress --self-test --dehummer --hum-frequency 50 --hum-harmonics 4
```

Run the self-test with the M5 multiband compressor enabled:

```sh
./carrierpress --self-test --multiband --multiband-bands 3 --multiband-preset speech
```

Run the self-test with the M6 AM output chain enabled:

```sh
./carrierpress --self-test --am --am-preset am-safe
./carrierpress --self-test --am --am-preset am-shortwave
./carrierpress --self-test --dehummer --multiband --multiband-bands 3 --am --am-preset am-shortwave
```

Process a mono or stereo WAV file through the current chain:

```sh
./carrierpress --input input.wav --output output.wav
```

Process a WAV file with 60 Hz hum notches and four harmonics:

```sh
./carrierpress --input input.wav --output output.wav --dehummer --hum-frequency 60 --hum-harmonics 4
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

Run live processing with conservative 50 Hz dehumming:

```sh
./carrierpress --live --dehummer --hum-frequency 50 --hum-harmonics 4 --hum-q 35
```

Run live processing with the simple speech multiband preset:

```sh
./carrierpress --live --multiband --multiband-bands 3 --multiband-preset speech
```

Run live processing with AM-safe output shaping:

```sh
./carrierpress --live --am --am-preset am-safe
```

Run live processing with the ncurses monitor:

```sh
./carrierpress --live --tui
```

Run live AM processing with the ncurses monitor and preset keys:

```sh
./carrierpress --live --tui --am
```

The live TUI supports safe AM preset switching while audio is running. Press `0`
for AM off, `1` for `am-safe`, `2` for `am-shortwave`, `3` for `am-wide`,
`4` for `am-voice`, and `q` to stop. Preset changes are validated and applied at
audio block boundaries.

Print meters once per second:

```sh
./carrierpress --live --meter-interval-ms 1000
```

PortAudio support requires a `WITH_PORTAUDIO=1` build. Without it, live and device-list commands exit with:

```text
PortAudio support not enabled. Rebuild with WITH_PORTAUDIO=1.
```

TUI support requires a `WITH_TUI=1` build. Without it, `--tui` exits with:

```text
TUI support not enabled. Rebuild with WITH_TUI=1.
```

Live mode is experimental. It is a USB sound-card backend for testing the existing DC blocker, AGC, limiter, and metering chain. It is not a broadcast-quality processor.

## Manual Live-Audio Test

Build with PortAudio:

```sh
make WITH_PORTAUDIO=1
```

Build with PortAudio and the TUI monitor:

```sh
make WITH_PORTAUDIO=1 WITH_TUI=1
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

Run the TUI monitor:

```sh
./carrierpress --live --tui --input-device 2 --output-device 3 --sample-rate 48000 --channels 2 --block-size 256
```

Check that input/output meters, AGC state, stream flags, multiband meters, and AM settings update. Press `q` or `Ctrl-C` to stop.

For live AM control testing, add `--am` and press `0` through `4` to switch AM
off or select one of the validated AM presets.

## Development

The core library has no optional audio backend dependency. WAV support lives in `cp_wav.c`, PortAudio support lives in `cp_portaudio.c`, and the ncurses monitor lives in `cp_tui.c`. Optional files are compiled only when requested. Process functions use caller-owned buffers and explicit state structs so real-time callbacks can remain malloc-free and deterministic.

Live TUI control is preset-only in this milestone. It can switch the AM output
chain between validated presets, but it does not expose arbitrary DSP parameter
editing.

## AGC Controls

The M3 AGC is a single linked gain rider for mono or stereo blocks. It measures RMS, applies one shared gain value across channels, uses fast attack for sudden loud input, waits through hold time before release, and freezes gain during gated or silent input so silence does not push gain to max.

| Control              | Purpose                                      |
| -------------------- | -------------------------------------------- |
| `target_rms`         | Desired working RMS level                    |
| `min_gain`           | Lowest allowed linear gain                   |
| `max_gain`           | Highest allowed linear gain                  |
| `attack_ms`          | Normal gain reduction timing                 |
| `release_ms`         | Gain recovery timing                         |
| `fast_attack_ms`     | Faster reduction for sudden loud input       |
| `hold_ms`            | Delay before release after gain reduction    |
| `gate_threshold_db`  | Below this, gain movement is held            |
| `silence_threshold_db` | Below this, input is treated as silence     |
| `max_gain_step_db`   | Largest gain change per processed block      |
| `sample_rate`        | Timing reference for millisecond controls    |

Practical starting points for later tuning:

| Preset               | target_rms | min_gain | max_gain | attack_ms | release_ms | fast_attack_ms | hold_ms | gate_db | silence_db | max_step_db |
| -------------------- | ---------- | -------- | -------- | --------- | ---------- | -------------- | ------- | ------- | ---------- | ----------- |
| AM music             | 0.20       | 0.125    | 6.0      | 80        | 1800       | 8              | 250     | -45     | -70        | 4           |
| AM speech            | 0.18       | 0.125    | 8.0      | 40        | 1200       | 5              | 200     | -50     | -75        | 5           |
| SSB speech           | 0.16       | 0.125    | 10.0     | 30        | 900        | 4              | 150     | -55     | -78        | 6           |
| Gentle file levelling | 0.18      | 0.25     | 4.0      | 120       | 2500       | 20             | 300     | -50     | -75        | 2           |

## Dehummer

The M4 dehummer is an optional fixed-frequency hum reducer placed after the DC blocker and before AGC. Removing steady mains hum before AGC keeps the gain rider from reacting to hum energy.

Use `--hum-frequency 50` in regions with 50 Hz mains power, and `--hum-frequency 60` in regions with 60 Hz mains power. `--hum-harmonics N` adds matching notches at integer multiples of the base frequency, for example 50, 100, 150, and 200 Hz when `N` is 4.

`--hum-q Q` controls notch width. Higher Q values are narrower and usually safer for program audio. Lower Q values remove a wider band but can damage nearby wanted low-frequency content. CarrierPress defaults to conservative narrow notches and does not claim to remove all noise or provide forensic-quality restoration.

## Multiband

The M5 multiband mode is an optional first compressor foundation. It splits mono or stereo audio into 2 to 4 bands, applies simple linked per-band compression, meters each band, and recombines before the final limiter. The structs are sized so 2 to 9 bands can be added later, but v0.1 accepts only 2, 3, or 4 active bands.

Enable it with `--multiband`. The `speech` preset uses slightly stronger conservative compression for spoken audio. The `music` preset uses gentler ratios and slower timing. These are practical starting points, not final AM or SSB broadcast presets.

```sh
./carrierpress --input input.wav --output output.wav --multiband --multiband-bands 2 --multiband-preset speech
./carrierpress --input input.wav --output output.wav --multiband --multiband-bands 3 --multiband-preset music
```

M5 uses cascaded 2nd-order low-pass sections with subtractive band creation for the crossover scaffold. It is bounded and deterministic, but it is not final broadcast processing.

## AM Mode

The M6 AM mode is audio-chain processing for legal AM transmitters and dummy-load testing. It is not an RF exciter, modulator, transmitter controller, or certified compliance tool. Users must obey their licence terms, transmitter limits, occupied bandwidth limits, and local radio regulations.

Enable AM mode with `--am`. The AM chain runs after AGC and the first multiband compressor, then before the final limiter. It provides high-pass filtering, low-pass audio bandwidth limiting, optional phase rotation, positive and negative peak control, and explicitly configured positive asymmetry.

```sh
./carrierpress --input input.wav --output output.wav --am --am-preset am-safe
./carrierpress --input input.wav --output output.wav --am --am-preset am-shortwave
./carrierpress --input input.wav --output output.wav --am --am-lowpass 4500 --am-highpass 80
```

AM presets:

| Preset         | Purpose                                      |
| -------------- | -------------------------------------------- |
| `am-safe`      | Conservative mono-safe starting point        |
| `am-shortwave` | 5-7 MHz shortwave-style starting bandwidth   |
| `am-wide`      | Wider lab and dummy-load testing preset      |
| `am-voice`     | Narrower voice-focused preset                |

Use `--am-asymmetry FLOAT` only when positive asymmetry is explicitly wanted. The module allows up to 200 percent positive capability for experiments, but that is not a default and is not a compliance claim. Negative peaks remain strictly limited by `--am-negative-peak`.

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

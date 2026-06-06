# Core Processing Quality

CarrierPress is not claiming broadcast-processor quality yet. This document
defines the current engineering baseline for making the Linux-host DSP chain
more consistent before adding larger features.

## Active Host Target

The active development host is Linux. The default host path is:

- offline WAV processing with libsndfile
- live audio with PortAudio
- WAV playout through PortAudio output
- optional ncurses TUI monitoring and preset controls

The optional sndio backend remains in the tree, but remaining sndio work is
deferred. STM32H753 and CMSIS-DSP work is also deferred.

## Current Chain

The current processing chain is:

```text
input
DC blocker
dehummer
restoration analysis tap
declipper
auto EQ analysis tap
natural dynamics
low-level boost
AGC
multiband compressor 1
bass EQ
multiband compressor 2
AM or SSB output shaping
limiter
meter
output
```

Most stages are disabled by default. The enabled default chain remains
conservative and bounded.

## Quality Gate

Before adding new DSP features, the existing chain should pass:

```sh
make clean
make
make test
make -j test
./carrierpress --self-test
make validate
make quality
make professional-check
make WITH_SNDFILE=1 WITH_PORTAUDIO=1 WITH_TUI=1
make WITH_SNDFILE=1 WITH_PORTAUDIO=1 test
```

These checks cover deterministic synthetic fixtures, finite output, limiter
bounds, silence stability, DC reduction, hum reduction, high-pass and low-pass
behavior, stereo stability, AGC limits, AM and SSB peak limits, restoration and
auto EQ analyzer metrics, declipper gating, and dynamics-stage activity.

## Acceptance Criteria

The current professional-standard target means:

- no non-finite output from any tested chain
- no limiter ceiling violations in tested profiles
- no gain runaway on silence or gated low-level material
- no AM or SSB peak-limit regressions in tested profiles
- stable mono and stereo behavior with linked gain where required
- deterministic results across normal and parallel test builds
- clear diagnostics for analysis, declipper bypass, meters, and TUI snapshots
- clear tonal-balance diagnostics without automatic EQ sample changes
- no heap allocation in DSP process functions or PortAudio callback processing

This is an engineering quality baseline. It is not a listening test,
transmitter test, spectrum mask test, restoration-quality claim, or regulatory
compliance claim.

## Known Limits

- The first and second multiband compressors support only 2 to 4 active bands.
- Bass EQ is static and preset-based. Automatic EQ processing is not
  implemented.
- Auto EQ is analysis-only and does not set or apply EQ gains.
- Natural dynamics and low-level boost are conservative wideband stages, not a
  final loudness processor.
- The declipper is a bounded research prototype for confident clipping cases.
- Delossifier, immersive bass, and true bass are not implemented yet.
- AM and SSB modes shape baseband audio only. CarrierPress is not an RF
  exciter, modulator, transmitter controller, or compliance tool.

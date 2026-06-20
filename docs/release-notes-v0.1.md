# CarrierPress v0.1.0 Release Notes

Suggested tag: `v0.1.0`

CarrierPress v0.1.0 is a clean-room C17 audio-processing foundation for AM and
SSB baseband audio chains. It is intended for legal transmitter audio paths,
recorded-audio processing, live host experiments, and dummy-load testing.

Users remain responsible for licence limits, occupied bandwidth,
station-control requirements, transmitter limits, and local radio rules.

## What v0.1.0 Is

v0.1.0 is a reproducible host validation point for the current CarrierPress
audio-chain foundation. It provides command-line, offline, live-host, TUI, GUI,
and read-only CAT monitor foundations that can be tested before later tuning
and embedded work.

## What v0.1.0 Is Not

v0.1.0 is not an RF generator, transmitter controller, regulatory-compliance
tool, or final broadcast processor.

## Core DSP Foundations

- Portable float32 block-DSP core with explicit processor state.
- DC blocker, metering, AGC, dehummer, conservative declipper research stage,
  natural dynamics, low-level boost, multiband foundations, static bass EQ,
  AM shaping, SSB shaping, and final limiter stages.
- Validation, quality, and professional-check fixtures for repeatable host
  testing.

## Optional Host Features

- Optional WAV processing with libsndfile.
- Optional PortAudio live audio and WAV playout.
- Optional ncurses TUI operator panel.
- Optional SDL3 GUI monitor with processed-output waveform.
- Optional FFTW GUI spectrum preview.
- Read-only CAT status through mock, flrig XML-RPC, and hamlib backends.

## What Is Not Included

- No RF generation.
- No transmitter-compliance or regulatory-approval claim.
- No PTT control or CAT write/control commands.
- No MP3 playout.
- No C-QUAM stereo.
- No final NRSC implementation.
- No STM32H753 or CMSIS-DSP port.
- No completed sndio hardware validation.

## Safety Notes

CarrierPress CAT support is read-only in v0.1.0. The flrig and hamlib backends
read frequency, mode, and PTT state for display. They do not key a transmitter,
change rig frequency, change mode, or send transmit commands.

PTT control is deferred. Any future PTT work must follow
`docs/cat-ptt-safety.md` and remain opt-in at compile time and runtime.

## Known Limitations

- The processing chain is a v0.1 foundation, not a finished broadcast
  processor.
- Presets are conservative starting points and require measurement on the
  user's actual audio path.
- The GUI spectrum is a monitor preview, not a calibrated spectrum analyser.
- Live audio depends on host PortAudio device behavior.
- WAV playout is PCM/WAV-focused; MP3 playout is deferred.
- sndio is present as an optional foundation, but broader OpenBSD hardware
  validation is deferred.
- STM32H753 and CMSIS-DSP support are planned after the host chain is proven.

## Validation Summary

Before tagging v0.1.0, run:

```sh
make release-check
```

Optional Linux-host checks can be run with:

```sh
CP_CHECK_SNDFILE=1 ./scripts/release-check.sh
CP_CHECK_PORTAUDIO=1 ./scripts/release-check.sh
CP_CHECK_TUI=1 ./scripts/release-check.sh
CP_CHECK_GUI=1 CP_CHECK_FFTW=1 ./scripts/release-check.sh
CP_CHECK_FLRIG=1 ./scripts/release-check.sh
CP_CHECK_HAMLIB=1 ./scripts/release-check.sh
```

For local Hamlib 4.7.1 validation, use the guidance in
`docs/release-checklist.md`. Do not hardcode local vendored paths into normal
builds or release scripts.

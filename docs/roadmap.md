# Roadmap

## M0 repo skeleton

Create the portable C17 DSP core, public headers, block API, CLI self-test, and unit tests.

## M1 offline WAV processing

Partially implemented. Optional libsndfile support can process mono and stereo WAV files in fixed-size blocks while preserving sample rate and channel count. The core still builds without libsndfile.

## M2 live PortAudio backend

Implemented as an optional PortAudio foundation. It supports device listing, callback-mode live processing, configurable devices, sample rate, channels, block size, and meter interval. Hardware behavior still needs broader USB sound-card testing.

## M3 AGC improvements

Improve detector behavior, gain law, channel linking, and transition handling while keeping explicit state structs.

## M4 dehummer

Research and add hum detection and reduction suitable for speech and program audio.

## M5 multiband compressor

Add crossover design, per-band dynamics, meters, and limiter interaction.

## M6 AM chain

Add AM-oriented shaping, bandwidth controls, and legal transmitter test-load workflows.

## M7 SSB chain

Add SSB-oriented filtering, asymmetry handling, and speech-focused processing.

## M8 auto EQ and bass modules

Research auto EQ, bass EQ, immersive bass, and true bass style functions without copying proprietary algorithms or presets.

## M9 declipper and delossifier research

Research restoration stages for clipped or lossy source material. Add only validated clean-room designs.

## M10 sndio backend

Add optional sndio live audio support for OpenBSD.

## M11 STM32H753/CMSIS-DSP port

Add fixed-size embedded build paths, Q31/Q15 planning, and CMSIS-DSP acceleration where it fits.

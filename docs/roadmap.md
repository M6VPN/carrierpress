# Roadmap

## M0 repo skeleton

Create the portable C17 DSP core, public headers, block API, CLI self-test, and unit tests.

## M1 offline WAV processing

Partially implemented. Optional libsndfile support can process mono and stereo WAV files in fixed-size blocks while preserving sample rate and channel count. The core still builds without libsndfile.

## M2 live PortAudio backend

Implemented as an optional PortAudio foundation. It supports device listing, callback-mode live processing, configurable devices, sample rate, channels, block size, and meter interval. Hardware behavior still needs broader USB sound-card testing.

## M3 AGC improvements

Implemented as a gated single-band input gain rider. It supports target RMS, min/max gain, attack/release timing, fast attack, hold, gate and silence thresholds, max gain step, sample-rate-aware timing, linked stereo gain, and AGC metering. It is not a complete broadcast processor.

## M4 dehummer

Partially implemented. CarrierPress now has an optional fixed-frequency dehummer using cascaded biquad notches for 50 Hz or 60 Hz mains hum and configured harmonics. It is conservative, disabled by default, and does not claim to remove all noise.

## M5 multiband compressor

Partially implemented. CarrierPress now has an optional first multiband compressor scaffold with 2 to 4 active bands, speech and music starting presets, per-band RMS/peak/gain-reduction meters, and recombination before the final limiter. The structs are sized for later 2 to 9 band support, but only 2 to 4 bands are accepted in M5.

## M6 AM chain

Partially implemented. CarrierPress now has an optional AM output-chain foundation with high-pass and low-pass filtering, optional phase rotation, separate positive and negative peak control, explicit asymmetry config, and conservative AM presets. It is audio-chain processing only, not an RF exciter or certified compliance implementation.

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

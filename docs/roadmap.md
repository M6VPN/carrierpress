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

## M6.5 live monitor foundation

Implemented as an optional ncurses TUI for PortAudio live mode. It shows input/output levels, AGC state, stream flags, multiband meters, and AM/SSB settings. The PortAudio callback still only updates atomic monitor values.

## M6.6 live AM control foundation

Implemented as a preset-only live control layer for the optional ncurses TUI. It can switch AM or SSB off and select validated presets while live audio is running. AM and SSB use explicit control banks so preset keys cannot change the wrong mode. Commands are handed from the foreground UI to the PortAudio callback and applied at audio block boundaries. Full parameter editing and web control are deferred.

## M6.7 laptop live audio selection

Implemented as a PortAudio device-selection improvement for laptop testing. Automatic live mode now prefers usable JACK devices, then PipeWire or Pulse-style full-duplex devices visible through PortAudio, then PortAudio defaults. Device listing shows host APIs, default markers, live candidates, and a recommended command.

## M6.8 WAV playout foundation

Partially implemented. CarrierPress can play WAV files or simple text playlists through the current DSP chain and an output-only PortAudio stream when built with both libsndfile and PortAudio. MP3 decode, web controls, and gapless playback are deferred.

## M6.9 play-mode live parity

Partially implemented. Play mode now reports live-style meters, honors `--meter-interval-ms`, uses the same DSP flags as live mode, and stops cleanly between blocks on `Ctrl-C`. It remains WAV-only and output-only.

## M6.10 playout TUI controls

Implemented as a first ncurses monitor for `--play` and `--playlist`. The TUI shows file and playlist position, live-style meters, AM/SSB state, and multiband state. It can stop with `q`, skip to the next playlist item with `n`, and apply the same mode-aware AM/SSB preset keys as live mode between processed blocks.

## M7 SSB chain

Partially implemented. CarrierPress now has an optional SSB output-chain foundation with speech-oriented high-pass and low-pass filtering, optional phase rotation, symmetric peak control, and conservative SSB presets. It is baseband audio-chain processing only, not an RF SSB modulator or compliance implementation.

## M7.1 TUI operator controls

Implemented as a preset-only operator-control pass for live and playout TUI modes. The TUI can toggle dehummer, cycle multiband through off, speech, and music, switch AM/SSB control banks, apply validated AM/SSB presets, stop processing, and skip playlist tracks. Arbitrary parameter editing, web control, and MP3 decode remain deferred.

## M7.2 playout format bridge

Partially implemented. CarrierPress stays WAV/PCM-native internally, documents external decode workflows for MP3/FLAC/OGG, reports unsupported playlist entries with line details, and adds deterministic validation fixtures for default, AM, and SSB chains. Native compressed-audio decoding remains deferred.

## M7.3 sample-rate handling

Partially implemented. WAV playout can use a dependency-free linear resampler when the requested or selected output rate differs from the file rate. The DSP chain runs at the output stream rate during resampled playout. High-quality resampling remains optional and deferred.

## M7.4 live/playout parity pass

Implemented as a host-path parity pass. Live mode and WAV playout now share host-to-DSP block config mapping, monitor snapshot extraction, AM/SSB mode-lock behavior, and config validation coverage. Hardware behavior still needs real device testing.

## M8 auto EQ and bass modules

Partially implemented. CarrierPress now has an optional static bass EQ foundation with conservative flat, speech, music, and warm presets. It uses bounded low-shelf and high-shelf filters after the first multiband compressor and before AM or SSB shaping. Automatic EQ, immersive bass, true bass, and adaptive source analysis remain deferred.

## M9 declipper and delossifier research

Research restoration stages for clipped or lossy source material. Add only validated clean-room designs.

## M10 sndio backend

Add optional sndio live audio support for OpenBSD.

## M11 STM32H753/CMSIS-DSP port

Add fixed-size embedded build paths, Q31/Q15 planning, and CMSIS-DSP acceleration where it fits.

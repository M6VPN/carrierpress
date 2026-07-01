# CarrierPress SSB Bulletin Workflow

CarrierPress is now oriented toward SSB voice bulletin and audio playout
workflows. It prepares audio that a receiver hears or records. It is not a
file-transfer modem and does not try to reconstruct files at the receiver.

The safe default is dry-run, preview, or audio-only output. No command keys a
transmitter by default. Future CAT or serial PTT must stay behind an explicit
`--arm-tx` gate and separate hardware backend work.

## Target Profile

The default bulletin profile is `hf-ssb-voice`:

- mono
- 48000 Hz output target
- SSB voice bandpass around 150 Hz to 2700 Hz
- speech-focused multiband compression
- limiter ceiling around -1 dBFS
- optional speech presence EQ
- radio speech processor assumed off first
- rig ALC target low to moderate

Use `hf-ssb-narrow` for weak or noisy paths. It uses a narrower 200 Hz to
2400 Hz voice band and denser speech processing.

Use `data-clean-pass-through` for JS8, Olivia, MT63, and other external
digital modem audio. Do not use the SSB voice profiles for modem/data tones.
The data-clean profile keeps speech EQ, speech compression, SSB shaping,
restoration, and clipping repair disabled.

## Built-In Profiles

CarrierPress accepts these built-in profile names in the bulletin workflow:

- `hf-ssb-voice`
- `hf-ssb-narrow`
- `vhf-fm-voice`
- `am-broadcast-style`
- `data-clean-pass-through`

Matching file profiles are also available under `profiles/`.

## Commands

Plan a prepared bulletin file without opening audio hardware:

```sh
./carrierpress ssb-play bulletin.wav \
  --profile hf-ssb-voice \
  --audio-device "USB Audio CODEC" \
  --id M6VPN \
  --repeat 3 \
  --dry-run
```

Generate a preview WAV when built with libsndfile:

```sh
make WITH_SNDFILE=1
./carrierpress ssb-play bulletin.wav \
  --profile hf-ssb-voice \
  --preview out.wav
```

Plan a spoken text bulletin. TTS is an adapter boundary in this slice, so
preview generation reports a clear unavailable message until a TTS adapter is
selected.

```sh
./carrierpress bulletin text-bulletin.txt \
  --tts \
  --profile hf-ssb-voice \
  --id M6VPN \
  --repeat 3 \
  --dry-run
```

Run live audio processing without PTT:

```sh
./carrierpress live \
  --input-device default \
  --output-device "USB Audio CODEC" \
  --profile hf-ssb-voice
```

Preview a carousel schedule without audio hardware:

```sh
./carrierpress carousel examples/ssb-carousel.toml \
  --profile hf-ssb-voice \
  --dry-run
```

## Carousel Schedule

Example schedule:

```toml
callsign = "M6VPN"
profile = "hf-ssb-voice"
repeat = 3
pre_roll_ms = 500
post_roll_ms = 800

[[items]]
type = "id"
text = "M6VPN test transmission"

[[items]]
type = "file"
path = "bulletin.wav"

[[items]]
type = "tts"
text = "End of bulletin. This is M6VPN."

[[items]]
type = "pause"
seconds = 10
```

The parser is intentionally small and explicit. It does not scan directories,
open files, decode media, or run TTS.

## PTT Safety

The current implementation does not include a hardware PTT backend. PTT
settings are parsed only as planning and validation metadata.

Rules:

- no PTT by default
- CAT or serial PTT requires `--arm-tx`
- armed CAT or serial PTT should include `--id`
- VOX mode is audio-only and does not call CAT or serial control
- no profile, config, report, batch, playlist, selector, or carousel item can
  secretly arm transmit
- no frequency or mode setting is part of this workflow

Future hardware PTT work remains under the T7 safety design.

## First Safe Test

1. Generate a preview WAV.
2. Listen in headphones.
3. Play into a dummy audio sink.
4. Test the radio into a dummy load.
5. Set rig power low.
6. Raise audio slowly.
7. Keep ALC low to moderate.
8. Use an antenna only when intentionally configured and legally permitted.

These steps are operating discipline, not RF, legal, regulatory, occupied
bandwidth, or transmitter compliance proof.

## Current Limits

This slice adds the bulletin plan/profile layer, built-in profiles, dry-run
subcommands, carousel parsing, and preview WAV routing through the existing
libsndfile processor.

Deferred:

- native MP3, Opus, FLAC, OGG, AAC, and M4A decoding
- native TTS generation
- stream/URL input adapters
- ALSA/PipeWire named-device selection beyond existing host audio backends
- hardware CAT/serial PTT
- pre-roll/post-roll execution around hardware PTT
- silence trimming
- station ID audio synthesis

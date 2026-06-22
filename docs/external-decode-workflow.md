# External Decode Workflow

CarrierPress playout is WAV/PCM-native in this milestone. MP3, FLAC, OGG,
Opus, and M4A are not native playout inputs. Convert compressed or container
formats to PCM WAV before using `--play` or `--playlist`.

CarrierPress does not call `ffmpeg` itself and does not add codec libraries for
this workflow. Conversion quality, codec licensing, and source-file handling
remain outside CarrierPress.

Possible future optional decoder approaches are documented in
[`optional-decoder-architecture.md`](optional-decoder-architecture.md). The
external conversion workflow remains the default.

## Target WAV Format

Use a simple PCM WAV target:

- 48 kHz sample rate.
- Mono or stereo.
- Sensible levels with no clipping.
- One WAV path per playlist line.

## Single File Conversion

These examples use `ffmpeg` as an external tool. Install and manage it outside
CarrierPress.

```sh
ffmpeg -i input.mp3 -ar 48000 -ac 2 output.wav
ffmpeg -i input.flac -ar 48000 -ac 2 output.wav
ffmpeg -i input.ogg -ar 48000 -ac 2 output.wav
ffmpeg -i input.opus -ar 48000 -ac 2 output.wav
ffmpeg -i input.m4a -ar 48000 -ac 2 output.wav
```

After conversion, check and play the WAV:

```sh
./carrierpress --play output.wav --meter-interval-ms 1000
```

## Batch Conversion

Convert into a separate directory so the original files stay untouched:

```sh
mkdir -p wav-out
for file in media/*.mp3; do
	name=${file##*/}
	ffmpeg -i "$file" -ar 48000 -ac 2 "wav-out/${name%.*}.wav"
done
```

Repeat the pattern for `.flac`, `.ogg`, `.opus`, or `.m4a` inputs as needed.

## Playlist Workflow

1. Convert source files to WAV.
2. Create a playlist containing WAV paths only.
3. Run the playlist checker:

```sh
./carrierpress --playlist-check playlist.txt
```

4. Build playout support:

```sh
make WITH_SNDFILE=1 WITH_PORTAUDIO=1
```

5. Run the playlist:

```sh
./carrierpress --playlist playlist.txt --meter-interval-ms 1000
```

CarrierPress prints cue/status lines when text playout starts, advances, stops,
or finishes. TUI and GUI modes suppress text cue lines so the monitor display
stays readable.

## Unsupported Inputs

If a playlist contains `.mp3`, `.flac`, `.ogg`, `.opus`, or `.m4a`, the checker
reports the line as unsupported and asks for conversion to WAV first. This is
intentional. Native compressed audio playout is deferred unless it is selected
as a later optional milestone.

## Selector Display

Interactive audio-file selectors use explicit candidate paths or recent cue
slots only. WAV candidates are selectable. Compressed candidates such as
`.mp3`, `.flac`, `.ogg`, `.opus`, `.m4a`, and `.aac` may be displayed as
disabled items with a convert-externally marker. The selector does not decode
compressed files, scan directories, open file dialogs, or process audio from UI
callbacks.

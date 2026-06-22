# Selector Workflow Foundation

CarrierPress selectors provide a dependency-light state model for future TUI
and GUI choice lists. The current foundation is data and formatting only. It
does not scan directories, open file dialogs, enumerate devices, open audio
devices, restart streams, process audio, decode compressed files, or add
transmit behavior.

## Selector Kinds

The shared selector model supports:

- `output_device`: output device choices collected by a host backend when
  enumeration is available.
- `audio_file`: candidate WAV file paths, recent paths, or explicit cue slots.
- `playlist`: candidate playlist paths, recent paths, or explicit cue slots.

Each item has a bounded label, bounded value, and enabled flag. Disabled items
can be displayed but cannot be selected.

## Output-Device Selector

P38B adds an output-device selector workflow foundation. Backend code that
already has output-device candidates can map them into `cp_selector` with
`cp_selector_load_output_devices()`. The helper does not enumerate devices,
open streams, close streams, or restart audio. It only turns collected
`cp_audio_device_candidate` entries into bounded selector labels and numeric
device-index values.

Output-capable devices are shown as enabled selector items. Input-only
candidates are ignored. Labels mark the current, requested, and default output
device where that state is known. Example lines:

```text
selector=output_device selected=2/3 label="USB Audio [current]" value="2"
> 2 USB Audio [current]
  4 Headphones [requested]
  1 Built-in Audio [default]
```

PortAudio live and playout paths can display the selector where enumeration is
already available. Device changes still use the existing deferred workflow
request and restart paths. sndio stays a named-device workflow and does not use
numeric output-device enumeration.

## Audio-File Selector

P38C adds an audio-file cue/load selector workflow foundation. Host code can
map explicit WAV candidates into `cp_selector` with
`cp_selector_load_audio_files()`. The helper does not scan directories, open
file dialogs, open files, call libsndfile, process audio, or decode compressed
formats. It only turns already-known paths, such as configured cue paths or
recent cue slots, into bounded selector labels and values.

WAV candidates with `.wav` or `.WAV` extensions are enabled. Compressed
formats such as `.mp3`, `.flac`, `.ogg`, `.opus`, `.m4a`, and `.aac` are shown
disabled with a convert-externally marker. Unknown extensions are shown
disabled as unsupported. Current and requested cue paths are marked when that
state is known. Example lines:

```text
selector=audio_file selected=2/3 label="news-bed.WAV [requested]" value="audio/news-bed.WAV"
> intro.wav [current]
  music.mp3 [convert externally] disabled
  news-bed.WAV [requested]
```

Selecting an enabled audio-file item can create the existing deferred
`load_wav` workflow request. Applying the request still happens in the host
workflow path, outside GUI callbacks, TUI draw paths, real-time audio
callbacks, and DSP block processing. Disabled compressed or unsupported items
cannot create a valid load request.

## Playlist Selector

P38D adds a playlist selector workflow foundation. Host code can map explicit
playlist candidates into `cp_selector` with `cp_selector_load_playlists()`.
The helper does not scan directories, open file dialogs, open playlist files,
call libsndfile, process audio, or decode compressed formats. It only turns
already-known paths, such as configured playlist cue paths or recent cue slots,
into bounded selector labels and values.

Playlist candidates with `.txt` or `.playlist` extensions are enabled. Audio
or media paths such as `.wav`, `.mp3`, `.flac`, `.ogg`, `.opus`, `.m4a`, and
`.aac` are shown disabled as not playlist files. Unknown extensions are shown
disabled as unsupported. Current and requested playlist paths are marked when
that state is known. Example lines:

```text
selector=playlist selected=2/3 label="morning.playlist [requested]" value="playlists/morning.playlist"
> show.txt [current]
  track.wav [not a playlist] disabled
  morning.playlist [requested]
```

Selecting an enabled playlist item can create the existing deferred
`load_playlist` workflow request. The existing playlist checker validates the
playlist before the request is accepted. Applying the request still happens in
the host workflow path, outside GUI callbacks, TUI draw paths, real-time audio
callbacks, and DSP block processing.

## Navigation Rules

Selectors are bounded and fixed-size:

- labels are limited to `CP_SELECTOR_LABEL_MAX`
- values are limited to `CP_SELECTOR_VALUE_MAX`
- item count is limited to `CP_SELECTOR_ITEMS_MAX`

`cp_selector_next()` and `cp_selector_prev()` wrap through enabled items.
`cp_selector_select()` rejects disabled and out-of-range items. Empty selectors
format as an explicit empty state.

Example formatted states:

```text
selector=output_device selected=1/3 label="USB Audio" value="2"
selector=audio_file selected=0/0 label=- value=-
selector=playlist selected=2/2 label="Show" value="playlists/show.txt"
```

## Future TUI and GUI Use

Future TUI and GUI selector work should reuse `cp_selector` for display and
navigation state:

- file and device changes must stay outside GUI callbacks, TUI draw paths,
  real-time audio callbacks, and DSP block processing
- paths, labels, and reasons must remain bounded before display
- selector display should live in the dashboard Selectors section, with related
  output-device state in the Device section

Native file dialogs, directory scanning, and compressed-audio decode support
remain future optional work. Compressed formats stay external-conversion
workflows by default.

## Safety Boundary

Selectors are ordinary UI state. They must not arm transmit control, key a
radio, send CAT write/control commands, call hamlib or flrig PTT functions, or
add active TX controls to ordinary builds.

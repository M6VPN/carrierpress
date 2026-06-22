# GUI Workflow Requests

This document defines the safe boundary for GUI file cueing and output device
selection. CarrierPress supports preconfigured GUI cue requests for WAV and
playlist paths, plus deferred output-device selection requests. It does not
implement GUI file dialogs. Live PortAudio and WAV playout can consume
deferred GUI output-device requests outside callbacks.

CarrierPress is baseband audio processing software. The GUI workflow path must
not add RF generation, transmitter compliance claims, licence-compliance
claims, regulatory approval claims, legal-bandwidth claims, CAT write/control
commands, or PTT control.

## Request Model

GUI-originated workflow changes are represented as bounded requests:

- `none`
- `load_wav`
- `load_playlist`
- `cue_playlist_item`
- `select_output_device`

The request type is defined by `cp_gui_workflow_request`. The model stores a
bounded path, a device index, and a playlist index. It does not perform file
I/O, validate audio contents, open devices, close devices, restart streams,
render SDL UI, or process audio.

Example formatted states:

```text
workflow=none
workflow=load_wav status=pending path=audio/program.wav
workflow=load_playlist status=pending path=playlists/show.txt
workflow=cue_playlist_item status=pending index=2 path=audio/program.wav
workflow=select_output_device status=pending device=3
Cue: WAV audio/program.wav | Playlist playlists/show.txt
output_device=current:3 requested:4 backend=auto device=default
```

When a request is consumed and validated by the host loop, the status is shown
in the same bounded format:

```text
workflow=load_wav status=valid reason=ok path=audio/program.wav
workflow=load_wav status=error reason=unsupported format: convert to WAV first path=audio/music.mp3
workflow=load_playlist status=valid reason=ok path=playlists/show.txt
workflow=select_output_device status=valid reason=deferred output device request device=4
```

## GUI Request Status Display

The GUI status panel shows the configured WAV cue slot, configured playlist
cue slot, last workflow request, request status, target path or device, and
validation reason when one exists.

Request status values are:

- `pending`: the GUI recorded the request, but the host loop has not validated
  or applied it yet.
- `valid`: the host loop accepted the request for deferred application.
- `error`: the host loop rejected the request.

Rejected cue and device requests remain visible as the last workflow request
with their reason and target path or device. CarrierPress does not retry
rejected GUI requests automatically. Paths and reasons are truncated before
drawing, and the SDL panel draw path clips text to the containing panel.

Where PortAudio enumeration is available, the GUI also shows a compact output
choice line with the current device, requested device, default output device,
and the first output-capable choices. The host loop collects this list outside
GUI callbacks and passes preformatted text to the GUI. sndio remains a
named-device workflow and is documented separately.

## Deferred Application

GUI event and render callbacks must not apply workflow changes directly.
Instead:

1. GUI code records a workflow request.
2. The outer host application loop consumes the request.
3. The host loop validates the request with existing parser and path rules.
4. The host loop applies the change outside real-time audio callbacks.

The following operations must happen outside real-time callbacks, DSP block
processing, and SDL render/draw routines:

- WAV path validation.
- Playlist path validation.
- Playlist item cueing.
- Sound output device selection.
- Audio stream stop, reopen, or restart.
- Any future file loading or queue update.

Output-device switching must stop and reopen streams safely from the host loop.
It must not be handled from an SDL render, SDL key event callback, or real-time
audio callback.

## File and Playlist Rules

The first GUI cueing implementation uses explicit cue slots:

```sh
./carrierpress --gui-demo --gui-cue-wav audio/program.wav --gui-cue-playlist playlist.txt
./carrierpress --play input.wav --gui --gui-cue-wav audio/program.wav
./carrierpress --playlist playlist.txt --gui --gui-cue-playlist playlist.txt
```

The GUI keys are:

- `l` requests the configured WAV cue slot.
- `p` requests the configured playlist cue slot.
- `c` requests the current playlist item as a cue request when playlist context
  exists.

These keys record and display deferred workflow requests. They do not open
audio devices, process audio, or start a replacement stream from the SDL event
path.

GUI file and playlist controls reuse the existing strict workflow rules where
practical:

- WAV files are the directly supported file input path.
- Playlist syntax should use existing playlist-check behavior.
- Batch and report paths should use existing batch/report validation where
  relevant.
- MP3, FLAC, OGG, Opus, and M4A remain external conversion workflows unless a
  later optional decoder milestone is selected.
- GUI path requests should be bounded and rejected if empty or too long.
- Path and playlist validation happens after the host loop consumes the GUI
  request, outside SDL render and event handling.

## Output Device Rules

M23C adds GUI output-device selection state as a deferred request. The GUI can
display the current configured output device, requested output device, selected
backend string, configured device name, and a compact PortAudio output-choice
list where enumeration is available. It does not enumerate hardware devices in
the base build.

The GUI keys are:

- `o` requests the next output device index.
- `O` requests the previous output device index, down to the default-device
  sentinel.

Output-device request rules:

- The GUI records the request only.
- The live PortAudio host loop validates the request after it is consumed.
- The default output-device sentinel is accepted where the CLI already uses it.
- Non-negative output device indices are accepted for deferred selection.
- Invalid negative device indices are rejected.
- Hardware existence is not checked in the base build.
- PortAudio output-device choices are collected by the host loop when
  PortAudio enumeration is available. GUI rendering only displays the bounded
  choice string.
- CLI-only output-device selection is preserved.
- In the live PortAudio GUI path, a changed output-device request makes the
  current stream loop exit cleanly. The outer host loop then starts a fresh
  stream with the requested output device.
- Device open, close, stream stop, and stream restart work is not performed
  from SDL event/render callbacks or real-time audio callbacks.
- If the requested device cannot be opened or started, the live PortAudio path
  reports the requested-device failure and tries once to fall back to the
  previous output device that was already running. If that fallback also fails,
  CarrierPress exits with the normal PortAudio error.
- In GUI WAV playout, a changed output-device request is consumed by the
  playout host loop after `cp_gui_update()` returns. CarrierPress stops and
  reopens the blocking PortAudio output stream between processed blocks. File
  position, DSP processor state, playlist position, and safe TUI/GUI controls
  are preserved. If the requested device cannot be opened or started,
  CarrierPress tries once to fall back to the previous output device that was
  already running.
- sndio output-device switching remains deferred. sndio uses named devices
  through `--device NAME`, while the current GUI output-device request is a
  PortAudio-oriented numeric-device request. The sndio evaluation is documented
  in [`sndio-gui-device-workflow.md`](sndio-gui-device-workflow.md).

## Current GUI Controls

The current GUI keyboard controls mirror safe TUI-style controls for monitor
and audio-chain operation:

- quit or stop
- playout next
- dehummer toggle
- multiband cycle
- second multiband cycle
- AM and SSB control-bank selection
- AM and SSB preset/off commands

These commands are not transmit controls. No GUI key maps to PTT, CAT
write/control, rig frequency, rig mode, or transmit state.

## Future Work Slices

M23B adds GUI-safe WAV and playlist cue requests with path validation in the
outer host loop. It does not process audio inside GUI callbacks, and it does
not add native file dialogs.

M23C1 adds output-device selection display and a deferred output-device
selection request. M23C2 applies that deferred request in the live PortAudio
GUI path by stopping and reopening audio streams outside callbacks. M23C2B adds
a single fallback attempt to the previous live PortAudio output device when a
requested device cannot be opened or started. M24A extends the same deferred
restart and fallback pattern to GUI WAV playout between processed blocks.
M25A evaluates sndio and keeps sndio GUI output-device restart deferred until a
named-device request model, restart-result boundary, and OpenBSD manual
validation evidence are available.

Any GUI TRANSMIT or CAT control toggle is T5-only work. It must require the T5
safety gates: compile-time opt-in, runtime arming, mock-only tests first,
emergency RX/drop handling, no real-time callback CAT control, and manual
dummy-load or receive-only validation. It must not be added by ordinary GUI
workflow or polish milestones. The future-only boundary is documented in
[`cat-ptt-safety.md`](cat-ptt-safety.md),
[`transmit-control-architecture.md`](transmit-control-architecture.md), and
[`transmit-control-checklist.md`](transmit-control-checklist.md).

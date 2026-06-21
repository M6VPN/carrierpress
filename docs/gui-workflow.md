# GUI Workflow Requests

This document defines the safe boundary for GUI file cueing and output device
selection. CarrierPress supports preconfigured GUI cue requests for WAV and
playlist paths. It does not implement GUI file dialogs or GUI output-device
switching yet.

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
workflow=load_wav path=audio/program.wav
workflow=load_playlist path=playlists/show.txt
workflow=cue_playlist_item index=2 path=audio/program.wav
workflow=select_output_device device=3
```

When a request is consumed and validated by the host loop, the status is shown
in the same bounded format:

```text
workflow=load_wav status=ok reason=ok path=audio/program.wav
workflow=load_wav status=error reason=unsupported format: convert to WAV first path=audio/music.mp3
workflow=load_playlist status=ok reason=ok path=playlists/show.txt
```

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

Output-device switching must stop and reopen streams safely in a later
implementation. It must not be handled from an SDL render or key event callback.

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

Future GUI output-device selection should:

- Display available devices only when the selected host backend can enumerate
  them safely.
- Represent the user selection as a deferred request.
- Apply the selection in the outer host loop.
- Preserve CLI-only output-device selection.
- Keep device open, close, and restart work outside real-time audio callbacks.

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

M23C should add output-device selection display and a deferred output-device
selection request. Stream restart must happen outside callbacks.

Any GUI TRANSMIT or CAT control toggle is T5-only work. It must require the T5
safety gates: compile-time opt-in, runtime arming, mock-only tests first,
emergency RX/drop handling, no real-time callback CAT control, and manual
dummy-load or receive-only validation. It must not be added by ordinary GUI
workflow or polish milestones.

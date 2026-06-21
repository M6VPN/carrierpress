# GUI Workflow Requests

This document defines the safe boundary for GUI file cueing and output device
selection. CarrierPress supports preconfigured GUI cue requests for WAV and
playlist paths, plus deferred output-device selection requests. It does not
implement GUI file dialogs or GUI audio stream switching yet.

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
output_device=current:3 requested:4 backend=auto device=default
```

When a request is consumed and validated by the host loop, the status is shown
in the same bounded format:

```text
workflow=load_wav status=ok reason=ok path=audio/program.wav
workflow=load_wav status=error reason=unsupported format: convert to WAV first path=audio/music.mp3
workflow=load_playlist status=ok reason=ok path=playlists/show.txt
workflow=select_output_device status=ok reason=deferred output device request device=4
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

M23C adds GUI output-device selection state as a deferred request. The GUI can
display the current configured output device, requested output device, selected
backend string, and configured device name. It does not enumerate hardware
devices in the base build.

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
- Playout and sndio output-device switching remain future work unless selected
  in a later slice.

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
requested device cannot be opened or started. Playout switching and sndio
switching remain future work.

Any GUI TRANSMIT or CAT control toggle is T5-only work. It must require the T5
safety gates: compile-time opt-in, runtime arming, mock-only tests first,
emergency RX/drop handling, no real-time callback CAT control, and manual
dummy-load or receive-only validation. It must not be added by ordinary GUI
workflow or polish milestones.

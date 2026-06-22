# sndio GUI Device Workflow Evaluation

This document evaluates whether CarrierPress should add GUI output-device
restart for the optional sndio backend.

CarrierPress remains baseband audio processing software. This workflow does not
add RF generation, transmitter compliance claims, licence-compliance claims,
regulatory approval claims, legal-bandwidth claims, CAT write/control commands,
PTT control, or active TRANSMIT controls.

## Current sndio State

sndio support is optional and selected explicitly with `WITH_SNDIO=1`.
OpenBSD and sndio validation remain manual unless a stable runner path is added
later.

The current public sndio API is:

```c
int cp_sndio_run(const struct cp_audio_config *,
	volatile sig_atomic_t *);
```

The sndio backend uses named devices through `--device NAME`:

```sh
./carrierpress --live --audio-backend sndio --device default
./carrierpress --live --audio-backend sndio --device snd/0
```

Numeric `--input-device N` and `--output-device N` are PortAudio-oriented
options. The sndio path rejects numeric input and output device indices.

## Difference from PortAudio

PortAudio uses enumerated numeric input and output device indices. CarrierPress
already has a PortAudio restart-result path for GUI live mode and GUI WAV
playout:

- GUI callbacks record a bounded workflow request.
- The host loop consumes and validates the request.
- Stream stop, close, reopen, and restart happen outside SDL callbacks.
- The real-time audio callback does not open, close, or restart streams.
- Failed requested output devices fall back once to the previous output device.

The sndio backend does not currently have an equivalent restart-result API. It
has a single blocking `cp_sndio_run()` call that owns open, parameter setup,
start, read, processing, write, and close for one run. It also uses named
devices rather than PortAudio-style numeric indices.

## Safety Boundary

Any future sndio GUI device restart must keep the same safety boundary as the
PortAudio path:

- GUI callbacks must only record requests.
- sndio open, close, start, stop, and parameter setup must happen in an outer
  host loop.
- Audio processing loops must not perform GUI event work from a real-time
  callback.
- Device names must be validated before any reopen attempt.
- Failure handling must be explicit and bounded.
- No CAT, PTT, transmit, rig frequency, rig mode, or station-control behavior is
  part of the device workflow.

## Evaluation

sndio GUI output-device restart is not technically ready in the current code.
The missing pieces are:

- a sndio run-result structure equivalent to `cp_portaudio_run_result`
- a host-loop restart wrapper for sndio named devices
- a GUI request shape for named sndio devices, not numeric PortAudio indices
- manual OpenBSD validation evidence for close/reopen behavior
- documented fallback behavior for failed named-device reopen attempts

The current GUI output-device request model is numeric because it mirrors
PortAudio enumeration. That model should not be reused directly for sndio
without a named-device selection design.

## Recommendation

Do not implement sndio GUI output-device restart in the current v0.4 slice.

Keep these boundaries:

- Linux PortAudio remains the active GUI output-device restart path for live
  mode and WAV playout.
- sndio remains optional and manually validated.
- sndio device selection remains CLI/config based with `--audio-backend sndio
  --device NAME`.
- OpenBSD/sndio validation remains manual.
- A future sndio restart implementation should first add a safe restart-result
  boundary and named-device request model, then gather OpenBSD manual validation
  evidence.

## Future Implementation Requirements

If sndio GUI output-device restart is selected later, the minimum safe shape is:

- Add a sndio run-result API that can report a deferred restart request.
- Add a named-device GUI workflow request separate from PortAudio numeric
  output-device requests.
- Consume requests in an outer host loop, not in GUI callbacks.
- Stop and reopen sndio only outside audio processing callbacks and GUI
  callbacks.
- Preserve Linux PortAudio behavior unchanged.
- Keep sndio optional in builds, tests, and pkg-config metadata.
- Add manual OpenBSD validation notes before treating it as release-ready.

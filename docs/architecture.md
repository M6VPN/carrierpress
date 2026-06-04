# CarrierPress Architecture

CarrierPress uses a block DSP model. Callers own input, output, and scratch buffers, then pass interleaved mono or stereo frames into processor functions. Each processor keeps explicit state in a struct with `init`, `reset`, and `process` functions.

The v0.1 core is written in C17 because it is portable across Linux, FreeBSD, OpenBSD, Raspberry Pi, and embedded targets. C also keeps ABI boundaries simple for later host backends, test tools, and MCU ports.

The audio path is designed to avoid heap allocation. Process functions validate their arguments, use caller-provided buffers, and update only their own state. This keeps future real-time callbacks deterministic and easier to audit.

## Backend Separation

The DSP core does not include PortAudio, libsndfile, sndio, or embedded platform headers. Optional host code sits behind small boundary modules:

- `cp_audio` owns live-audio defaults and config validation.
- `cp_wav` owns optional offline WAV processing with libsndfile.
- `cp_portaudio` owns optional live sound-card processing with PortAudio.
- `cp_playout` owns optional WAV file playout through PortAudio output.
- `cp_monitor` owns dependency-free monitor snapshots and scaling.
- `cp_control` owns validated live control commands.
- `cp_tui` owns optional ncurses live monitoring.

This split keeps the core library usable for offline tools, live hosts, and embedded ports without forcing every dependency into every build.

## Planned Host Backends

- PortAudio for Linux and FreeBSD live USB sound-card processing. It is optional because many target builds, including embedded and test builds, do not need a host audio API.
- sndio for OpenBSD live audio after the portable core is stable.
- CMSIS-DSP for STM32H753 after the float32 host chain is proven.

The PortAudio backend uses callback mode. Buffers and DSP state are allocated before stream start, the callback does not print, and meter/status values are handed to the foreground loop for reporting. Text meters and the optional ncurses TUI both read monitor snapshots outside the callback.

Live device selection stays at the PortAudio boundary. Automatic mode prefers usable JACK devices, then PipeWire or Pulse-style full-duplex devices visible through PortAudio, then PortAudio defaults. CarrierPress does not include direct ALSA, JACK, or PipeWire code in the DSP core.

WAV playout is a host feature, not a DSP core feature. It requires both
libsndfile and PortAudio, reads WAV data in fixed-size blocks, processes those
blocks through the normal CarrierPress chain, and writes them to an output-only
PortAudio stream. Play mode reports live-style meters from the same processor
state used by the live monitor and stops cleanly between blocks when interrupted.
M6.10 still uses blocking PortAudio output for file playout. The TUI can monitor
playout, switch validated operator presets, and skip to the next playlist item.
Callback playout and web control remain deferred.

Live TUI controls use a small command handoff. The foreground TUI validates key input into a preset command, stores one pending command atomically, and the callback applies it at the next block boundary. Playout TUI controls are applied between blocking file-output blocks. M7.4 keeps live and playout aligned by sharing host-to-DSP block config setup and processor snapshot extraction before live mode hands values to atomics. Future sndio/OpenBSD work should keep the same boundary. The STM32H753 path should call the same block DSP model directly or through a CMSIS-DSP adapter.

## State Ownership

There is no global mutable DSP state. A host program creates processor structs, initializes them, and controls buffer lifetime. This keeps offline tools, live audio backends, and embedded ports using the same core model.

# CarrierPress Operator Workflow

This guide describes ordinary operator-facing CarrierPress workflows. It covers
CLI, TUI, GUI, CAT status, examples, and demo scripts. Ordinary operator
workflow does not include active transmit control.

CarrierPress remains a baseband audio processor. It does not provide hardware
PTT, CAT write/control, rig frequency setting, rig mode setting, or an active
TRANSMIT UI in ordinary builds.

## CLI Workflow

The CLI is the base interface and remains usable without GUI or TUI support.
Common safe workflows are:

- Validate profile files with `--validate-profile PATH`.
- Validate config files with `--validate-config PATH`.
- Inspect resolved settings with `--print-effective-config`.
- Process built-in self-test tones.
- Process offline WAV files when built with `WITH_SNDFILE=1`.
- Check playlists and batch plans without opening audio devices.
- Run batch processing and report tools.
- Run local validation targets such as `make validation-help`,
  `make test-matrix-help`, and `make transmit-control-safety-audit`.

CLI profile, config, report, batch, playlist, and validation paths must not arm
or request transmit control. The guarded transmit-control namespace is
mock-only and is not part of ordinary CLI workflow.

## TUI Workflow

The ncurses TUI is optional. It is a local monitor and safe-control surface for
operator state and audio-chain controls.

Safe TUI controls include:

- stop or quit
- playout next, where playout supports it
- dehummer toggle
- multiband cycle
- second multiband cycle
- AM and SSB control-bank selection
- AM and SSB preset/off commands

The TUI has no TRANSMIT key, no PTT key, no CAT write/control key, and no
transmit arming path.

## GUI Workflow

The SDL3 GUI is optional. It is a monitor and safe-control surface. It also
shows deferred workflow state for queued WAV and playlist paths, request
status, rejected request reasons, and output-device choices where PortAudio
enumeration is available.

GUI workflow changes are represented as deferred requests. The GUI records a
bounded request, and the host loop validates and consumes it outside SDL event
callbacks, SDL render callbacks, DSP block processing, and audio callbacks.

Safe GUI controls include:

- stop or quit
- playout next
- dehummer toggle
- multiband cycle
- second multiband cycle
- AM and SSB control-bank selection
- AM and SSB preset/off commands
- queued WAV and playlist request keys
- deferred output-device request keys

Future interactive selectors are documented in
[`selector-workflow.md`](selector-workflow.md). The current selector foundation
is a shared bounded state model for later TUI and GUI output-device, audio-file,
and playlist selection. It does not scan directories, open file dialogs,
enumerate devices, open audio devices, or change audio behavior.

GUI text must remain bounded and clipped inside panels. Long paths, request
reasons, output-device names, and help strings are truncated before drawing.

The GUI has no active TRANSMIT button, no PTT toggle, no CAT write/control
control, and no transmit arming path.

## CAT Status Boundary

Ordinary CAT paths are read-only status paths. The mock CAT backend, optional
flrig backend, and optional hamlib backend must not send write/control
commands.

CAT status may show mock or read-only radio state, but ordinary workflows do
not set rig frequency, set rig mode, key PTT, request transmit, or arm
transmit control.

Transmit-control scaffolding is T5 guarded and mock-only. It is covered by the
safety docs, checklist, and static audit:

- [`cat-ptt-safety.md`](cat-ptt-safety.md)
- [`transmit-control-architecture.md`](transmit-control-architecture.md)
- [`transmit-control-checklist.md`](transmit-control-checklist.md)

## Examples and Demos

Examples are local wrappers for safe inspection and validation. They must not
install packages, use `sudo`, create tags, push tags, publish releases, upload
artifacts, key a radio, or send CAT write/control commands.

Some examples write files under `build/`, such as screenshots, batch output,
or evidence reports. Some examples require optional dependencies, such as
libsndfile, PortAudio, SDL3, FFTW, ncurses, sndio, flrig, or hamlib. Those
requirements are documented in [`examples/README.md`](../examples/README.md).

Use the safe local operator demo after building:

```sh
./examples/operator-safe-demo.sh
```

The demo runs local inspection commands only. It does not open audio devices,
require optional libraries, access hardware, or run clean-mutating targets.

## Safety Audits

Run the transmit-control isolation audit:

```sh
make transmit-control-safety-audit
```

Run the operator workflow audit:

```sh
make operator-workflow-safety-audit
```

Both targets are non-mutating. They do not build, clean, publish, install
packages, or access hardware.

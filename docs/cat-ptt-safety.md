# CAT PTT Safety Design

CarrierPress CAT support is read-only. The mock, flrig, and hamlib paths report
status for operator display and tests. They do not control a transmitter.

## Current Status

- CarrierPress reads CAT status outside the DSP core.
- The flrig backend reads frequency, mode, and PTT state only.
- The hamlib backend reads frequency, mode, and PTT state only.
- No PTT control is implemented.
- No CLI, TUI, or GUI command can key a transmitter.
- No CAT backend sends transmit commands, set-frequency commands, set-mode
  commands, or set-PTT commands.

## Threat Model

Any future PTT control must assume these failures are possible:

- Accidental transmit from a bad command, bad default, UI mistake, or stale
  state.
- Transmit on the wrong band, frequency, or mode.
- Transmit into the wrong antenna, disconnected antenna, tuner, amplifier, or
  load.
- Transmit while unlicensed or outside licence conditions.
- Transmit unattended or without a local operator in control.
- Stuck PTT after a crash, blocked process, CAT timeout, GUI hang, TUI hang, or
  backend failure.
- Displayed state not matching actual rig state.
- Audio-processing state racing with station-control state.
- Network-exposed flrig or rigctld endpoints receiving unintended commands.
- Wrong rig model, serial path, network host, or network port.
- User confusion between CarrierPress audio processing and transmitter control.

## Requirements Before PTT Control

PTT control must not be added until all of these gates are implemented and
tested:

- Compile-time opt-in such as `WITH_CAT_PTT=1`.
- Runtime opt-in with an explicit dangerous option.
- Runtime arming before any transmit-control action is accepted.
- PTT control disabled by default.
- No PTT auto-enable from `make autodetect`.
- PTT control disabled in GUI and TUI unless explicitly enabled.
- Mock-only transmit-control state-machine tests before any hardware backend.
- Read-only CAT status must work before any PTT command is accepted.
- Frequency, mode, and PTT readback must be displayed before any PTT action.
- A clear user-responsibility warning must be shown before arming transmit
  control.
- A maximum transmit timer or watchdog must force a bounded transmit window.
- An emergency drop-to-RX command must be available.
- Startup must force or verify RX, not TX.
- Shutdown and error paths must attempt RX only if PTT control was explicitly
  enabled.
- PTT must never be keyed from an audio callback.
- CAT control must never block an audio callback.
- Audio presence must never automatically key transmit.
- Profiles, config files, reports, batch files, GUI workflow requests, and
  playout playlists must not arm transmit or request transmit.
- Scheduled or remote unattended transmit must not be implemented.
- CAT control over untrusted network endpoints must require an explicit warning.
- Automated tests must use mock backends only unless a manual hardware test is
  explicitly selected.
- Manual hardware testing must start with a dummy load or receive-only mock path.

## Forbidden in Ordinary Builds

Ordinary CarrierPress builds must not expose active station-control features.
These items stay outside normal profiles, configs, reports, batch files, GUI
workflow requests, TUI controls, and CLI audio workflow commands:

- PTT control.
- TRANSMIT controls.
- CAT write or control commands.
- Rig frequency setting.
- Rig mode setting.
- Automatic transmit.
- Audio-triggered transmit.
- Hidden transmit control in config or profile files.
- Transmit actions from live audio, playout, batch processing, report tools, or
  real-time callbacks.

Read-only CAT status remains allowed. Displaying read-only PTT state is not the
same as controlling PTT.

## T5 Implementation Phases

T5 work must stay phased. A later phase must not start until earlier gate
evidence exists.

- T5A: safety-gate design and test plan.
- T5B: compile-time guard and disabled-by-default API stubs.
- T5C: mock-only runtime-arming transmit-control state-machine tests.
- T5D: mock-only emergency RX/drop semantics.
- T5E: callback isolation audit and manual validation checklist closeout.
- T5F: optional hardware backend after all previous gates pass.

T5A is documentation and safety planning only. T5B adds a compile-time guard
and disabled API stubs only. T5C adds a mock-only runtime-arming state machine
only. T5D adds mock-only emergency RX/drop only. T5F is not part of T5A, T5B,
T5C, T5D, or T5E.

## Current T5E Boundary

The `cp_transmit_control` namespace is a mock-only boundary for future work. In
an ordinary build, `cp_tx_control_available()` returns unavailable and requests
return disabled. With `WITH_TRANSMIT_CONTROL=1`, the guarded mock state machine
starts disarmed and rejects transmit requests until the control object is armed
in memory.

T5D adds `cp_tx_control_emergency_rx()` as an immediate mock-only safety
override. It clears mock runtime arming and returns to `disarmed` without
calling CAT, PTT, hamlib, flrig, serial, GPIO, VOX, or other hardware-control
paths.

T5E adds static callback and path isolation audit coverage. The audit checks
that transmit-control calls remain out of real-time callback paths, GUI/TUI
paths, profile/config parsing, report generation, batch processing, playout,
and main CLI workflow paths.

T5E does not add a hardware backend, GUI controls, TUI controls, CLI options,
profile keys, config keys, report fields, or batch fields. The guarded mock
`tx_requested` and `tx_active` states are local test state only and cannot key a
radio.

No CAT write/control backend exists. Read-only CAT status remains separate from
the guarded mock transmit-control namespace.

## Current T6A Boundary

T6A adds a guarded mock-only status display for operator surfaces. In a
`WITH_TRANSMIT_CONTROL=1` build, the GUI and TUI may show the mock state,
whether the mock object is armed, and whether the mock state is active.

T6A is display-only. It does not add arm/disarm controls, transmit-request
controls, emergency controls, hardware backends, CAT write/control, hamlib or
flrig PTT calls, profile/config arming, or report/batch/playlist transmit
fields. Ordinary builds remain unavailable and free of active TX controls.

## Current T6B Boundary

T6B adds guarded mock-only runtime arm/disarm controls. In a
`WITH_TRANSMIT_CONTROL=1` build, operator surfaces may use `r` to arm the mock
state machine and `u` to disarm it. These keys affect only the in-memory mock
state and do not call CAT, hamlib, flrig, serial, GPIO, VOX, or hardware
backends.

T6B does not add transmit-request controls, emergency RX/drop UI controls,
profile/config arming, report/batch/playlist transmit fields, or
frequency/mode control. Ordinary builds do not expose the guarded mock
arm/disarm keys.

## Future Architecture

Any future implementation should use a separate PTT command boundary, for
example `cp_cat_ptt_*`, instead of extending read-only snapshots into command
state.

- Keep PTT commands host-side and outside DSP modules.
- Keep command state separate from monitor snapshots.
- Show armed or disarmed state separately from read-only PTT readback.
- Treat any GUI TRANSMIT toggle as T5-only work. It must not be added by
  ordinary GUI polish, and it must stay disabled unless PTT support is
  compiled, explicitly armed, and covered by mock-only tests first.
- GUI file cueing, playlist cueing, and output-device selection milestones must
  not introduce a TRANSMIT toggle or any CAT write/control command.
- Make commands edge-triggered and bounded, not continuous hidden state.
- Keep GUI and TUI controls locked unless PTT support is compiled and armed.
- Prefer a single PTT state machine shared by mock, flrig, and hamlib command
  backends.

The future PTT state machine should have these explicit states:

- `disabled`
- `disarmed`
- `armed_rx`
- `tx_requested`
- `tx_active`
- `rx_requested`
- `fault`

The fault state should prefer RX or drop PTT where possible, but it must not
loop indefinitely or hide repeated CAT failures.

## User Responsibility

CarrierPress is an audio processor and monitor. It is not an RF exciter,
transmitter controller, certified station controller, or regulatory-compliance
tool. Users remain responsible for licence limits, transmitter limits, occupied
bandwidth, station control, antenna and dummy-load safety, and local radio
regulations.

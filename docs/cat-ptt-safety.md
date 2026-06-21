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
- PTT control disabled by default.
- No PTT auto-enable from `make autodetect`.
- PTT control disabled in GUI and TUI unless explicitly enabled.
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
- Scheduled or remote unattended transmit must not be implemented.
- CAT control over untrusted network endpoints must require an explicit warning.
- Automated tests must use mock backends only unless a manual hardware test is
  explicitly selected.
- Manual hardware testing must start with a dummy load or receive-only mock path.

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

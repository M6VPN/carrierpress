# Transmit Control Checklist

CarrierPress does not implement PTT or TRANSMIT control. This checklist is for
future T5 safety-gated work only. It is not permission to test on air.

## Pre-build Checklist

- Confirm the selected work is part of T5, not ordinary GUI, TUI, profile,
  config, report, batch, or release-polish work.
- Confirm transmit-control code is behind an explicit compile-time opt-in.
- Confirm ordinary builds keep CAT read-only.
- Confirm no profile, config file, report, batch list, playlist, or GUI
  workflow request can arm transmit.
- Confirm no transmit-control call can run from a DSP block, live audio
  callback, playout callback, GUI callback, or report path.

## T5B Compile-Time Guard Checklist

- Confirm ordinary builds report transmit-control unavailable.
- Confirm `WITH_TRANSMIT_CONTROL=1` builds only guarded scaffolding.
- Confirm guarded scaffolding has no hardware backend.
- Confirm guarded scaffolding has no hamlib or flrig PTT calls.
- Confirm request placeholders cannot enter TX.
- Confirm no GUI or TUI key maps to active transmit control.
- Confirm no CLI option, profile key, config key, report field, batch field, or
  playlist directive can arm transmit.

## Mock-test Checklist

- Start with a mock backend only.
- Prove the initial state is RX/off.
- Prove disarmed requests cannot enter TX.
- Prove runtime arming is required before any transmit request is accepted.
- Prove emergency RX/drop overrides all other requested states.
- Prove timeout or watchdog logic returns to RX/off.
- Prove backend failure enters a fault state and does not retry forever.
- Prove no GUI/TUI key maps to active transmit in ordinary builds.

## T5C Mock State-Machine Checklist

- Confirm ordinary builds report transmit-control unavailable.
- Confirm ordinary state-changing calls return disabled.
- Confirm `WITH_TRANSMIT_CONTROL=1` starts the mock object disarmed.
- Confirm disarmed transmit requests are rejected.
- Confirm runtime arming moves only to `armed_rx`.
- Confirm mock transmit requests move only through mock state-machine states.
- Confirm mock RX requests return to `armed_rx`.
- Confirm no hardware backend exists.
- Confirm no GUI, TUI, CLI, profile, config, batch, report, or playlist path
  can arm transmit.
- Confirm emergency RX/drop is tested separately by the T5D checklist.

## T5D Mock Emergency RX/Drop Checklist

- Confirm ordinary builds keep emergency RX/drop disabled.
- Confirm `WITH_TRANSMIT_CONTROL=1` exposes mock emergency RX/drop only.
- Confirm emergency RX/drop from `tx_requested` returns directly to
  `disarmed`.
- Confirm emergency RX/drop from `tx_active` returns directly to `disarmed`.
- Confirm emergency RX/drop from `rx_requested` returns directly to
  `disarmed`.
- Confirm emergency RX/drop clears runtime arming.
- Confirm emergency RX/drop does not require a mock state-machine step.
- Confirm transmit requests are rejected after emergency until re-armed.
- Confirm emergency RX/drop does not call any backend.

## T5E Callback-Isolation and Validation Closeout Checklist

- Confirm `make transmit-control-safety-audit` passes in ordinary builds.
- Confirm `make transmit-control-safety-audit` passes in guarded mock builds.
- Confirm transmit-control calls appear only in the guarded namespace and its
  dedicated tests.
- Confirm no transmit-control call is made from DSP, live audio, playout, GUI,
  TUI, report, profile, config, batch, playlist, or main CLI paths.
- Confirm no CAT write/control call is made from callbacks.
- Confirm GUI and TUI still have no active transmit controls.
- Confirm profile, config, batch, report, and playlist paths cannot arm
  transmit.
- Confirm no hardware backend exists.
- Confirm no on-air testing is implied by the checklist.
- Run targets that execute `make clean`, including
  `make transmit-control-mock-test`, serially.

## Runtime-arming Checklist

- Require an explicit runtime arming command after process start.
- Keep arming separate from config/profile loading.
- Show arming state separately from read-only CAT PTT readback.
- Require a clear operator action before any future transmit-control action.
- Clear arming state on shutdown, fatal error, and backend failure.

## Receive-only Validation Checklist

- Use a mock or receive-only setup first.
- Treat this section as manual evidence only, not proof of compliance or
  permission for on-air testing.
- Verify CAT readback still works with transmit control disabled.
- Verify emergency RX/drop is callable in the mock path.
- Verify no audio level, cue event, playlist item, batch item, or report event
  can request transmit.
- Verify GUI and TUI remain usable without active transmit controls.
- Keep any future hardware backend work in a separate milestone.

## Dummy-load-only Validation Checklist

- Use only a dummy load or receive-only equivalent.
- Treat this section as manual evidence only, not proof of compliance or
  permission for on-air testing.
- Keep the operator present locally.
- Confirm the selected rig, path, host, port, frequency, mode, and power chain
  outside CarrierPress.
- Confirm emergency RX/drop behavior before any transmit assertion.
- Confirm failure paths return to RX/off or report a clear fault.
- Stop if readback does not match expected rig state.
- Keep any future hardware backend work in a separate milestone.

## Emergency Stop and RX Checklist

- Emergency RX/drop must be available after runtime arming.
- Emergency RX/drop must override TX requests.
- Emergency RX/drop must not wait on audio buffers, GUI rendering, or report
  writes.
- Emergency RX/drop failures must be reported clearly.
- Repeated emergency failures must not loop indefinitely.

## Release Checklist Additions

- Confirm ordinary builds do not compile transmit-control backends.
- Confirm ordinary examples do not use transmit-control options.
- Confirm release notes describe transmit control as deferred unless a future
  T5 implementation is actually selected.
- Confirm docs do not claim RF generation, transmitter compliance, licence
  compliance, certification, legal bandwidth, or regulatory approval.
- Confirm on-air testing is not suggested unless the operator is legally
  permitted and intentionally configured for that test.

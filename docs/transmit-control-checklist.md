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

## Mock-test Checklist

- Start with a mock backend only.
- Prove the initial state is RX/off.
- Prove disarmed requests cannot enter TX.
- Prove runtime arming is required before any transmit request is accepted.
- Prove emergency RX/drop overrides all other requested states.
- Prove timeout or watchdog logic returns to RX/off.
- Prove backend failure enters a fault state and does not retry forever.
- Prove no GUI/TUI key maps to active transmit in ordinary builds.

## Runtime-arming Checklist

- Require an explicit runtime arming command after process start.
- Keep arming separate from config/profile loading.
- Show arming state separately from read-only CAT PTT readback.
- Require a clear operator action before any future transmit-control action.
- Clear arming state on shutdown, fatal error, and backend failure.

## Receive-only Validation Checklist

- Use a mock or receive-only setup first.
- Verify CAT readback still works with transmit control disabled.
- Verify emergency RX/drop is callable in the mock path.
- Verify no audio level, cue event, playlist item, batch item, or report event
  can request transmit.
- Verify GUI and TUI remain usable without active transmit controls.

## Dummy-load-only Validation Checklist

- Use only a dummy load or receive-only equivalent.
- Keep the operator present locally.
- Confirm the selected rig, path, host, port, frequency, mode, and power chain
  outside CarrierPress.
- Confirm emergency RX/drop behavior before any transmit assertion.
- Confirm failure paths return to RX/off or report a clear fault.
- Stop if readback does not match expected rig state.

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

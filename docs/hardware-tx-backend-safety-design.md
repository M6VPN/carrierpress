# Hardware TX Backend Safety Design

T7 is a future hardware transmit-control track. T7A is design only. It does
not add hardware PTT, CAT write/control, hamlib or flrig PTT calls,
serial/GPIO/VOX control, frequency setting, mode setting, or any path that can
key a radio.

The existing T6 operator controls remain guarded and mock-only under
`WITH_TRANSMIT_CONTROL=1`. Ordinary builds remain non-transmit.

## Scope

- Future hardware TX backend work is a separate safety track.
- T7A records requirements and implementation gates only.
- Existing T6 remains mock-only.
- No operational hardware control is added by T7A.
- No GUI, TUI, CLI, profile, config, report, batch, playlist, or selector path
  may arm or request hardware transmit.

## Non-negotiable Gates

A hardware backend must not be implemented until these gates have documented
evidence:

- Compile-time opt-in for hardware transmit-control code.
- Runtime arming before any transmit-control action.
- Explicit local operator confirmation.
- Receive-only validation evidence.
- Dummy-load validation evidence before any backend assertion.
- Emergency RX/drop implemented and tested before hardware calls.
- Backend execution outside real-time callbacks.
- Backend execution outside GUI/TUI draw and render callbacks.
- No hidden arming from config, profile, report, batch, playlist, or selector
  paths.
- No frequency or mode automation unless separately designed.
- No on-air, RF-generation, transmitter-compliance, licence-compliance,
  certification, legal-bandwidth, occupied-bandwidth, or broadcast-quality
  claims.

## Proposed Future Milestones

```text
T7A: Safety design and implementation gates only.
T7B: Backend interface skeleton, disabled by default, no hardware calls.
T7C: Receive-only/manual validation harness and evidence checklist.
T7D: Mock backend parity tests against hardware-backend interface.
T7E: Optional hardware backend prototype, still disabled by default.
T7F: Manual dummy-load validation documentation.
```

T7E and T7F are not part of T7A. They require a separate selected milestone
and fresh review.

## Backend Boundary

A future backend must:

- Receive requests from the existing guarded state-machine boundary.
- Never run in DSP block processing or audio callbacks.
- Never run in GUI or TUI draw/render callbacks.
- Use explicit state transitions.
- Keep runtime arming separate from profile/config loading.
- Support emergency RX/drop before any hardware call is accepted.
- Report failures clearly.
- Fail closed to RX/off/disarmed state.
- Avoid retry loops that can hide a stuck or failed backend.

The backend boundary must stay host-side. It must not become part of DSP,
report generation, batch processing, playlist application, selector
application, or read-only CAT status polling.

## Backend Types Considered

These backend types are candidates for future design work only.

### hamlib PTT backend

Risk: rig model mismatch, network rigctld endpoint mistakes, unexpected rig
state, timeout behavior, and confusion between read-only status and write
control.

Future gate: a hamlib backend requires explicit compile-time opt-in,
receive-only validation, dummy-load validation, clear failure handling, and no
frequency/mode automation unless separately designed.

### flrig PTT backend

Risk: network endpoint mistakes, stale readback, remote XML-RPC availability,
and operator confusion between readback and control.

Future gate: a flrig backend requires explicit compile-time opt-in, local
operator confirmation, receive-only evidence, dummy-load evidence, and
emergency RX/drop coverage before any control call.

### Serial RTS/DTR backend

Risk: wrong serial device, inverted keying, stale line state after process
exit, permission differences, and interface wiring mistakes.

Future gate: a serial backend requires a receive-only harness, explicit device
selection, line-state verification, startup RX/off verification, and shutdown
RX/off behavior.

### GPIO backend

Risk: wrong pin, wrong board, inverted output, retained pin state, and unsafe
system permissions.

Future gate: a GPIO backend requires separate hardware documentation, explicit
pin mapping, receive-only verification, dummy-load verification, and no
package-install or privilege-escalation behavior in project scripts.

### VOX/audio-triggered TX

Risk: accidental transmit from audio presence, noise, playout, monitoring, or
processing state.

Future gate: VOX/audio-triggered TX remains deferred. It must not be added as
an automatic transmit path. Any future discussion requires a separate safety
design.

## Explicitly Deferred

- Hardware PTT.
- CAT write/control.
- Frequency or mode setting.
- Automatic TX from audio level.
- Native station automation.
- On-air validation.
- Compliance or legal-bandwidth assertions.
- Hidden arming from GUI, TUI, CLI, profile, config, report, batch, playlist,
  selector, or example paths.

## T7A Outcome

T7A adds design and evidence templates only. Hardware backend implementation,
hardware control symbols, station automation, and operational transmit support
remain absent.

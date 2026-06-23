# Transmit Control Architecture Note

CarrierPress does not implement PTT or TRANSMIT control. This document records
a future safety boundary for T5 only.

## Boundary

Future transmit control must be separate from DSP. The DSP chain processes
baseband audio samples and must not own station-control state.

Future transmit-control requests should have their own command boundary and
state machine. Read-only CAT snapshots must not become hidden write-control
state.

## Request Flow

Future GUI and TUI controls must issue bounded requests only. They must not
call hardware backends directly.

A controlled outer host loop must consume requests, validate the compile-time
and runtime gates, update the transmit-control state machine, and call a
backend only when all gates pass.

Backend calls must stay outside:

- real-time audio callbacks
- DSP block processing
- GUI render callbacks
- GUI event callbacks
- report generation
- profile or config parsing
- batch processing

## Callback Isolation

T5E adds a static callback and path isolation audit. The audit fails if
transmit-control calls appear outside the guarded transmit-control namespace
and its dedicated tests.

The audit target enforces that transmit-control calls are not made from:

- DSP block processing
- live audio callbacks
- playout callbacks
- GUI render or event callbacks
- report generation
- profile or config parsing
- batch processing
- main CLI workflow paths

No hardware backend exists. The guarded mock state machine remains separate
from audio processing, CAT readback, GUI/TUI controls, profile/config parsing,
report generation, batch processing, and playout.

## Current T5D Namespace

T5B added `cp_transmit_control` as a disabled API namespace. T5C extended that
namespace with a mock-only runtime-arming state machine. Ordinary builds keep
`cp_tx_control_available()` false and initialize the control state as
`disabled`.

With `WITH_TRANSMIT_CONTROL=1`, the scaffold reports that the guarded namespace
was compiled. The guarded mock object starts `disarmed`, must be armed in
memory at runtime, and can move through mock `tx_requested` and `tx_active`
states only inside the local state machine.

T5D adds `cp_tx_control_emergency_rx()`. In the guarded mock build, emergency
RX/drop is an immediate safety override that clears runtime arming and returns
the mock object to `disarmed` from `armed_rx`, `tx_requested`, `tx_active`, or
`rx_requested`. It does not require `cp_tx_control_mock_step()` to leave a mock
TX state. A later transmit request is rejected until the mock object is armed
again.

No backend exists. The guarded mock state machine does not call CAT, hamlib,
flrig, serial, GPIO, VOX, or any other hardware-control path.

## T6A Mock Status Display

T6A makes the guarded mock state visible to operator surfaces as a status line
only. When `WITH_TRANSMIT_CONTROL=1` is built, GUI and TUI safety/status areas
may show the current mock state, arming flag, and active flag from the guarded
state machine.

The T6A display path does not add arm, disarm, transmit-request, or emergency
controls. It does not call CAT, hamlib, flrig, serial, GPIO, VOX, or hardware
backends. Ordinary builds keep transmit control unavailable and do not expose
active TX controls.

## T6B Mock Arm/Disarm Controls

T6B adds guarded operator commands for the mock runtime arming boundary. When
`WITH_TRANSMIT_CONTROL=1` is built, operator surfaces may map `r` to mock arm
and `u` to mock disarm. These commands call only the in-memory mock
`cp_tx_control_arm()` and `cp_tx_control_disarm()` state transitions.

T6B does not add transmit-request controls, emergency RX/drop UI controls, CAT
write/control, hardware backends, hamlib or flrig PTT calls, profile/config
arming, report/batch/playlist transmit fields, or frequency/mode control.
Ordinary builds do not include the mock arm/disarm keys or command namespace.

## T6C Mock TX Request and Emergency RX Controls

T6C adds guarded operator commands for the remaining mock-only control path.
When `WITH_TRANSMIT_CONTROL=1` is built, operator surfaces may map `t` to a
mock TX request and `x` to mock emergency RX/drop. These commands call only the
in-memory mock `cp_tx_control_request_transmit()` and
`cp_tx_control_emergency_rx()` state transitions through the guarded operator
command wrapper.

The mock TX request requires prior mock arming. From `disarmed`, the request is
rejected and the state remains `disarmed`. From `armed_rx`, it may move only to
`tx_requested`; the UI path does not auto-step to `tx_active`.

Mock emergency RX/drop clears runtime arming and returns directly to
`disarmed` from mock TX or RX request states. A later mock TX request requires
the operator to arm the mock object again.

T6C does not add hardware backends, CAT write/control, hamlib or flrig PTT
calls, profile/config arming, report/batch/playlist transmit fields, or
frequency/mode control. Ordinary builds do not include the mock TX request or
emergency RX/drop keys.

## State Machine

A future state machine should start in RX/off state and keep states explicit:

- `disabled`
- `disarmed`
- `armed_rx`
- `tx_requested`
- `tx_active`
- `rx_requested`
- `fault`

Emergency RX/drop overrides all non-fault mock states by clearing arming and
returning to `disarmed`. Fault handling remains conservative and reports an
invalid-state error without entering TX.

## Build and Runtime Gates

Future CAT write/control backends must be compile-time gated. Ordinary builds
must keep CAT read-only.

T5C adds runtime arming for the mock object only. Config files and profiles
must not arm transmit. Reports, batch files, playlists, and GUI workflow
requests must not arm transmit.

`cp_tx_control_request_rx()` remains an ordinary mock state-machine request
that may pass through `rx_requested`. `cp_tx_control_emergency_rx()` is the
immediate mock safety override that clears arming and lands in `disarmed`.

## Backend Scope

Future hardware backends must start mock-only. flrig or hamlib PTT calls must
not be added until mock-only state-machine tests, runtime arming, emergency
RX/drop behavior, and manual receive-only or dummy-load checklist evidence
exist.

No future backend should set rig frequency or rig mode as part of ordinary
audio processing.

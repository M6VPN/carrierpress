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

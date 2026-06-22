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

## Current T5B Namespace

T5B adds `cp_transmit_control` as a disabled API namespace for later mock-only
state-machine work. Ordinary builds keep `cp_tx_control_available()` false and
initialize the control state as `disabled`.

With `WITH_TRANSMIT_CONTROL=1`, the scaffold reports that the guarded namespace
was compiled, but it still has no backend and no transmit-capable transition.
The current request placeholder returns unsupported and leaves state disabled.

## State Machine

A future state machine should start in RX/off state and keep states explicit:

- `disabled`
- `disarmed`
- `armed_rx`
- `tx_requested`
- `tx_active`
- `rx_requested`
- `fault`

Emergency RX/drop must override all other states. Fault handling must prefer
RX/off where possible, report failures clearly, and avoid endless retries.

## Build and Runtime Gates

Future CAT write/control backends must be compile-time gated. Ordinary builds
must keep CAT read-only.

Runtime arming must be required before any transmit-control action. Config
files and profiles must not arm transmit. Reports, batch files, playlists, and
GUI workflow requests must not arm transmit.

## Backend Scope

Future hardware backends must start mock-only. flrig or hamlib PTT calls must
not be added until mock-only state-machine tests, runtime arming, emergency
RX/drop behavior, and manual receive-only or dummy-load checklist evidence
exist.

No future backend should set rig frequency or rig mode as part of ordinary
audio processing.

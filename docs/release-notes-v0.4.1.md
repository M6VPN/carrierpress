# CarrierPress v0.4.1 Release Notes

Suggested tag: `v0.4.1`

CarrierPress v0.4.1 is a conservative T5 safety-gate scaffolding patch after
v0.4.0. It documents and packages mock-only TRANSMIT/PTT safety infrastructure.
It does not add operational transmit support.

## T5 Safety Design

- Added the T5 safety-gate design and test plan.
- Added the transmit-control architecture note.
- Added receive-only and dummy-load validation checklist sections.
- Kept the validation checklist as manual evidence only, not compliance proof
  or permission for on-air testing.

## Compile-Time Guard and Disabled Stubs

- Added `WITH_TRANSMIT_CONTROL`, defaulting to `0`.
- Ordinary builds keep transmit control unavailable and disabled.
- Added a guarded `cp_transmit_control` namespace for future mock testing.
- No hardware backend exists.

## Mock-Only Runtime Arming

- Added a `WITH_TRANSMIT_CONTROL=1` guarded mock state machine.
- Runtime arming is required before a mock TX request can be accepted.
- Config, profiles, reports, batch files, and playlists cannot arm transmit.
- GUI and TUI do not expose active transmit controls.

## Mock Emergency RX/Drop

- Added mock-only emergency RX/drop semantics.
- Emergency RX/drop clears mock runtime arming.
- Emergency RX/drop exits mock TX states directly to `disarmed`.
- A new arm action is required before another mock TX request can be accepted.
- Emergency RX/drop does not call a backend.

## Static Safety Audit

- Added `make transmit-control-safety-audit`.
- The audit checks callback and path isolation for the guarded mock namespace.
- Transmit-control calls are kept out of GUI, TUI, audio, playout, report,
  batch, config, profile, and main CLI workflow paths.
- No CAT write/control backend exists.

## Explicit Non-Goals

- No working PTT.
- No hardware transmit backend.
- No CAT write/control commands.
- No hamlib or flrig PTT calls.
- No serial, GPIO, or VOX transmit control.
- No active TRANSMIT GUI, TUI, CLI, profile, config, report, batch, or playlist
  path.
- No RF generation.
- No transmitter-compliance, regulatory-certification, licence-compliance, or
  legal-bandwidth claims.
- No DSP changes.

## Validation

Before tagging `v0.4.1`, run the ordinary validation profile:

```sh
make clean
make
./carrierpress --version
make test
make -j test
make validate
make quality
make quality-json
make professional-check
make public-header-smoke
make example-libcarrierpress
./build/examples/libcarrierpress-minimal
make transmit-control-safety-audit
make release-check
```

Run guarded mock validation serially:

```sh
make clean
make WITH_TRANSMIT_CONTROL=1
make WITH_TRANSMIT_CONTROL=1 test
make transmit-control-safety-audit
make transmit-control-mock-test
```

Do not run `make transmit-control-mock-test` concurrently with other make
targets because it runs `make clean` internally. Do not run clean-mutating
targets alongside `make -j test`.

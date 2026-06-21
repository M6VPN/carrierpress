# CarrierPress v0.1.1 Release Notes

Suggested tag: `v0.1.1`

CarrierPress v0.1.1 is a patch release after `v0.1.0`. It packages the
post-v0.1.0 install, CI, and examples work so the project is easier to build,
validate, install into a prefix, and review before later DSP milestones.

## What Changed

- Bumped the project version to `0.1.1`.
- Added `carrierpress --version`.
- Added local `install` and `uninstall` targets.
- Added staged install support with `DESTDIR` and configurable `PREFIX`.
- Added static library, public header, pkg-config, and man page installation.
- Added base `carrierpress.pc` metadata without optional dependency
  requirements.
- Added the `carrierpress(1)` man page.
- Added `make install-smoke` for non-root packaging checks.
- Added a GitHub Actions CI matrix for base, optional dependency, and staged
  install profiles.
- Added safe examples and sample command wrappers under `examples/`.
- Added local release validation helpers and documentation.

## DSP and Audio Behavior

This patch release does not intentionally change DSP algorithms or audio
processing behavior. It is focused on release polish, packaging metadata,
validation, and examples.

## CAT and Station Control

CAT support remains read-only. The mock, flrig, and hamlib CAT paths can report
status for display and tests, but CarrierPress does not key transmitters, set
PTT, set frequency, set mode, or send transmit commands.

PTT control remains deferred behind the safety design in
`docs/cat-ptt-safety.md` and the unchecked T5 items in `TODO.md`.

## Safety and Compliance Boundary

CarrierPress processes baseband audio. It does not generate RF, certify
transmitter compliance, provide regulatory approval, or guarantee legal
operation. Users remain responsible for radio regulations, licence limits,
occupied bandwidth, station-control requirements, transmitter limits, and local
rules.

## Validation

Before tagging `v0.1.1`, run:

```sh
make release-check
make install-smoke
./carrierpress --version
./examples/self-test.sh
./examples/cat-mock-status.sh
```

If optional dependencies are available, also run the relevant optional profiles
documented in `docs/release-checklist.md`.

## Known Deferred Work

- PTT control remains deferred.
- MP3 playout remains planned.
- sndio hardware validation remains deferred.
- STM32H753 and CMSIS-DSP support remain planned.
- Screenshot comparison remains manual and non-pixel-perfect.
- RF generation, transmitter control, NRSC certification, C-QUAM, and
  regulatory compliance tooling are not part of this release.

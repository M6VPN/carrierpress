# CarrierPress v0.5 Roadmap

CarrierPress v0.5 focuses on hardening, maintainability, operator workflow
polish, and optional-feature boundaries after the v0.4.0 and v0.4.1 releases.
This roadmap is planning only. It does not implement DSP changes, compressed
audio decoding, CAT write or control commands, hardware PTT, active TRANSMIT
controls, RF generation, transmitter compliance tooling, licence-compliance
tooling, certification claims, legal-bandwidth claims, or regulatory approval
claims.

CarrierPress remains a baseband audio processor and monitor. Users remain
responsible for radio regulations, licence limits, occupied bandwidth,
transmitter limits, station-control requirements, and local rules.

## Release Boundary

v0.4.0 completed GUI playout output-device restart with fallback, sndio GUI
device workflow evaluation, GUI request and output-device display polish, batch
summary reports and evidence workflow, API and packaging hardening, and
optional decoder architecture research.

v0.4.1 completed mock-only T5 safety-gate scaffolding through callback and path
isolation closeout. Ordinary builds still cannot transmit. The guarded
`WITH_TRANSMIT_CONTROL=1` path remains mock-only. No hardware PTT backend, CAT
write/control backend, hamlib or flrig PTT call, active TRANSMIT UI, or
profile/config/report/batch/playlist arming path exists.

v0.5 should build on that boundary without changing audio processing behavior,
moving release publication into project scripts, or turning optional research
tracks into default features. Release publication remains manual and local.

The product-facing roadmap pivot is documented in
[`roadmap-product.md`](roadmap-product.md). It keeps the v0.5 safety boundary
while planning DSP product polish, professional TUI/GUI layout work,
interactive selectors, mock-only TX operator controls, and any future hardware
TX backend as a separate safety track.

## A. M30 Repository and CI Hygiene

v0.5 should reduce validation friction after the v0.4.1 safety-gate work.

Planned work:

- Review validation targets after v0.4.1.
- Reduce duplicated validation logic where practical.
- Document serial versus parallel validation targets.
- Improve make target descriptions.
- Check that clean-mutating targets are not invoked from parallel validation.
- Improve local release evidence consistency.
- Keep validation scripts mutation-safe unless explicitly documented.

## B. M31 Test Matrix Hardening

The base build must remain dependency-light while optional feature validation
stays clear and repeatable.

Planned work:

- Document ordinary and optional build matrix coverage.
- Add or refine guarded mock transmit-control test matrix notes.
- Document optional dependency test matrix expectations.
- Keep optional dependency failures clear and non-fatal for base builds.
- Keep optional dependencies optional.
- Keep the base build dependency-light.

## C. M32 Operator Workflow Polish

Operator surfaces should remain consistent without adding transmit controls or
changing audio behavior.

Planned work:

- Review GUI, TUI, help text, and documentation consistency.
- Improve the examples index.
- Improve safe local demo scripts.
- Keep TRANSMIT and PTT absent from ordinary operator workflow.
- Keep GUI and TUI text restrained, bounded, and safe.

## D. M33 Report and Evidence Workflow Polish

Report tooling should stay useful for engineering review and regression work.

Planned work:

- Improve report summary and compare UX.
- Improve batch evidence examples.
- Review evidence bundle workflow wording.
- Keep reports as engineering metrics only.
- Do not claim compliance, legal bandwidth proof, transmitter compliance,
  licence compliance, regulatory approval, or broadcast-quality proof.

## E. M34 API and Packaging Polish

The public header and packaging surfaces should be reviewed after v0.4.1 use.

Planned work:

- Review public headers after v0.4.1.
- Review `carrierpress_core.h` and `carrierpress_tooling.h` after example use.
- Review the install manifest.
- Review pkg-config smoke coverage.
- Improve package maintainer notes if needed.
- Keep optional dependencies out of base package metadata.
- Keep optional dependencies out of base pkg-config metadata.

## F. M35 Future Optional Feature Research

Future optional features remain research tracks unless explicitly selected.

Planned work:

- Review optional decoder architecture without adding decoder libraries.
- Keep compressed audio formats external by default.
- Keep native MP3, FLAC, OGG, Opus, M4A, and AAC decoding out of the base
  build.
- Keep the base build WAV and PCM native.
- Review future transmit backend requirements without adding a hardware backend.
- Keep hardware transmit backend work out of v0.5 by default.
- Require a separate T6 or T7 safety milestone before any hardware transmit
  backend work is selected.
- Keep CAT write/control commands, hamlib/flrig PTT calls, active TRANSMIT UI,
  and profile/config/report/batch/playlist arming paths absent.

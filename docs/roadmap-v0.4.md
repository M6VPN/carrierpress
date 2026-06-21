# CarrierPress v0.4 Roadmap

CarrierPress v0.4 should focus on GUI workflow parity, safer playout and
device operations, report automation, and maintainability after the v0.3.0
release. This roadmap is planning only. It does not implement DSP changes, CAT
write or control commands, PTT control, active TRANSMIT controls, RF
generation, transmitter compliance tooling, licence-compliance tooling,
certification claims, legal-bandwidth claims, or regulatory approval claims.

CarrierPress remains a baseband audio processor and monitor. Users remain
responsible for radio regulations, licence limits, occupied bandwidth,
transmitter limits, station-control requirements, and local rules.

## Release Boundary

v0.3.0 completed the v0.3 operator-workflow milestones: effective config and
profile inspection, batch offline WAV processing, schema-versioned reports with
summary and compare helpers, TUI and GUI operator polish, library and API
examples, package-maintainer notes, and GUI workflow requests for WAV and
playlist cueing plus live PortAudio output-device restart with fallback.

v0.4 should build on that boundary without rewriting the v0.3.0 release notes,
changing existing audio processing defaults, or moving release publication into
project scripts. Release publication remains manual and local.

GUI playout output-device restart and sndio GUI output-device restart remain
optional follow-ups unless explicitly selected. PTT and TRANSMIT control remain
deferred to the separate T5 safety gate in `TODO.md` and
[`cat-ptt-safety.md`](cat-ptt-safety.md).

## A. GUI Playout Workflow Parity

The live PortAudio GUI path can apply deferred output-device requests outside
callbacks. v0.4 may extend that pattern to WAV playout if selected.

Planned work:

- Add GUI playout output-device restart if selected.
- Keep restart outside GUI callbacks.
- Keep restart outside real-time audio callbacks.
- Preserve CLI-only playout behavior.
- Preserve TUI playout controls.
- Keep WAV and playlist cueing validated before application.
- Keep compressed formats external unless a decoder milestone is explicitly
  selected.

## B. sndio GUI and Device Workflow Evaluation

sndio remains optional and manually validated. Any GUI device workflow for
sndio should be designed around what the backend can support safely.

Planned work:

- Document what sndio can support safely.
- Add sndio GUI output-device restart only if technically safe.
- Keep sndio optional.
- Keep Linux PortAudio path unchanged.
- Keep OpenBSD and sndio validation manual unless CI support appears.

## C. GUI File and Device Workflow Polish

The GUI now has deferred request types and safe cue slots. v0.4 should make
those states easier for an operator to inspect without changing the callback
boundary.

Planned work:

- Add clearer GUI state for queued WAV and playlist paths.
- Add selectable output-device display if enumeration is available.
- Add safer pending-request status display.
- Add error display for rejected cue and device requests.
- Keep GUI text clipped to panels.
- Keep file and device changes outside callbacks.

## D. Batch and Report Automation Polish

Batch processing and report tools are now usable. v0.4 can improve evidence
collection for repeatable offline runs while keeping reports as engineering
metrics only.

Planned work:

- Add a batch summary report for whole batch runs.
- Add a batch report comparison helper.
- Add a report bundle or evidence directory workflow.
- Document report bundle fields.
- Keep reports as engineering metrics only.
- Do not claim compliance, legal bandwidth proof, transmitter compliance,
  licence compliance, regulatory approval, or broadcast-quality proof.

## E. API and Packaging Hardening

v0.3 added public header tiers and packaging maintainer notes. v0.4 should use
feedback from those surfaces to tighten docs and smoke checks without changing
the base dependency profile.

Planned work:

- Review `carrierpress_core.h` and `carrierpress_tooling.h` after real use.
- Add more public-header smoke examples.
- Improve package-maintainer notes from v0.3 feedback.
- Keep optional dependencies out of base pkg-config metadata.

## F. Optional Decoder Research, Not Default Implementation

CarrierPress remains WAV and PCM native by default. Compressed audio support
should not enter the base build by accident.

Planned work:

- Document possible optional decoder architecture.
- Keep native MP3, FLAC, OGG, Opus, and M4A out of the base build.
- Do not add decoder libraries unless explicitly selected later.
- Keep the external decode workflow as the default.

## G. Deferred T5 TRANSMIT and PTT Remain Separate

TRANSMIT, PTT, and CAT write/control work is not normal v0.4 workflow work.
Any future transmit-control feature must start from the T5 safety gate.

Planned constraints:

- No TRANSMIT or PTT implementation in normal v0.4 workflow.
- Any GUI TRANSMIT toggle is T5-only.
- T5 requires compile-time opt-in.
- T5 requires runtime arming.
- T5 requires mock-only tests before hardware backend work.
- T5 requires emergency RX/drop handling.
- T5 requires a manual dummy-load or receive-only validation checklist.
- T5 requires no real-time callback CAT control.
- No CAT write/control commands in profiles, config files, reports, batch
  processing, or ordinary GUI work.

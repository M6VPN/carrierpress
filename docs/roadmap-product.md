# CarrierPress Product Roadmap

CarrierPress is moving toward an enthusiast-friendly audio processing and
station-operator tool while keeping safety gates, optional dependencies, and
host workflow boundaries clear.

This roadmap is planning plus foundation work. It does not implement DSP
algorithm changes, hardware PTT, CAT write/control commands, hamlib or flrig
PTT calls, active TX controls in ordinary builds, native compressed-audio
decoding, RF generation, transmitter compliance tooling, licence-compliance
tooling, certification claims, legal-bandwidth claims, occupied-bandwidth
proof, broadcast-quality proof, release publication, or package installation.

## A. P36 DSP Product Polish

Planned work:

- Review AM and SSB processing presets.
- Improve subjective listening defaults only when supported by tests.
- Add clearer preset descriptions.
- Add A/B comparison helpers if useful.
- Keep DSP changes measurable and regression-tested.
- Keep report and preset docs free of compliance claims.

## B. P37 Professional TUI and GUI Layout Polish

Planned work:

- Improve dashboard layout hierarchy.
- Make meters and status panels clearer.
- Keep help text consistent across CLI, TUI, and GUI docs.
- Group processing, playout, device, report, and safety state cleanly.
- Keep long labels, paths, device names, and reasons bounded or truncated.
- Keep active TX controls absent from ordinary builds.

## C. P38 Interactive Selectors

Planned work:

- Add output-device selector workflow.
- Add audio-file selector workflow.
- Add playlist selector workflow.
- Support cue/load requests from selector choices.
- Add TUI keyboard navigation.
- Add GUI list navigation.
- Keep selector labels and paths bounded.
- Keep native file dialogs deferred unless selected later.
- Keep compressed audio external-conversion by default.

The first implementation slice adds a shared dependency-light selector model
for output-device, audio-file, and playlist choices. It does not scan the file
system, enumerate devices, open audio devices, process audio, or change UI
behavior.

## D. T6 Mock TX Operator Controls

Planned work:

- Build only under `WITH_TRANSMIT_CONTROL=1`.
- Use the existing mock transmit-control state machine only.
- Require runtime arming before mock TX requests.
- Make emergency RX/drop visible in mock-only UI surfaces.
- Keep ordinary builds free of TX controls.
- Keep hardware backend absent.
- Keep CAT write/control absent.
- Keep hamlib and flrig PTT calls absent.
- Keep profile, config, report, batch, and playlist arming paths absent.

## E. T7 Future Hardware TX Backend

This is a future and separate safety track. It is not part of this patch.

Required boundaries before any future implementation:

- Separate safety design.
- Explicit operator opt-in.
- Receive-only validation evidence.
- Dummy-load validation evidence before any backend assertion.
- CAT write/control execution outside callbacks.
- No GUI, TUI, CLI, profile, config, report, batch, or playlist hidden arming.
- No frequency or mode automation unless separately designed.
- No on-air claims or compliance claims.

## F. P39 Enthusiast Onboarding

Planned work:

- Add a concise quick-start guide.
- Improve safe demo flows.
- Document recommended validation commands.
- Keep optional dependency setup clear.
- Keep project scripts free of package installation and `sudo`.
- Explain ordinary, optional, and guarded mock workflows without implying
  hardware transmit support.

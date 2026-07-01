# CarrierPress Product Roadmap

CarrierPress is moving toward an enthusiast-friendly audio processing and
station-operator tool while keeping safety gates, optional dependencies, and
host workflow boundaries clear.

The product-roadmap pivot is packaged for v0.5.0 in
[`release-notes-v0.5.0.md`](release-notes-v0.5.0.md). Local readiness checks
are listed in
[`release-readiness-v0.5.0.md`](release-readiness-v0.5.0.md).

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

The first implementation slice adds an enthusiast-facing preset guide,
listening notes template, safe preset review script, and clearer shipped
profile comments. It does not change DSP algorithms or profile parameter
values.

## B. P37 Professional TUI and GUI Layout Polish

Planned work:

- Add shared dashboard section labels.
- Improve dashboard layout hierarchy.
- Make meters and status panels clearer.
- Keep help text consistent across CLI, TUI, and GUI docs.
- Group processing, playout, selectors, device, workflow, and safety state
  cleanly.
- Keep long labels, paths, device names, and reasons bounded or truncated.
- Keep active TX controls absent from ordinary builds.

The first implementation slice adds shared dashboard section labels and applies
them to the TUI/GUI display foundation without changing DSP, selector,
workflow, audio, or transmit-control behavior.

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

The first implementation slices add a shared dependency-light selector model
plus output-device, audio-file, and playlist selector workflow foundations.
They do not scan the file system, open file dialogs, open audio devices,
process audio, decode compressed formats, or add transmit behavior.

## D. T6 Mock TX Operator Controls

T6 mock operator controls:

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

T7A adds the safety design and validation evidence template only:

- [`hardware-tx-backend-safety-design.md`](hardware-tx-backend-safety-design.md)
- [`hardware-tx-validation-template.md`](hardware-tx-validation-template.md)

No hardware backend exists after T7A.

## F. P39 Enthusiast Onboarding

Planned work:

- Add a concise quick-start guide.
- Improve safe demo flows.
- Document recommended validation commands.
- Keep optional dependency setup clear.
- Keep project scripts free of package installation and `sudo`.
- Explain ordinary, optional, and guarded mock workflows without implying
  hardware transmit support.

## G. B40 SSB Bulletin and Audio Playout Pivot

CarrierPress is now aimed at automated SSB voice bulletin and audio playout
processing, not receiver-side file reconstruction.

Planned work:

- Treat WAV, future decoded audio, text/TTS, live microphone, and stream inputs
  as audio bulletin sources.
- Normalize source audio into a common internal sample format.
- Keep SSB voice profiles separate from external digital modem audio.
- Use `hf-ssb-voice` as the default SSB bulletin profile.
- Keep `data-clean-pass-through` available for JS8, Olivia, MT63, and similar
  external digital modes.
- Add carousel scheduling, station ID insertion, and dry-run planning.
- Add preview WAV output before any hardware-output workflow.
- Keep CAT, serial, RTS/DTR, GPIO, and VOX control disabled by default.
- Require explicit `--arm-tx` before any future CAT or serial PTT backend can
  be accepted.
- Keep all hardware PTT work deferred to a separate safety-gated milestone.

The first implementation slice adds built-in bulletin profile names, shipped
profile files, a dependency-light bulletin/carousel plan model, safe dry-run
CLI subcommands, and preview WAV routing through the existing libsndfile
processor.

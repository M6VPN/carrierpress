# CarrierPress v0.3 Roadmap

CarrierPress v0.3 should focus on operator polish, inspectability, safer batch
workflows, and better evidence and report handling after the v0.2.0 release.
This roadmap is planning only. It does not implement DSP changes, CAT write or
control commands, PTT control, RF generation, transmitter compliance tooling,
certification claims, legal-bandwidth claims, or regulatory approval claims.

CarrierPress remains a baseband audio processor and monitor. Users remain
responsible for radio regulations, licence limits, occupied bandwidth,
transmitter limits, station-control requirements, and local rules.

## Release Boundary

v0.2.0 completed the v0.2 workflow milestones: strict profiles, strict config
files, playlist checking and cue/status output, JSON engineering reports,
OpenBSD and sndio validation notes, and packaging polish.

v0.3 should build on that boundary without rewriting the v0.2.0 release notes
or changing the existing audio processing defaults. Release publication remains
manual and local.

PTT control remains deferred to the separate T5 safety gate in `TODO.md` and
`docs/cat-ptt-safety.md`. It is not part of the normal v0.3 planning unless T5
is explicitly selected later.

## A. Effective Configuration and Profile Inspection

Profiles and config files are now usable at runtime. The next useful operator
step is showing exactly what CarrierPress resolved after defaults, config
files, profiles, and command-line overrides.

Planned work:

- Add `--print-effective-config`.
- Add `--validate-profile PATH`.
- Add `--validate-config PATH`.
- Show profile and config source paths.
- Show final resolved audio-chain settings after command-line overrides.
- Show host workflow settings that affect live, playout, TUI, GUI, and reports.
- Keep CAT, PTT, rig-control, transmit-state, and station-control settings out
  of ordinary profile and config inspection.

## B. Batch Offline WAV Workflow

Offline WAV processing should become safer and easier to repeat for file sets.
The batch path should remain WAV-native and should not add compressed-audio
decoders unless a later optional decoder milestone is selected.

Planned work:

- Define a simple batch input-list format.
- Add batch dry-run validation.
- Add safe output-directory planning.
- Avoid overwriting existing files unless explicitly requested.
- Generate sidecar reports per processed output file.
- Keep compressed formats external unless a later optional decoder milestone is
  selected.
- Keep batch processing outside the real-time audio path.

## C. Report Schema and Compare Tools

v0.2 added JSON quality reports and processed-file sidecar reports. v0.3 should
make those reports easier to compare and safer to consume in scripts.

Planned work:

- Add an explicit report schema version field.
- Document stable report fields.
- Add a report summary mode.
- Add a report compare helper for two JSON reports.
- Keep reports as engineering metrics only.
- Avoid claims that reports prove legal bandwidth, transmitter compliance,
  regulatory approval, licence compliance, or broadcast quality.

## D. TUI and GUI Operator Polish

The TUI and GUI should remain optional operator interfaces. v0.3 should make
their current state displays clearer while keeping the CLI fully usable.

Planned work:

- Display active profile and config state.
- Display report sidecar state where relevant.
- Improve cue and status visibility.
- Improve spectrum and level labels.
- Improve screenshot and evidence metadata.
- Keep GUI rendering outside real-time audio callbacks.
- Keep CLI operation independent from TUI and GUI support.

## E. Library and API Examples

CarrierPress now installs public headers and `libcarrierpress.a`. v0.3 should
make the library boundary easier to review without promising long-term ABI
stability too early.

Planned work:

- Add a minimal `libcarrierpress` usage example.
- Review public headers.
- Document stable and experimental API areas.
- Keep internal-only implementation details private where practical.
- Keep optional host backends separate from the DSP core API.

## F. Packaging Polish After v0.2

The current packaging support is intentionally simple. v0.3 should document
expectations for maintainers without turning the repo into distro-specific
packaging.

Planned work:

- Add a package-maintainer checklist.
- Add example Debian packaging notes.
- Add example OpenBSD port notes.
- Add portable packaging notes for source-based builds.
- Keep source tarball and checksum generation local and manual.
- Keep release publication out of project scripts and CI.

## G. Deferred PTT Remains T5 Only

PTT control is not a v0.3 workflow item. Any future transmit-control work must
start from the T5 safety gate and must remain separate from the DSP core,
profiles, config files, report tools, batch processing, and real-time audio
callbacks.

Planned constraints:

- No PTT implementation in v0.3 unless T5 is explicitly selected.
- No CAT write/control commands in profile, config, report, or batch work.
- No automatic transmit based on audio presence.
- No CAT control from real-time audio callbacks.
- No regulatory, licence, legal-bandwidth, RF-generation, or
  transmitter-compliance claim.

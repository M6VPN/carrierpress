# CarrierPress v0.2 Roadmap

CarrierPress v0.2 should focus on practical audio workflow improvements after
the v0.1.1 patch release. This roadmap is planning only. It does not add DSP
features, station-control commands, PTT control, RF generation, transmitter
compliance claims, certification claims, or regulatory approval claims.

CarrierPress remains a baseband audio processor and monitor. Users remain
responsible for radio regulations, licence limits, occupied bandwidth,
transmitter limits, station-control requirements, and local rules.

## Release Boundary

v0.1.1 covers install, packaging, CI, examples, and release-prep polish after
v0.1.0. v0.2 should build on that foundation without rewriting the v0.1.0 or
v0.1.1 release notes.

PTT control remains deferred to the separate T5 safety gate in `TODO.md` and
`docs/cat-ptt-safety.md`. It is not part of the normal v0.2 audio-workflow
roadmap.

## A. Preset and Profile Files

The next useful operator workflow is external preset and profile files. The
goal is to make repeatable AM, SSB, and file-cleanup setups easier to review
and share without adding arbitrary unsafe runtime editing.

Planned work:

- Define a small profile file format.
- Add strict validation for every profile field.
- Support named AM, SSB, and audio-chain profiles.
- Add example profiles for `am-safe`, `am-shortwave`, `ssb-speech`, and
  `file-cleanup`.
- Document command-line override precedence.
- Keep profile loading outside the real-time audio callback.
- Keep PTT and station-control settings out of normal audio profiles.

## B. Config-file Support

CarrierPress should gain optional config-file support for host workflow
settings that are currently command-line only.

Planned work:

- Add an optional config file path.
- Define default config search rules.
- Make command-line options override config values.
- Reject unknown or invalid config keys.
- Keep config parsing outside real-time audio callbacks.
- Keep auto-transmit, PTT, and station-control settings out of ordinary audio
  config files.

## C. Better Playout Workflow

The current WAV playout path is useful, but v0.2 should improve operator
feedback and file-list handling.

Planned work:

- Improve playlist diagnostics with file and line details.
- Add playlist dry-run validation.
- Add simple cue and status output.
- Improve unsupported-format messages.
- Document external decode workflows for formats outside current WAV support.
- Keep MP3, FLAC, and OGG support separate and optional unless selected for a
  later implementation milestone.

## D. Audio Measurement and Export Reports

CarrierPress already has validation and quality fixtures. v0.2 should add
operator-facing reports for processed files and test runs.

Planned work:

- Add machine-readable JSON or text reports.
- Export measured peak, RMS, crest factor, AGC state, limiter state, and
  enabled processing stages where available.
- Add an optional processed-file sidecar report.
- Document report fields as engineering metrics, not compliance proof.
- Avoid claims that reports prove legal bandwidth, transmitter compliance, or
  broadcast quality.

## E. GUI and TUI Polish

The ncurses TUI and SDL3 GUI should remain optional host interfaces. v0.2
should make them clearer without replacing the CLI.

Planned work:

- Display the active profile name and profile source.
- Improve spectrum scale labels.
- Make saved screenshots and release evidence easier to compare manually.
- Keep GUI rendering outside real-time audio callbacks.
- Keep the CLI and TUI fully usable without GUI support.

## F. OpenBSD and sndio Validation

Linux remains the active host path. v0.2 can improve OpenBSD notes and sndio
validation without regressing the Linux PortAudio path.

Planned work:

- Add OpenBSD build notes.
- Add a manual sndio validation checklist.
- Document sndio device notes.
- Keep sndio optional.
- Keep Linux PortAudio validation unchanged.

## G. Packaging Polish

v0.1.1 adds install and pkg-config support. v0.2 should improve packaging notes
without publishing releases automatically.

Planned work:

- Add distro packaging notes.
- Review install paths and staged install output.
- Document source tarball checksum workflow.
- Keep release publication manual and local.
- Keep optional dependencies out of the base pkg-config file.

## H. Deferred PTT Remains T5 Only

PTT control is not a v0.2 audio-workflow item. Any future transmit-control work
must start from the T5 safety gate and must remain separate from the DSP core.

Planned constraints:

- No PTT implementation in v0.2 unless T5 is explicitly selected.
- No CAT write/control commands in the audio profile or config work.
- No automatic transmit based on audio presence.
- No real-time callback CAT control.
- No regulatory or transmitter-compliance claim.

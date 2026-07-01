# Changelog

## 0.6.0-dev - unreleased

### Added

- Post-v0.5.0 SSB bulletin/audio playout development cycle.
- B40A SSB bulletin workflow foundation.

### Changed

- Main branch now tracks post-v0.5.0 development toward SSB voice bulletin and
  audio playout workflows.

### Safety

- Hardware TX backend remains absent.
- CAT/serial PTT remains planning/validation only.
- Ordinary workflows remain dry-run, preview, or audio-only unless future
  explicitly gated work is selected.

### Deferred

- Native MP3/Opus decoding.
- Native TTS.
- Stream/URL input.
- ALSA/PipeWire named-device output beyond existing host backends.
- Hardware PTT/CAT/serial TX control.

## 0.5.0 - 2026-07-01

### Added

- Enthusiast-facing DSP preset guide and listening notes template.
- Safe DSP preset review, operator quick-start, and report evidence flows.
- TUI/GUI dashboard section labels and bounded status formatting polish.
- Output-device, WAV audio-file, and playlist selector workflow foundations.
- Guarded mock-only TX status, arm/disarm, mock TX request, and emergency
  RX/drop operator controls under `WITH_TRANSMIT_CONTROL=1`.
- Hardware TX backend safety design and future validation evidence template.
- v0.5.0 release-readiness checklist.
- Final v0.5.0 release evidence template.
- Manual v0.5.0 tag and release checklist.

### Changed

- README scope now describes the v0.5.0 enthusiast product-readiness line.
- Release, validation, packaging, operator, selector, and quick-start docs now
  point to v0.5.0 readiness material.

### Safety

- Ordinary builds remain non-transmit.
- Guarded transmit-control builds remain mock-only.
- Hardware TX backend remains absent.
- CAT write/control remains absent.
- hamlib/flrig PTT calls remain absent.
- serial/GPIO/VOX TX control remains absent.
- Frequency and mode setting remain absent.
- Optional dependencies remain opt-in.

### Deferred

- Native compressed-audio decoding.
- Native file dialogs.
- Hardware TX backend implementation.
- T7B and later backend interface or validation follow-ups.
- DSP refinements that require new test-backed behavior changes.

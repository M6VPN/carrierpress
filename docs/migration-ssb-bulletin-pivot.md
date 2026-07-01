# CarrierPress SSB Bulletin Pivot Migration Note

CarrierPress v0.5.x started as a broad DSP, workflow, selector, and safety-gate
foundation. The next line pivots the product direction toward automated SSB
voice bulletin and audio playout processing.

The receiver hears or records audio. CarrierPress does not attempt recoverable
file transfer at the receiver.

## What Changes

- New built-in bulletin profiles are available:
  - `hf-ssb-voice`
  - `hf-ssb-narrow`
  - `vhf-fm-voice`
  - `am-broadcast-style`
  - `data-clean-pass-through`
- New safe subcommands are available:
  - `ssb-play`
  - `bulletin`
  - `live`
  - `carousel`
- Carousel schedules can be dry-run planned from a small TOML-like format.
- `--profile hf-ssb-voice` and related built-in names work without a profile
  file.

## What Does Not Change

- Existing `--input` / `--output` WAV processing remains available.
- Existing `--play`, `--playlist`, `--live`, GUI, TUI, report, batch, and
  selector work remains available.
- CAT status remains read-only in ordinary builds.
- Hardware PTT remains absent.
- `WITH_TRANSMIT_CONTROL=1` remains guarded mock-only unless later safety work
  changes that boundary.

## Safety Boundary

No command keys a transmitter by default. CAT or serial PTT cannot be accepted
without `--arm-tx`, and this slice still has no hardware backend to execute
PTT.

Use `data-clean-pass-through` for external digital modem audio. Do not use the
SSB voice profiles for modem/data tones.

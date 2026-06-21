# CarrierPress API Notes

CarrierPress installs public headers and `libcarrierpress.a` for small C
programs that want to use the baseband audio processing core. The current API
is suitable for examples, local tooling, and experiments. It is not a frozen
long-term ABI yet, and v0.x releases may still adjust source interfaces.

CarrierPress does not generate RF, certify a transmitter, prove legal
bandwidth, or provide regulatory approval. The library API does not implement
PTT control or CAT write/control commands.

## Recommended Includes

New applications should include the narrowest umbrella that matches their use:

- `carrierpress_core.h` for stable in-memory DSP processing.
- `carrierpress_tooling.h` for profile, config, batch, and report tools.
- `carrierpress.h` only when older code expects the broad compatibility
  umbrella.

The broad umbrella remains installed and compatible, but it includes more
headers than ordinary library users normally need.

## Stable Core Areas

The most stable public area is the dependency-light DSP block interface:

- `carrierpress_core.h`
- `cp_types.h`
- `cp_block.h`
- `cp_monitor.h`
- `cp_meter.h`
- `cp_version.h`

These headers expose sample types, status codes, block configuration,
processor initialization, block processing, reset, and monitor snapshots. New
fields may be added, but existing basic block-processing calls are intended to
remain source-compatible where practical.

The preset/config helpers for AM, SSB, AGC, limiter, compressor, crossover,
dehummer, multiband, and bass EQ are also public and useful for applications
that build explicit processing chains. Treat detailed struct fields as v0.x
source interfaces, not as a stable binary ABI.

## Tooling APIs

The profile, config-file, batch, report, and report-tool headers are public for
local tooling:

- `carrierpress_tooling.h`
- `cp_profile.h`
- `cp_config_file.h`
- `cp_batch.h`
- `cp_report.h`
- `cp_report_tool.h`

These APIs are intended for command-line helpers, validation tools, and batch
workflows. They are stable enough for scripts and examples, but field additions
or stricter validation may appear in v0.x releases.

## Compatibility Umbrella

`carrierpress.h` is the broad compatibility umbrella. It remains available for
existing source code, but new applications should prefer `carrierpress_core.h`
or `carrierpress_tooling.h` so they do not depend on unrelated header tiers.

## Experimental and Internal-Facing Areas

Some installed headers expose implementation details because CarrierPress is
still a small static-library project. Treat these as experimental:

- GUI/TUI formatting and monitor helper headers.
- Operator-state and screenshot evidence helpers.
- Restoration, declipper, auto-EQ, natural dynamics, low-level boost, and
  other algorithm-specific internal state structs.
- Batch WAV and host playout helpers.

Prefer `cp_block.h`, presets, profiles, and report APIs for external programs.
Avoid depending on internal state layouts unless you are developing CarrierPress
itself.

## Host-Specific Optional Areas

Host and optional backend headers are not part of the portable core API:

- PortAudio live/playout support.
- sndio live-audio support.
- ncurses TUI support.
- SDL3 GUI support.
- FFTW spectrum monitor support.
- flrig and hamlib read-only CAT backends.

Base builds and `carrierpress.pc` do not require these optional dependencies.
Build with the matching `WITH_*` flag before using optional backend functions.

## Minimal Example

Build and run the in-tree example without installing CarrierPress:

```sh
make example-libcarrierpress
./build/examples/libcarrierpress-minimal
```

After a staged or local install, an installed build can use pkg-config:

```sh
cc -std=c17 -Wall -Wextra -o libcarrierpress-minimal \
  examples/libcarrierpress-minimal.c \
  $(pkg-config --cflags --libs carrierpress)
```

The example uses the block processor in memory only. It does not open audio
devices, read or write files, use optional host backends, or control CAT/PTT.

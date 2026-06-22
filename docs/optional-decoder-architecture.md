# Optional Decoder Architecture Research

CarrierPress is WAV/PCM-native by default. MP3, FLAC, OGG, Opus, M4A, AAC, and
container inputs remain external-conversion workflows unless a later explicit
implementation milestone selects optional decoder support.

CarrierPress does not call `ffmpeg` itself. Existing playlist and batch
checkers should continue rejecting compressed inputs in the base build with a
clear message to convert to WAV first.

## Current Position

The current supported path is:

1. Convert compressed or container media outside CarrierPress.
2. Check the resulting WAV files with `--playlist-check` or `--batch-check`.
3. Process or play WAV inputs through the existing WAV/PCM path.

This keeps codec parsing, codec licensing, and external tool selection outside
the base CarrierPress build. The default build remains dependency-light and
does not require decoder libraries.

## Design Goals

Any future decoder milestone should keep these constraints:

- Decoder support is optional only.
- Decoder support is compile-time gated.
- Feature availability is visible in the runtime feature summary.
- The base build dependency list does not change.
- The existing WAV path remains unchanged.
- Batch preflight safety remains unchanged.
- The external decode workflow remains documented and supported.
- Unsupported formats continue to fail with clear messages.
- Decoder work stays outside real-time audio callbacks.

## Architecture Options

### Option 1: External Predecode Workflow

This is the current and default workflow. Users convert files with an external
tool, then give CarrierPress WAV input.

Benefits:

- No codec dependency burden in CarrierPress.
- No codec licensing burden in the base package.
- Predictable WAV/PCM input path.
- Simple packaging and validation.

Tradeoffs:

- Users manage conversion tools themselves.
- Workflows need an extra conversion step.

### Option 2: Optional libsndfile-expanded Decode

Some libsndfile builds may support formats beyond WAV, depending on platform
and distribution configuration. CarrierPress must not assume that support is
available.

If this path is selected later:

- Detect each supported input format explicitly.
- Keep support behind a compile-time flag.
- Preserve current WAV behavior.
- Keep unsupported compressed formats rejected when support is unavailable.
- Document distro-specific behavior clearly.

Risk:

- Format support may vary between systems, which can make batch behavior less
  predictable.

### Option 3: Optional ffmpeg/libav Backend

A future `WITH_FFMPEG=1` or similar build flag could decode supported media to
PCM blocks before the existing DSP path.

Requirements:

- Keep ffmpeg/libav out of the base build.
- Keep the dependency and licensing impact visible to package maintainers.
- Decode into PCM outside real-time callbacks.
- Reuse existing WAV/PCM DSP processing after decode.
- Add malformed-input tests and resource limits before enabling normal use.

Risks:

- Large dependency surface.
- Codec and container parsing attack surface.
- More complex packaging and licensing review.

### Option 4: External Helper Process

A future milestone could allow a user-supplied helper to provide PCM or WAV
data to CarrierPress.

Benefits:

- Codec implementation stays outside CarrierPress.
- Users can choose the helper that fits their platform.

Requirements:

- Avoid shell invocation with untrusted paths.
- Validate helper configuration explicitly.
- Bound process output and error handling.
- Keep helper use optional and visible.
- Keep external conversion docs as the default path.

Risks:

- Process lifecycle and error handling become part of the workflow.
- Pipe or stream handling must be careful to avoid hangs and partial reads.

## Security and Safety Considerations

Compressed media and containers are untrusted parser inputs. Any future decoder
support should include:

- File size and duration limits.
- Clear memory and CPU bounds.
- Malformed-file tests.
- Path handling that avoids shell evaluation.
- No automatic external tool execution.
- No network fetching.
- No package installation.
- Clear failure modes and non-zero exits on decode failure.

Decoder code must not run in real-time audio callbacks. It should decode to
bounded PCM buffers or files before the existing processing path consumes the
audio.

## Packaging Considerations

The base package should remain decoder-free. Optional decoder packages, if ever
selected, should be split or feature-gated by maintainers.

The base `carrierpress.pc` metadata must not include decoder libraries. Optional
decoder dependencies must not leak into base pkg-config output, base public
header smoke tests, or the default build.

The source archive and checksum workflow stays local and unchanged. Adding an
optional decoder backend must not publish releases, create tags, install system
packages, or modify system audio configuration.

## Future Milestone Checklist

If decoder support is selected later:

- Define a `WITH_DECODER_*` or backend-specific feature flag.
- Add feature-summary output.
- Add configure or build checks for the selected backend.
- Add tiny fixture tests for enabled and disabled builds.
- Keep unsupported formats rejected unless the matching backend is enabled.
- Preserve and link the external conversion docs.
- Document codec, container, licensing, and packaging implications.
- Add malformed-file or fuzz-style tests if practical.
- Keep decoder work outside real-time callbacks.
- Keep base builds WAV/PCM-native.

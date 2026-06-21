# CarrierPress TODO

This file tracks processable follow-up milestones. Keep these items as future
work until they are selected for implementation. Do not mark a milestone
implemented until code, docs, and validation are complete.

## Rules for Future Work

- [ ] Keep the DSP core independent from host, UI, CAT, and GUI backends.
- [ ] Keep optional dependencies optional.
- [ ] Do not use `sudo` or install packages from project commands.
- [ ] If a dependency is missing, print the package or library name.
- [ ] Keep Linux with PortAudio, libsndfile, and ncurses as the active host path.
- [ ] Do not claim RF generation, transmitter compliance, or regulatory approval.
- [ ] Keep AM and SSB processing as baseband audio-chain processing.
- [ ] Keep every real-time audio callback malloc-free and print-free.

## T1 TUI Usability Cleanup

- [x] Define the target ncurses style as a clear Borland-like operator panel.
- [x] Split the screen into stable regions for transport, mode, devices, meters,
      processing chain, and key help.
- [x] Make AM, SSB, and neutral mode state visually unambiguous.
- [x] Show when AM controls are locked because SSB mode is active, and when SSB
      controls are locked because AM mode is active.
- [x] Group input meters, output meters, AGC state, limiter state, and stream
      flags in a consistent meter panel.
- [x] Group dehummer, restoration, declipper, dynamics, low-level boost, AGC,
      multiband, bass EQ, AM, SSB, and limiter state in chain order.
- [x] Add a compact key legend that changes with live, play, and playlist mode.
- [x] Keep TUI support behind `WITH_TUI=1`.
- [x] Keep TUI commands preset-based until arbitrary parameter editing has a
      dedicated validation pass.
- [x] Add non-interactive tests for TUI state formatting where practical.
- [x] Add manual terminal-size test notes for 80x24 and wider terminals.

## T2 Build Auto-Detection Cleanup

- [x] Add a documented `make autodetect` target or default build detection path.
- [x] Detect available libsndfile support.
- [x] Detect available PortAudio support.
- [x] Detect available ncurses support.
- [x] Detect available sndio support without making sndio active Linux work.
- [x] Reserve detection hooks for future hamlib or flrig support.
- [x] Print a feature summary before building optional targets.
- [x] Print missing package or library names without installing anything.
- [x] Keep explicit `WITH_SNDFILE=1`, `WITH_PORTAUDIO=1`, `WITH_TUI=1`, and
      `WITH_SNDIO=1` overrides working.
- [x] Keep `make` deterministic under parallel builds.
- [x] Ensure generated object directories remain separated by feature set.
- [x] Update README build commands after auto-detection is implemented.
- [x] Validate `make`, `make test`, `make -j test`, and optional builds.

## T3 flrig and hamlib CAT Control

- [x] Add a clean CAT backend boundary before any protocol implementation.
- [x] Keep CAT support optional and outside the DSP core.
- [x] Define build flags for CAT support only after dependency probing is settled.
- [x] Support read-only rig status first.
- [x] Read frequency.
- [x] Read mode.
- [x] Read PTT state.
- [ ] Add optional PTT control only after safety checks are documented.
- [x] Support flrig XML-RPC or compatible control through a small host backend.
- [x] Support hamlib only as an optional backend if development headers are
      available.
- [x] Add a mock or simulator test path before requiring hardware.
- [x] Show CAT status in the TUI without blocking audio processing.
- [x] Document that users remain responsible for licence limits and station
      control requirements.

## T4 Optional GUI Monitor

- [x] Keep the GUI optional and separate from the DSP core.
- [x] Do not replace the ncurses TUI.
- [x] Choose the GUI toolkit after dependency and portability review.
- [x] Reuse existing monitor snapshot data where possible.
- [x] Display processed output waveform.
- [x] Display processed output spectrum.
- [x] Display input and output peak/RMS meters.
- [x] Display AGC, limiter, AM, SSB, multiband, bass EQ, and restoration state.
- [x] Keep GUI rendering off the real-time audio callback.
- [x] Add a manual GUI smoke-test checklist.
- [x] Add screenshot or visual-regression notes only after a toolkit is chosen.
- [x] Keep the CLI fully usable without GUI support.

## T5 Optional PTT Control - Deferred

- [ ] Implement an explicit compile-time PTT-control gate.
- [ ] Implement a runtime arming gate.
- [ ] Implement mock-only PTT state-machine tests.
- [ ] Implement emergency RX/drop command.
- [ ] Add a manual dummy-load hardware test checklist.
- [ ] Add flrig PTT control only after mock tests pass.
- [ ] Add hamlib PTT control only after mock tests pass.
- [ ] Keep all PTT commands out of real-time callbacks.

## R1 v0.1 Release Hardening

- [x] Add a v0.1 release checklist.
- [x] Add a reproducible release validation helper.
- [x] Keep required release checks dependency-light.
- [x] Keep optional dependency checks opt-in.
- [x] Document local Hamlib 4.7.1 validation without hardcoding the path.
- [x] Preserve read-only CAT and deferred PTT-control status.

## R2 v0.1 Release Notes and Procedure

- [x] Add v0.1 release notes with safety and limitation wording.
- [x] Add a manual local GitHub release procedure.
- [x] Document example local release evidence assets.
- [x] Keep tag creation and GitHub release publication as manual commands.
- [x] Keep release docs free of RF generation, transmitter compliance, and
      regulatory approval claims.
- [x] Preserve read-only CAT and deferred PTT-control status.

## R3 Install and Packaging Polish

- [x] Add local `install` and `uninstall` Makefile targets.
- [x] Support staged installs with `DESTDIR` and configurable `PREFIX`.
- [x] Install the binary, public headers, static library, pkg-config metadata,
      and man page.
- [x] Add base pkg-config metadata without optional dependency requirements.
- [x] Add a `carrierpress(1)` man page.
- [x] Add `carrierpress --version`.
- [x] Add a non-root `install-smoke` target for packaging review.

## R4 CI Matrix

- [x] Add GitHub Actions base validation workflow.
- [x] Add staged install and package smoke validation.
- [x] Add optional dependency build profiles where runner packages are
      available.
- [x] Document the GUI and SDL3 manual validation gap.
- [x] Add README CI badge.
- [x] Keep releases manual and local.
- [x] Keep PTT control deferred.

## R5 Examples and Sample Configs

- [x] Add examples README.
- [x] Add safe self-test examples.
- [x] Add WAV processing and playout examples.
- [x] Add GUI demo and screenshot examples.
- [x] Add read-only CAT mock, flrig, and hamlib examples.
- [x] Add local Hamlib 4.7.1 validation example.
- [x] Keep all examples free of PTT control and release publication.

## R6 v0.1.1 Patch Release Prep

- [x] Bump project version to 0.1.1.
- [x] Add v0.1.1 release notes.
- [x] Add manual v0.1.1 release procedure.
- [x] Update release checklist for v0.1.1.
- [x] Keep v0.1.0 historical notes unchanged.
- [x] Keep PTT control deferred.
- [x] Keep release publication manual and local.

## V2 v0.2 Planning

- [x] Add v0.2 roadmap document.
- [x] Keep PTT control deferred to T5.
- [x] Preserve v0.1.1 release boundary.

## M11 Preset and Profile Files

- [x] Define profile file format.
- [x] Add profile parser with strict validation.
- [x] Add AM-safe, AM-shortwave, SSB-speech, and file-cleanup example profiles.
- [x] Add profile CLI option.
- [x] Add profile tests.
- [x] Document command-line override precedence.
- [x] Keep PTT and station-control settings out of ordinary audio profiles.

## M12 Config File Support

- [x] Define config file scope.
- [x] Add config parser.
- [x] Add config validation tests.
- [x] Document default config search paths.
- [x] Keep command-line options overriding config values.
- [x] Keep PTT/control settings out of ordinary audio config.

## M12B Runtime Config Loading

- [x] Add `--config PATH`.
- [x] Apply config files before later command-line overrides.
- [x] Load profile paths named by config files.
- [x] Document runtime config precedence.
- [x] Keep config parsing outside real-time audio callbacks.

## M13 Playout Workflow Polish

- [x] Improve playlist diagnostics.
- [x] Add playlist dry-run validation.
- [x] Add cue/status output.
- [x] Document external decode workflow.
- [x] Keep unsupported formats clearly reported.
- [x] Keep MP3, FLAC, and OGG support separate and optional unless selected.

## M14 Measurement Report Export

- [x] Add machine-readable quality report output.
- [x] Add processed-file report sidecar option.
- [x] Document report fields as engineering metrics, not compliance proof.
- [x] Keep reports free of transmitter-compliance and regulatory claims.

## M15 OpenBSD/sndio Validation

- [x] Add OpenBSD build notes.
- [x] Add sndio manual validation checklist.
- [x] Add sndio device notes.
- [x] Keep Linux PortAudio path unchanged.

## M16 Packaging and Release Polish

- [x] Add distro packaging notes.
- [x] Review install paths and staged install output.
- [x] Document source tarball checksum workflow.
- [x] Keep release publication manual and local.

## R7 v0.2.0 Release Prep

- [x] Bump project version to 0.2.0.
- [x] Add v0.2.0 release notes.
- [x] Update manual v0.2.0 release procedure.
- [x] Update release checklist for v0.2.0.
- [x] Preserve v0.1.0 and v0.1.1 historical notes.
- [x] Keep PTT control deferred.
- [x] Keep release publication manual and local.

## V3 v0.3.0 Planning

- [x] Add v0.3 roadmap document.
- [x] Keep PTT control deferred to T5.
- [x] Preserve v0.2.0 release boundary.

## M17 Effective Config/Profile Inspection

- [x] Add `--print-effective-config`.
- [x] Add `--validate-profile PATH`.
- [x] Add `--validate-config PATH`.
- [x] Show profile/config source paths.
- [x] Document final override precedence in inspection output.
- [x] Keep PTT and station-control settings out of ordinary inspection output.

## M18 Batch Offline WAV Workflow

- [x] Define batch input-list format.
- [x] Add batch dry-run validation.
- [x] Add safe output-directory handling.
- [x] Avoid overwriting files unless explicitly requested.
- [x] Generate sidecar reports per processed file.
- [x] Keep compressed formats external unless selected later.

## M19 Report Schema and Compare Tools

- [x] Add report schema version field.
- [x] Document stable report fields.
- [x] Add report summary mode.
- [x] Add report compare helper.
- [x] Keep reports free of transmitter-compliance and regulatory claims.

## M20 TUI/GUI Operator Polish

- [x] Display active profile/config state.
- [x] Display cue/status context more clearly.
- [x] Improve spectrum and level labels.
- [x] Improve screenshot/evidence metadata.
- [x] Keep CLI usable without TUI/GUI.

## M21 Library/API Examples

- [x] Add minimal library usage example.
- [x] Review public headers.
- [x] Document stable and experimental API areas.
- [x] Keep internal implementation details private where practical.

## M22 Packaging Maintainer Notes

- [x] Add package-maintainer checklist.
- [x] Add example distro packaging notes.
- [x] Keep source tarball/checksum workflow local and manual.

## M23 Future GUI Workflow Ideas

- [x] Load/cue WAV files from GUI.
- [x] Load/cue playlist entries from GUI.
- [x] Select output sound interface from GUI.
- [x] Keep GUI file/device changes outside real-time callbacks.
- [x] Keep GUI transmit controls deferred to T5.

## M23B GUI File and Playlist Cueing

- [x] Add GUI-safe WAV path cue request.
- [x] Add GUI-safe playlist cue request.
- [x] Validate paths outside GUI callbacks.
- [x] Keep compressed formats external.

## M23C GUI Output Device Selection

- [x] Display output device choices.
- [x] Add deferred output-device selection request.
- [x] Reopen and restart audio outside callbacks.
- [x] Preserve CLI-only output-device selection.

## M23C2 GUI Output Stream Restart

- [x] Apply deferred output-device requests in the outer host loop.
- [x] Stop and reopen audio streams outside GUI callbacks.
- [ ] Preserve safe fallback on device-open failure.
- [x] Keep CLI-only output-device selection working.

## M23C3 GUI Output Device Follow-up

- [ ] Add safe fallback to the previous output device on restart-open failure.
- [ ] Add GUI playout output-device restart if selected.
- [ ] Add sndio GUI output-device restart if selected.

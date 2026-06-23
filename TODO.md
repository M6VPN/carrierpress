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

- [x] Implement an explicit compile-time PTT-control gate.
- [x] Implement a runtime arming gate.
- [x] Implement mock-only PTT state-machine tests.
- [x] Implement emergency RX/drop command.
- [x] Add a manual dummy-load hardware test checklist.
- [ ] Add flrig PTT control only after mock tests pass.
- [ ] Add hamlib PTT control only after mock tests pass.
- [x] Keep all PTT commands out of real-time callbacks.

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
- [x] Preserve safe fallback on device-open failure.
- [x] Keep CLI-only output-device selection working.

## Deferred GUI Output Device Follow-up

- [ ] Add sndio GUI output-device restart if selected.
- [ ] Add manual OpenBSD validation evidence for sndio GUI restart.

## R8 v0.3.0 Release Prep

- [x] Bump project version to 0.3.0.
- [x] Add v0.3.0 release notes.
- [x] Update manual v0.3.0 release procedure.
- [x] Update release checklist for v0.3.0.
- [x] Preserve v0.1.0, v0.1.1, and v0.2.0 historical notes.
- [x] Keep PTT control deferred.
- [x] Keep release publication manual and local.

## V4 v0.4.0 Planning

- [x] Add v0.4 roadmap document.
- [x] Keep PTT/TRANSMIT control deferred to T5.
- [x] Preserve v0.3.0 release boundary.

## M24 GUI Playout Workflow Parity

- [x] Add GUI playout output-device restart if selected.
- [x] Keep playout restart outside GUI callbacks.
- [x] Keep playout restart outside real-time audio callbacks.
- [x] Preserve CLI-only playout behaviour.
- [x] Preserve TUI playout controls.

## M25 sndio GUI Device Workflow Evaluation

- [x] Document sndio GUI output-device constraints.
- [x] Evaluate sndio GUI output-device restart safety.
- [x] Keep sndio optional.
- [x] Keep Linux PortAudio path unchanged.
- [x] Keep OpenBSD/sndio validation manual unless CI support appears.

## M26 GUI File and Device Workflow Polish

- [x] Improve queued WAV/playlist display.
- [x] Improve pending request status display.
- [x] Improve rejected request error display.
- [x] Display output-device choices where enumeration is available.
- [x] Keep GUI text restrained to panels.

## M26B GUI Output Device Choice Display

- [x] Display PortAudio output-device choices where enumeration is available.
- [x] Keep enumeration optional and outside real-time callbacks.
- [x] Keep sndio named-device selection documented separately.

## M27 Batch and Report Evidence Bundles

- [x] Add batch-level summary report.
- [x] Add batch report comparison helper.
- [x] Add evidence directory workflow.
- [x] Document report bundle fields.
- [x] Keep reports free of transmitter-compliance and regulatory claims.

## M28 API and Packaging Hardening

- [x] Review `carrierpress_core.h` after example use.
- [x] Add more public-header smoke examples.
- [x] Review package-maintainer notes after v0.3.
- [x] Keep optional dependencies out of base pkg-config metadata.

## M29 Optional Decoder Architecture Research

- [x] Document optional decoder interface options.
- [x] Keep compressed audio formats external by default.
- [x] Do not add decoder libraries unless selected later.
- [x] Keep base build WAV/PCM-native.

## R9 v0.4.0 Release Prep

- [x] Bump project version to 0.4.0.
- [x] Add v0.4.0 release notes.
- [x] Update manual v0.4.0 release procedure.
- [x] Update release checklist for v0.4.0.
- [x] Preserve historical release notes.
- [x] Keep PTT/TRANSMIT control deferred.
- [x] Keep decoder implementation deferred.
- [x] Keep release publication manual and local.

## T5 Deferred TRANSMIT/PTT Safety Gate

- [x] Keep TRANSMIT/PTT out of ordinary GUI workflow.
- [x] Require compile-time opt-in before any transmit-control build.
- [x] Require runtime arming before any transmit-control action.
- [x] Add mock-only transmit-control tests before hardware backend work.
- [x] Add emergency RX/drop handling.
- [x] Keep CAT control out of real-time callbacks.
- [x] Require manual dummy-load or receive-only validation checklist.

## T5A Safety-Gate Design

- [x] Expand CAT/PTT safety design.
- [x] Add transmit-control checklist.
- [x] Add transmit-control architecture note.
- [x] Keep this patch documentation-only with no working PTT or CAT
      write/control.

## T5B Compile-Time Guard

- [x] Add disabled-by-default transmit-control build guard.
- [x] Add inert transmit-control API namespace.
- [x] Add tests proving ordinary builds cannot transmit.
- [x] Keep hardware PTT/CAT write backends absent.
- [x] Keep GUI/TUI TRANSMIT controls absent.

## T5C Mock Runtime-Arming State Machine

- [x] Add mock-only runtime arming state machine.
- [x] Keep ordinary builds disabled.
- [x] Add guarded `WITH_TRANSMIT_CONTROL=1` mock tests.
- [x] Keep hardware PTT/CAT write backends absent.
- [x] Keep GUI/TUI TRANSMIT controls absent.
- [x] Keep config/profile/batch/report paths unable to arm transmit.

## T5D Mock Emergency RX/Drop

- [x] Add mock-only emergency RX/drop API.
- [x] Emergency RX/drop clears runtime arming.
- [x] Emergency RX/drop exits mock TX states without requiring a mock step.
- [x] Add guarded tests for emergency RX/drop.
- [x] Keep hardware PTT/CAT write backends absent.
- [x] Keep GUI/TUI TRANSMIT controls absent.

## T5E Callback Isolation and Validation Closeout

- [x] Add static transmit-control callback/path isolation audit.
- [x] Add Makefile target for transmit-control safety audit.
- [x] Document receive-only and dummy-load validation boundaries.
- [x] Document serial validation for targets that run `make clean`.
- [x] Keep hardware PTT/CAT write backends absent.
- [x] Keep GUI/TUI/CLI/profile/config/report/batch transmit paths absent.

## R10 v0.4.1 Release Prep

- [x] Bump project version to 0.4.1.
- [x] Add v0.4.1 release notes.
- [x] Update manual v0.4.1 release procedure.
- [x] Update release checklist for T5 safety-gate validation.
- [x] Update release evidence helper for transmit-control safety audit.
- [x] Keep T5 hardware backend absent.
- [x] Keep release publication manual and local.

## V5 v0.5 Roadmap Planning

- [x] Add v0.5 roadmap document.
- [x] Define post-v0.4.1 milestone groups.
- [x] Keep hardware PTT backend out of v0.5 default scope.
- [x] Keep decoder implementation out of v0.5 default scope.
- [x] Keep release publication manual.

## M30 Repository and CI Hygiene

- [x] Review validation targets after v0.4.1.
- [x] Document serial-only clean-mutating targets.
- [x] Improve local release evidence consistency.
- [x] Keep validation scripts mutation-safe unless explicitly documented.

## M30A Validation Target Hygiene

- [x] Add validation target guide.
- [x] Add non-mutating validation help target.
- [x] Document serial-only clean-mutating targets.
- [x] Refresh release evidence helper without adding publishing actions.
- [x] Keep validation/release scripts free of tag, push, release, sudo,
      package-install, and transmit actions.

## M31 Test Matrix Hardening

- [x] Document ordinary and optional build matrix.
- [x] Add or refine guarded mock transmit-control test matrix notes.
- [x] Keep optional dependencies optional.
- [x] Keep base build dependency-light.

## M31A Test Matrix Hardening

- [x] Add test matrix guide.
- [x] Add non-mutating test matrix help target.
- [x] Document guarded mock transmit-control validation.
- [x] Document optional dependency validation boundaries.
- [x] Keep optional dependency failures out of base build requirements.

## M32 Operator Workflow Polish

- [x] Review GUI/TUI/help consistency.
- [x] Review examples index and safe demo scripts.
- [x] Keep TRANSMIT/PTT absent from ordinary operator workflow.
- [x] Keep GUI/TUI text restrained and safe.

## M32A Operator Workflow Polish

- [x] Add operator workflow guide.
- [x] Review CLI/GUI/TUI help wording for consistency.
- [x] Refresh examples index and safe demo notes.
- [x] Keep ordinary operator workflow free of active TRANSMIT/PTT controls.
- [x] Keep GUI/TUI text bounded and restrained.
- [x] Add non-mutating operator workflow safety audit.

## M33 Report and Evidence Workflow Polish

- [x] Review report summary and compare UX.
- [x] Review batch evidence examples.
- [x] Keep reports engineering-only.
- [x] Keep compliance/proof claims absent.

## M33A Report and Evidence Workflow Polish

- [x] Add report evidence workflow guide.
- [x] Polish report summary/compare wording and tests.
- [x] Refresh report and batch evidence examples.
- [x] Keep report tooling inspection-only.
- [x] Keep compliance/proof claims absent.

## M34 API and Packaging Polish

- [x] Review public headers after v0.4.1.
- [x] Review install manifest and pkg-config smoke.
- [x] Review package maintainer notes.
- [x] Keep optional dependencies out of base package metadata.

## M34A API and Packaging Surface Polish

- [x] Add API/package surface guide.
- [x] Add non-mutating packaging surface audit.
- [x] Add non-mutating packaging help target.
- [x] Review public header tiers after v0.4.1.
- [x] Review install manifest and pkg-config smoke documentation.
- [x] Keep optional dependencies out of base package metadata.

## M35 Future Optional Feature Research

- [x] Review optional decoder architecture without adding decoder libraries.
- [x] Review future transmit backend requirements without adding hardware
      backend.
- [x] Keep base build WAV/PCM-native.
- [x] Keep hardware transmit backend deferred unless a separate safety milestone
      is selected.

## V6 Product Roadmap Pivot

- [x] Add enthusiast product roadmap.
- [x] Define DSP polish milestone.
- [x] Define TUI/GUI layout polish milestone.
- [x] Define interactive selector milestone.
- [x] Define mock-only TX operator-control milestone.
- [x] Keep hardware TX backend deferred to a separate safety milestone.

## P36 DSP Product Polish

- [x] Review AM/SSB preset descriptions and defaults.
- [x] Add listening/regression notes for DSP changes.
- [x] Improve DSP preset UX without compliance claims.
- [x] Keep DSP changes measurable and tested.

## P36A DSP Preset UX and Listening Notes

- [x] Add DSP preset/product guide.
- [x] Add listening notes template.
- [x] Review shipped AM/SSB/profile comments.
- [x] Add safe DSP preset review workflow.
- [x] Keep DSP reports and notes free of compliance/proof claims.

## P37 TUI and GUI Layout Polish

- [x] Review dashboard layout hierarchy.
- [x] Improve TUI/GUI panel grouping and labels.
- [x] Improve meter/status readability.
- [x] Keep GUI/TUI strings bounded and restrained.

## P37A Dashboard Layout Polish

- [x] Add shared dashboard section labels.
- [x] Improve GUI/TUI display grouping and labels.
- [x] Keep selector/workflow/device/status lines bounded.
- [x] Keep ordinary operator UI free of active TX/PTT controls.
- [x] Update operator workflow docs for dashboard grouping.

## P38 Interactive File and Device Selectors

- [x] Add shared selector model for UI/TUI.
- [x] Add output-device selector workflow.
- [x] Add audio-file cue/load selector workflow.
- [x] Add playlist selector workflow.
- [x] Keep selector paths bounded and safe.

## P38A Selector Foundation

- [x] Add shared selector model.
- [x] Add selector tests.
- [x] Add bounded selector formatting.
- [x] Document selector workflow direction.
- [x] Keep selectors dependency-light and hardware-free.

## P38B Output-Device Selector Workflow

- [x] Map output-device candidates into shared selector model.
- [x] Display output-device selector state in operator UI surfaces where available.
- [x] Keep PortAudio enumeration optional.
- [x] Keep sndio named-device workflow separate.
- [x] Keep device changes outside callbacks and real-time paths.

## P38C Audio-File Cue/Load Selector Workflow

- [x] Map explicit audio-file candidates into shared selector model.
- [x] Display WAV cue/load selector state in operator UI surfaces where available.
- [x] Keep compressed formats disabled and external-conversion only.
- [x] Keep file loading/cueing outside callbacks and real-time paths.
- [x] Keep native file dialogs and directory scanning deferred.

## P38D Playlist Selector Workflow

- [x] Map explicit playlist candidates into shared selector model.
- [x] Display playlist selector state in operator UI surfaces where available.
- [x] Validate playlist candidates before applying workflow requests.
- [x] Keep playlist loading/cueing outside callbacks and real-time paths.
- [x] Keep native file dialogs and directory scanning deferred.

## T6 Mock TX Operator Controls

- [x] Add guarded mock-only TX status panel.
- [x] Add guarded mock-only runtime arm/disarm controls.
- [x] Add guarded mock-only TX request and emergency RX/drop controls.
- [x] Keep ordinary builds free of TX controls.
- [x] Keep hardware backend absent.

## T6A Mock TX Status Panel

- [x] Add guarded mock-only TX status formatter.
- [x] Display mock TX state in operator safety/status surfaces when guarded build is enabled.
- [x] Keep this patch display-only with no arm/disarm/TX request controls.
- [x] Keep ordinary builds free of active TX controls.
- [x] Keep hardware backend absent.

## T6B Mock Arm/Disarm Controls

- [x] Add guarded mock-only arm/disarm operator commands.
- [x] Show guarded mock-only arm/disarm help text only when `WITH_TRANSMIT_CONTROL=1`.
- [x] Keep arm/disarm controls mock-state-only.
- [x] Keep this patch free of TX request and emergency UI controls.
- [x] Keep ordinary builds free of arm/disarm TX controls.
- [x] Keep hardware backend absent.

## T6C Mock TX Request and Emergency RX Controls

- [x] Add guarded mock-only TX request operator command.
- [x] Add guarded mock-only emergency RX/drop operator command.
- [x] Show guarded mock-only help text only when `WITH_TRANSMIT_CONTROL=1`.
- [x] Keep TX request and emergency controls mock-state-only.
- [x] Keep ordinary builds free of TX controls.
- [x] Keep hardware backend absent.
- [x] Keep CAT write/control absent.

## T7 Future Hardware TX Backend

- [ ] Keep deferred until explicitly selected.
- [ ] Require separate safety design before implementation.
- [ ] Require receive-only and dummy-load validation evidence.
- [ ] Keep CAT write/control outside callbacks.
- [ ] Keep frequency/mode automation out of scope unless separately designed.

## P39 Enthusiast Onboarding

- [ ] Add quick-start guide.
- [ ] Improve safe demo flows.
- [ ] Document recommended enthusiast validation commands.
- [ ] Keep optional dependency setup manual and explicit.

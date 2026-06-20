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

- [ ] Keep the GUI optional and separate from the DSP core.
- [ ] Do not replace the ncurses TUI.
- [ ] Choose the GUI toolkit after dependency and portability review.
- [ ] Reuse existing monitor snapshot data where possible.
- [ ] Display processed output waveform.
- [ ] Display processed output spectrum.
- [ ] Display input and output peak/RMS meters.
- [ ] Display AGC, limiter, AM, SSB, multiband, bass EQ, and restoration state.
- [ ] Keep GUI rendering off the real-time audio callback.
- [ ] Add a manual GUI smoke-test checklist.
- [ ] Add screenshot or visual-regression notes only after a toolkit is chosen.
- [ ] Keep the CLI fully usable without GUI support.

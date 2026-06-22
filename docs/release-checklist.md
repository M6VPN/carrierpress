# CarrierPress Release Checklist

This checklist is for preparing CarrierPress release tags. CarrierPress is a
portable audio-processing foundation for legal transmitter audio chains and
test loads. It is not an RF generator, transmitter controller, certified
compliance tool, or licence-compliance tool.

## Pre-release Assumptions

- The DSP core remains C17 and dependency-light.
- Optional host features stay behind explicit build flags.
- CAT support remains read-only.
- PTT control is not implemented.
- AM and SSB modes are baseband audio-chain processing only.
- Users remain responsible for radio regulations, licence limits, transmitter
  limits, occupied bandwidth, and local station-control rules.

## Required Base Validation

Target scheduling, parallel-safe target guidance, and serial-only target notes
are documented in `docs/validation-targets.md`. The short local guide is:

```sh
make validation-help
```

Ordinary, guarded mock, and optional dependency build profiles are documented
in `docs/test-matrix.md`. The short local matrix guide is:

```sh
make test-matrix-help
```

Run the default release validation profile:

```sh
make release-check
```

The default profile runs:

```sh
make clean
make
make test
make -j test
make validate
make quality
make professional-check
make autodetect
```

`make release-check` runs `make clean`, so run it serially. Do not run
clean-mutating targets concurrently with `make -j test` or other build targets.

These checks must pass before a release tag. They use deterministic fixtures and
do not prove broadcast-processor quality, RF compliance, transmitter safety, or
legal operation.

Optional local engineering evidence may be saved with:

```sh
mkdir -p build
make -s quality-json > build/quality-report.json
```

The JSON quality report is for software regression review only. It does not
prove RF bandwidth, transmitter compliance, regulatory approval, or legal
operation.

GitHub Actions CI must also be green before a release tag is created. CI
coverage is documented in `docs/ci.md`.

## v0.4.1 Release Checks

Use this section when preparing `v0.4.1`, the patch release for T5 safety-gate
scaffolding after v0.4.0.

- Confirm `./carrierpress --version` prints `CarrierPress 0.4.1`.
- Confirm staged install pkg-config metadata reports `Version: 0.4.1`.
- Confirm `docs/release-notes-v0.4.1.md` is the release body.
- Confirm ordinary build tests pass.
- Confirm guarded mock build tests pass with `WITH_TRANSMIT_CONTROL=1`.
- Confirm `make transmit-control-safety-audit` passes.
- Confirm `make transmit-control-mock-test` passes when run serially.
- Confirm `make validation-help` prints the current target classes.
- Confirm `make test-matrix-help` prints the current build/test matrix.
- Confirm `docs/validation-targets.md` matches the release procedure.
- Confirm `docs/test-matrix.md` documents ordinary, guarded mock, and optional
  dependency profiles.
- Confirm `scripts/release-evidence.sh` remains a non-mutating reporter.
- Confirm ordinary builds keep transmit control unavailable.
- Confirm guarded builds remain mock-only.
- Confirm guarded mock transmit-control validation remains serial-only.
- Confirm optional dependency builds remain opt-in and are not required for the
  base build.
- Confirm no hardware transmit backend exists.
- Confirm no hamlib or flrig PTT calls exist.
- Confirm no CAT write/control backend exists.
- Confirm no GUI, TUI, CLI, profile, config, report, batch, or playlist
  transmit path exists.
- Confirm receive-only and dummy-load checklist boundaries are reviewed.
- Confirm `make dist-check` creates and verifies the source tarball checksum.
- Confirm GitHub Actions CI is green before tagging.
- Confirm no release docs or package metadata claim RF generation, transmitter
  compliance, licence compliance, regulatory approval, legal bandwidth, or
  broadcast quality.
- Confirm release publication remains manual and local per
  `docs/release-procedure.md`.

## v0.4.0 Release Checks

Use this section when preparing `v0.4.0`, the release for v0.4 workflow-polish
milestones M24 through M29.

- Confirm `./carrierpress --version` prints `CarrierPress 0.4.0`.
- Confirm staged install pkg-config metadata reports `Version: 0.4.0`.
- Confirm `docs/release-notes-v0.4.0.md` is the release body.
- Confirm historical release notes remain historical and unchanged except for
  index-style links elsewhere.
- Confirm GUI playout output-device restart and fallback docs are present in
  `docs/gui-workflow.md`.
- Confirm sndio GUI and device evaluation docs are present in
  `docs/sndio-gui-device-workflow.md`.
- Confirm GUI request, status, and device display docs are present in
  `docs/gui-workflow.md`.
- Confirm batch summary and evidence workflow docs are present in
  `docs/batch-workflow.md`.
- Confirm batch summary comparison is documented in
  `docs/measurement-reports.md` and `docs/batch-workflow.md`.
- Confirm optional decoder architecture docs are present in
  `docs/optional-decoder-architecture.md`.
- Confirm `make public-header-smoke` passes.
- Confirm `make pkg-config-smoke` passes and optional dependencies are absent
  from base pkg-config metadata.
- Confirm `make dist-check` creates and verifies the source tarball checksum.
- Confirm GitHub Actions CI is green before tagging.
- Confirm no decoder libraries or native compressed-audio support were added.
- Confirm no CAT write/control commands were added.
- Confirm T5 PTT and TRANSMIT control remain deferred and unchecked in
  `TODO.md`.
- Confirm no release docs or package metadata claim RF generation, transmitter
  compliance, licence compliance, regulatory approval, legal bandwidth, or
  broadcast quality.
- Confirm release publication remains manual and local per
  `docs/release-procedure.md`.

## v0.3.0 Release Checks

Use this section when preparing `v0.3.0`, the release for v0.3 operator-polish
milestones M17 through M23.

- Confirm `./carrierpress --version` prints `CarrierPress 0.3.0`.
- Confirm staged install pkg-config metadata reports `Version: 0.3.0`.
- Confirm `docs/release-notes-v0.3.0.md` is the release body.
- Confirm `docs/release-notes-v0.1.md`, `docs/release-notes-v0.1.1.md`, and
  `docs/release-notes-v0.2.0.md` remain historical and unchanged except for
  index-style links elsewhere.
- Confirm profile validation works through `./examples/validate-profile.sh`.
- Confirm config validation works through `./examples/validate-config.sh`.
- Confirm effective config inspection works through
  `./examples/print-effective-config.sh`.
- Confirm effective config output does not include PTT, transmit, CAT
  write/control, rig-control, or station-control fields.
- Confirm batch dry-run validation works through `./examples/batch-check.sh`.
- Confirm batch processing and per-file sidecar reports are documented in
  `docs/batch-workflow.md`.
- Confirm report schema versioning, summary, and comparison are documented in
  `docs/measurement-reports.md`.
- Confirm `--report-summary` and `--report-compare` work on
  `build/quality-report.json`.
- Confirm TUI and GUI operator-state notes are present in
  `docs/gui-workflow.md`.
- Confirm live PortAudio GUI output-device restart and fallback are documented
  in `docs/gui-workflow.md`.
- Confirm GUI playout and sndio output-device restart follow-ups remain
  deferred unless explicitly selected.
- Confirm `make public-header-smoke` passes.
- Confirm `make example-libcarrierpress` builds and
  `./build/examples/libcarrierpress-minimal` runs.
- Confirm API notes are present in `docs/api.md`.
- Confirm package maintainer notes are present in
  `docs/package-maintainer-checklist.md`.
- Confirm `make install-manifest` prints the expected staged files.
- Confirm `make dist-check` creates and verifies the source tarball checksum.
- Confirm GitHub Actions CI is green before tagging.
- Confirm no release docs or package metadata claim RF generation, transmitter
  compliance, licence compliance, regulatory approval, legal bandwidth, or
  broadcast quality.
- Confirm CAT remains read-only and no CAT write/control commands were added.
- Confirm T5 PTT control remains deferred and unchecked in `TODO.md`.
- Confirm release publication remains manual and local per
  `docs/release-procedure.md`.

## v0.2.0 Release Checks

Use this section when preparing `v0.2.0`, the release for v0.2 audio-workflow
milestones M11 through M16.

- Confirm `./carrierpress --version` prints `CarrierPress 0.2.0`.
- Confirm staged install pkg-config metadata reports `Version: 0.2.0`.
- Confirm `docs/release-notes-v0.2.0.md` is the release body.
- Confirm `docs/release-notes-v0.1.md` and `docs/release-notes-v0.1.1.md`
  remain historical and unchanged except for index-style links elsewhere.
- Confirm profile examples validate through `./examples/profile-self-test.sh`.
- Confirm config examples validate through `./examples/config-self-test.sh`.
- Confirm playlist checking works through `./examples/playlist-check.sh`.
- Confirm `make quality-json` produces valid JSON.
- Confirm processed-file sidecar reporting is documented in
  `docs/measurement-reports.md`.
- Confirm OpenBSD and sndio notes are present in `docs/openbsd-sndio.md`.
- Confirm packaging notes are present in `docs/packaging.md`.
- Confirm package maintainer notes are present in
  `docs/package-maintainer-checklist.md`.
- Confirm Debian-style, OpenBSD ports-style, and portable source packaging
  notes are present under `docs/`.
- Confirm `make install-manifest` prints the expected staged files.
- Confirm `make dist-check` creates and verifies the source tarball checksum.
- Confirm GitHub Actions CI is green before tagging.
- Confirm no release docs or package metadata claim RF generation, transmitter
  compliance, licence compliance, regulatory approval, legal bandwidth, or
  broadcast quality.
- Confirm CAT remains read-only and no CAT write/control commands were added.
- Confirm T5 PTT control remains deferred and unchecked in `TODO.md`.
- Confirm release publication remains manual and local per
  `docs/release-procedure.md`.

## v0.1.1 Patch Release Checks

Use this section when preparing `v0.1.1`, the patch release for post-v0.1.0
install, packaging, CI, and examples work.

- Confirm `./carrierpress --version` prints `CarrierPress 0.1.1`.
- Confirm `make install-smoke` passes.
- Confirm `make install-manifest` prints only the expected staged files.
- Confirm `make dist-check` creates and verifies the source tarball checksum.
- Confirm staged install pkg-config metadata reports `Version: 0.1.1`.
- Confirm `docs/packaging.md` matches the staged install layout.
- Confirm `docs/release-notes-v0.1.1.md` is the release body.
- Confirm `docs/release-notes-v0.1.md` remains historical and unchanged.
- Confirm `examples/README.md` and the example scripts are present.
- Confirm examples do not publish releases, create tags, use `sudo`, or send
  CAT write/PTT commands.
- Confirm GitHub Actions CI is green before tagging.
- Confirm T5 PTT control remains deferred and unchecked in `TODO.md`.
- Confirm release publication remains manual and local per
  `docs/release-procedure.md`.

## Optional Linux-host Validation

Run optional profiles only on hosts with the matching development libraries:

```sh
CP_CHECK_SNDFILE=1 ./scripts/release-check.sh
CP_CHECK_PORTAUDIO=1 ./scripts/release-check.sh
CP_CHECK_TUI=1 ./scripts/release-check.sh
CP_CHECK_GUI=1 CP_CHECK_FFTW=1 ./scripts/release-check.sh
CP_CHECK_ALL_OPTIONAL=1 ./scripts/release-check.sh
```

If an optional dependency is missing, the build should fail with a package or
library name. Do not install packages from project scripts.

GitHub Actions covers the base build, staged install, libsndfile, PortAudio,
ncurses TUI, FFTW, flrig read-only CAT, and hamlib read-only CAT profiles when
the runner packages are available. SDL3 GUI validation is attempted in a
separate allowed-to-skip job because Ubuntu runner package availability can
vary. Local GUI validation remains required for release review when SDL3 is not
available in CI.

## Optional CAT Validation

Mock CAT status should work without hardware:

```sh
./carrierpress --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off --cat-status
./carrierpress --cat-backend none --cat-status
```

Optional flrig and hamlib profiles are read-only:

```sh
CP_CHECK_FLRIG=1 ./scripts/release-check.sh
CP_CHECK_HAMLIB=1 ./scripts/release-check.sh
```

No release check may require a real radio. Real rig tests, if performed, must be
manual and read-only for v0.1.

## Optional GUI Validation

Build and test the GUI monitor:

```sh
CP_CHECK_GUI=1 CP_CHECK_FFTW=1 ./scripts/release-check.sh
```

Run the deterministic GUI demo screenshot after a GUI build:

```sh
./carrierpress --gui-demo-screenshot build/gui-demo.bmp --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off
```

Inspect the saved BMP and follow `docs/gui-visual-regression.md`. Small renderer
or font differences are not release blockers unless panels are missing,
unreadable, or incorrect.

## Local Hamlib 4.7.1 Validation

Latest Hamlib is available locally at:

```sh
/home/dgm/vendored/hamlib-4.7.1
```

This path is for local validation only. Do not hardcode it into normal builds,
tests, docs examples that imply portability, or release scripts.

If tooling needs pkg-config, the local tree currently has `hamlib.pc` at the
source root. Use this only for the local shell command:

```sh
HAMLIB_LOCAL=/home/dgm/vendored/hamlib-4.7.1
PKG_CONFIG_PATH="$HAMLIB_LOCAL:$HAMLIB_LOCAL/lib/pkgconfig:$PKG_CONFIG_PATH" \
	make WITH_HAMLIB=1 test
```

The tested local build path uses headers under `include` and libraries under
`src/.libs`:

```sh
HAMLIB_LOCAL=/home/dgm/vendored/hamlib-4.7.1
CPPFLAGS="-I$HAMLIB_LOCAL/include" \
LDFLAGS="-L$HAMLIB_LOCAL/src/.libs" \
LD_LIBRARY_PATH="$HAMLIB_LOCAL/src/.libs:${LD_LIBRARY_PATH:-}" \
	make WITH_HAMLIB=1 test
```

Do not commit generated Hamlib build products or make CarrierPress depend on
the local vendored path.

GitHub Actions must not depend on this local Hamlib path. CI uses distribution
packages only. The local Hamlib 4.7.1 check remains a manual local validation
step.

## Manual Checks Before Tagging

- Confirm `LICENSE` contains the ISC licence and 2026 M6VPN ownership.
- Confirm README and docs do not claim RF generation, regulatory approval, or
  transmitter compliance.
- Confirm T3 PTT control remains deferred and unchecked.
- Confirm `docs/cat-ptt-safety.md` is present and linked.
- Confirm `docs/gui-visual-regression.md` is present for GUI smoke checks.
- Confirm `docs/release-notes-v0.1.md` is ready for the release body.
- Confirm `docs/release-procedure.md` is followed manually for tags and GitHub
  draft releases.
- Confirm `docs/release-assets-v0.1.md` lists only optional local evidence
  assets.
- Confirm GitHub Actions CI is green.
- Run `make install-smoke` before packaging or prefix-install testing.
- Confirm `git status --short` contains only intentional release changes.
- Draft release notes that list implemented audio and host features as v0.1
  foundations, not final broadcast-processing claims.
- Tag only after the required base validation profile passes.

## Known Deferred Work

- PTT control remains deferred behind the T5 safety checklist.
- MP3 playout remains planned.
- OpenBSD/sndio hardware validation is manual unless a stable runner is added.
- STM32H753 and CMSIS-DSP support remain planned.
- RF generation, transmitter control, NRSC certification, C-QUAM, and regulatory
  compliance tooling are not part of v0.1.

## Optional OpenBSD and sndio Validation

Use `docs/openbsd-sndio.md` for the manual checklist.

- Confirm the base OpenBSD build runs `make`, `make test`, and `make validate`.
- Confirm `make WITH_SNDIO=1` builds when sndio headers and libraries are
  available.
- Confirm `make WITH_SNDIO=1 test` passes.
- Run `./examples/sndio-smoke.sh`.
- Run live sndio only on a safe local device or loopback.
- Confirm Linux PortAudio commands and validation remain unchanged.

## Packaging Checks

- Confirm `build/stage/usr/bin/carrierpress` is installed by `make install-smoke`.
- Confirm `build/stage/usr/include/carrierpress/` contains the public headers.
- Confirm `build/stage/usr/lib/libcarrierpress.a` is installed.
- Confirm `build/stage/usr/lib/pkgconfig/carrierpress.pc` reports the release
  version.
- Confirm `build/stage/usr/share/man/man1/carrierpress.1` is installed.
- Confirm the current `build/dist/carrierpress-VERSION.tar.gz` exists after
  `make dist-check`.
- Confirm the current `build/dist/carrierpress-VERSION.tar.gz.sha256` verifies.
- Review `docs/package-maintainer-checklist.md`.
- Review `docs/packaging-debian-notes.md`.
- Review `docs/packaging-openbsd-notes.md`.
- Review `docs/packaging-portable-notes.md`.
- Confirm no real `debian/` packaging directory was added.
- Confirm no OpenBSD `ports/` tree was added.
- Confirm source tarball and checksum generation remain local and manual.
- Confirm no package metadata claims RF generation, transmitter compliance,
  licence compliance, regulatory approval, legal bandwidth, or broadcast
  quality.

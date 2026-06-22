# CarrierPress Release Procedure

This is a manual local procedure for creating CarrierPress GitHub releases. It
uses local `git` and `gh` commands only when the operator runs them.
CarrierPress project scripts must not create tags, push tags, or publish GitHub
releases.

## Required Validation

Validation target scheduling is documented in
`docs/validation-targets.md`. For a local command summary, run:

```sh
make validation-help
```

Run the release profile from a clean working tree:

```sh
git status --short
make release-check
```

Confirm GitHub Actions CI is green for the commit that will be tagged. CI
coverage and optional-profile notes are documented in `docs/ci.md`.

If optional host dependencies are available, run the relevant opt-in profiles:

```sh
CP_CHECK_SNDFILE=1 ./scripts/release-check.sh
CP_CHECK_PORTAUDIO=1 ./scripts/release-check.sh
CP_CHECK_TUI=1 ./scripts/release-check.sh
CP_CHECK_GUI=1 CP_CHECK_FFTW=1 ./scripts/release-check.sh
CP_CHECK_FLRIG=1 ./scripts/release-check.sh
CP_CHECK_HAMLIB=1 ./scripts/release-check.sh
```

Local Hamlib 4.7.1 validation may use `/home/dgm/vendored/hamlib-4.7.1` only
as a shell-local override. Do not commit this path into portable build logic.

```sh
HAMLIB_LOCAL=/home/dgm/vendored/hamlib-4.7.1
CPPFLAGS="-I$HAMLIB_LOCAL/include" \
LDFLAGS="-L$HAMLIB_LOCAL/src/.libs" \
LD_LIBRARY_PATH="$HAMLIB_LOCAL/src/.libs:${LD_LIBRARY_PATH:-}" \
	make WITH_HAMLIB=1 test
```

## v0.4.1 Release

`v0.4.1` is the patch release for T5A through T5E safety-gate scaffolding after
v0.4.0. It packages mock-only transmit-control safety infrastructure and audit
coverage. It does not add hardware PTT, CAT write/control, hamlib or flrig PTT
calls, GUI/TUI transmit controls, or operational transmit support.

Before tagging:

```sh
make validation-help
make clean
make
make test
make -j test
make validate
make quality
make quality-json
make professional-check
make public-header-smoke
make example-libcarrierpress
./build/examples/libcarrierpress-minimal
make transmit-control-safety-audit
make release-check
sh scripts/release-evidence.sh
./carrierpress --version
git status --short
```

Confirm GitHub Actions CI is green for the same commit. Confirm
`./carrierpress --version` prints `CarrierPress 0.4.1`.

Guarded mock validation must be run serially:

```sh
make clean
make WITH_TRANSMIT_CONTROL=1
make WITH_TRANSMIT_CONTROL=1 test
make transmit-control-safety-audit
make transmit-control-mock-test
```

Do not run `make transmit-control-mock-test` concurrently with other make
targets because it runs `make clean` internally. Do not run clean-mutating
targets alongside `make -j test`. Serial reruns are the expected validation for
clean-mutating targets. The serial and non-cleaning target classes are listed
in `docs/validation-targets.md`.

`make dist-check` creates an archive from committed `HEAD`. Commit the intended
R10 release-prep changes before treating the archive as the release source. Do
not move the existing `v0.4.0` tag.

After committing R10, create and verify the local release archive:

```sh
make clean
make dist-check
sha256sum -c build/dist/carrierpress-0.4.1.tar.gz.sha256
tar -xOf build/dist/carrierpress-0.4.1.tar.gz carrierpress-0.4.1/include/cp_version.h | grep '0.4.1'
```

Create and publish the tag only by explicit local operator action:

```sh
git tag -a v0.4.1 -m "CarrierPress v0.4.1"
git push origin v0.4.1
```

Create the GitHub release manually:

```sh
gh release create v0.4.1 \
	--title "CarrierPress v0.4.1" \
	--notes-file docs/release-notes-v0.4.1.md \
	build/dist/carrierpress-0.4.1.tar.gz \
	build/dist/carrierpress-0.4.1.tar.gz.sha256
gh release view v0.4.1
```

No project script creates tags, pushes tags, uploads artifacts, or publishes
GitHub releases.

## v0.4.0 Release

`v0.4.0` is the release for the v0.4 workflow-polish milestones M24 through
M29. Do not rewrite or retag earlier releases.

Before tagging:

```sh
make clean
make
make test
make -j test
make validate
make quality
make quality-json
make professional-check
make public-header-smoke
make example-libcarrierpress
./build/examples/libcarrierpress-minimal
make install-smoke
make install-manifest
make dist-check
make release-check
./carrierpress --version
git status --short
```

Confirm GitHub Actions CI is green for the same commit. Confirm
`./carrierpress --version` prints `CarrierPress 0.4.0`.

`make dist-check` creates an archive from committed `HEAD`. Commit the intended
R9 release-prep changes before treating the archive as the release source.

Create and publish the tag only by explicit local operator action:

```sh
git tag -a v0.4.0 -m "CarrierPress v0.4.0"
git push origin v0.4.0
```

Create the GitHub release manually:

```sh
gh release create v0.4.0 \
	--title "CarrierPress v0.4.0" \
	--notes-file docs/release-notes-v0.4.0.md \
	build/dist/carrierpress-0.4.0.tar.gz \
	build/dist/carrierpress-0.4.0.tar.gz.sha256
gh release view v0.4.0
```

No project script creates tags, pushes tags, uploads artifacts, or publishes
GitHub releases.

## v0.3.0 Release

`v0.3.0` is the release for the v0.3 operator-polish milestones M17 through
M23. Do not rewrite or retag `v0.1.0`, `v0.1.1`, or `v0.2.0`.

Before tagging:

```sh
make clean
make
make test
make -j test
make validate
make quality
make quality-json
make professional-check
make public-header-smoke
make example-libcarrierpress
./build/examples/libcarrierpress-minimal
make install-smoke
make install-manifest
make dist-check
make release-check
./carrierpress --version
git status --short
```

Confirm GitHub Actions CI is green for the same commit. Confirm
`./carrierpress --version` prints `CarrierPress 0.3.0`.

`make dist-check` creates an archive from committed `HEAD`. Commit the intended
R8 release-prep changes before treating the archive as the release source.

Create and publish the tag only by explicit local operator action:

```sh
git tag -a v0.3.0 -m "CarrierPress v0.3.0"
git push origin v0.3.0
```

Create the GitHub release manually:

```sh
gh release create v0.3.0 \
	--title "CarrierPress v0.3.0" \
	--notes-file docs/release-notes-v0.3.0.md \
	build/dist/carrierpress-0.3.0.tar.gz \
	build/dist/carrierpress-0.3.0.tar.gz.sha256
gh release view v0.3.0
```

No project script creates tags, pushes tags, uploads artifacts, or publishes
GitHub releases.

## v0.2.0 Release

`v0.2.0` is the release for the v0.2 audio-workflow milestones M11 through M16.
Do not rewrite or retag `v0.1.0` or `v0.1.1`.

Before tagging:

```sh
make clean
make
make test
make -j test
make validate
make quality
make quality-json
make professional-check
make install-smoke
make install-manifest
make dist-check
make release-check
./carrierpress --version
git status --short
```

Confirm GitHub Actions CI is green for the same commit. Confirm
`./carrierpress --version` prints `CarrierPress 0.2.0`.
Review packaging maintainer notes before tagging:

```sh
sed -n '1,220p' docs/package-maintainer-checklist.md
sed -n '1,220p' docs/packaging-debian-notes.md
sed -n '1,220p' docs/packaging-openbsd-notes.md
sed -n '1,220p' docs/packaging-portable-notes.md
```

Create and publish the tag only by explicit local operator action:

```sh
git tag -a v0.2.0 -m "CarrierPress v0.2.0"
git push origin v0.2.0
```

Create the GitHub release manually:

```sh
gh release create v0.2.0 \
	--title "CarrierPress v0.2.0" \
	--notes-file docs/release-notes-v0.2.0.md
gh release view v0.2.0
```

No project script creates tags, pushes tags, uploads artifacts, or publishes
GitHub releases.

`make dist-check` creates and verifies a local source archive and checksum
under `build/dist/`. It does not create tags, push tags, call GitHub tools, or
publish a release.

The archive is created from committed `HEAD`. Commit the intended release
changes before treating the archive as the release source.

## v0.1.1 Patch Release

`v0.1.1` is the patch release for post-v0.1.0 install, packaging, CI, and
examples work. Do not rewrite or retag `v0.1.0`.

Before tagging:

```sh
make release-check
make install-smoke
make install-manifest
make dist-check
ls -lh build/dist/
git status --short
```

Confirm GitHub Actions CI is green for the same commit. Confirm
`./carrierpress --version` prints `CarrierPress 0.1.1`.

Create and publish the tag only by explicit local operator action:

```sh
git tag -a v0.1.1 -m "CarrierPress v0.1.1"
git push origin v0.1.1
```

Create the GitHub release manually:

```sh
gh release create v0.1.1 \
	--title "CarrierPress v0.1.1" \
	--notes-file docs/release-notes-v0.1.1.md
gh release view v0.1.1
```

No project script creates tags, pushes tags, or publishes GitHub releases.

`make dist-check` creates and verifies a local source archive and checksum
under `build/dist/`. It does not create tags, push tags, call GitHub tools, or
publish a release.

The archive is created from committed `HEAD`. Commit the intended release
changes before treating the archive as the release source.

## Local Release Assets

Create optional local evidence files under `build/release-v0.1.0/`. These files
are useful for review, but they are not required by normal builds.

```sh
mkdir -p build/release-v0.1.0
make release-check 2>&1 | tee build/release-v0.1.0/release-check.log
git rev-parse HEAD > build/release-v0.1.0/source-commit.txt
cp docs/release-notes-v0.1.md build/release-v0.1.0/RELEASE_NOTES.md
```

If the GUI build is available, save a deterministic BMP screenshot:

```sh
make WITH_GUI=1 WITH_FFTW=1
./carrierpress --gui-demo-screenshot build/release-v0.1.0/gui-demo.bmp --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off
```

Generate checksums for local review:

```sh
cd build/release-v0.1.0
sha256sum * > SHA256SUMS
cd ../..
```

Do not commit generated release assets unless a later task explicitly asks for
a tracked fixture or static example.

## Manual Tag

Confirm the final source state:

```sh
git status --short
git log -1 --oneline
```

Create the annotated tag locally:

```sh
git tag -a v0.1.0 -m "CarrierPress v0.1.0"
```

Push the tag only after validation and review:

```sh
git push origin v0.1.0
```

Do not push a tag until `make release-check` has passed locally and GitHub
Actions CI is green for the selected commit.

## Manual GitHub Draft Release

Create a draft release from the local release notes:

```sh
gh release create v0.1.0 --draft --title "CarrierPress v0.1.0" --notes-file docs/release-notes-v0.1.md
```

Optional local evidence assets can be uploaded manually:

```sh
gh release upload v0.1.0 build/release-v0.1.0/release-check.log build/release-v0.1.0/source-commit.txt build/release-v0.1.0/SHA256SUMS --clobber
```

If a GUI screenshot was produced:

```sh
gh release upload v0.1.0 build/release-v0.1.0/gui-demo.bmp --clobber
```

Review the draft release in GitHub before publishing:

```sh
gh release view v0.1.0 --json tagName,isDraft,isPrerelease,name,url
```

Publish only by explicit operator action:

```sh
gh release edit v0.1.0 --draft=false
```

## Final Manual Checks

- Confirm release notes do not claim RF generation, transmitter compliance, or
  regulatory approval.
- Confirm CAT support remains read-only.
- Confirm T5 PTT control remains deferred in `TODO.md`.
- Confirm `docs/cat-ptt-safety.md` remains linked.
- Confirm optional dependencies are presented as optional.
- Confirm generated assets are local review evidence, not required source files.

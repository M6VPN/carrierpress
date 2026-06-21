# CarrierPress v0.1 Release Procedure

This is a manual local procedure for creating a `v0.1.0` GitHub release. It
uses local `git` and `gh` commands only when the operator runs them. CarrierPress
project scripts must not create tags, push tags, or publish GitHub releases.

## Required Validation

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

## v0.1.1 Patch Release

`v0.1.1` is the patch release for post-v0.1.0 install, packaging, CI, and
examples work. Do not rewrite or retag `v0.1.0`.

Before tagging:

```sh
make release-check
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

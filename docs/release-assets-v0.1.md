# CarrierPress v0.1 Example Release Assets

This document lists optional local files that can accompany a GitHub v0.1.0
draft release. These assets are review evidence only. They are not required for
normal builds, tests, or source use.

## Suggested Local Asset Directory

```sh
build/release-v0.1.0/
```

Keep generated files under `build/` so they remain outside source control.

## Suggested Assets

| File                | Purpose                                                                |
| ------------------- | ---------------------------------------------------------------------- |
| `RELEASE_NOTES.md`  | Copy of `docs/release-notes-v0.1.md` used for the draft release body.  |
| `release-check.log` | Output from `make release-check`.                                      |
| `source-commit.txt` | Commit hash validated for the tag.                                     |
| `gui-demo.bmp`      | Optional deterministic GUI screenshot from `--gui-demo-screenshot`.    |
| `SHA256SUMS`        | Checksums for local release evidence files.                            |

## Create Example Assets

```sh
mkdir -p build/release-v0.1.0
make release-check 2>&1 | tee build/release-v0.1.0/release-check.log
git rev-parse HEAD > build/release-v0.1.0/source-commit.txt
cp docs/release-notes-v0.1.md build/release-v0.1.0/RELEASE_NOTES.md
```

If GUI and FFTW support are available:

```sh
make WITH_GUI=1 WITH_FFTW=1
./carrierpress --gui-demo-screenshot build/release-v0.1.0/gui-demo.bmp --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off
```

Create checksums:

```sh
cd build/release-v0.1.0
sha256sum * > SHA256SUMS
cd ../..
```

## Upload Example Assets Manually

After a draft release exists, upload only the local evidence files the operator
wants to publish:

```sh
gh release upload v0.1.0 build/release-v0.1.0/release-check.log build/release-v0.1.0/source-commit.txt build/release-v0.1.0/SHA256SUMS --clobber
```

If a GUI screenshot was created:

```sh
gh release upload v0.1.0 build/release-v0.1.0/gui-demo.bmp --clobber
```

Do not automate this upload in project scripts. Uploading assets must remain an
explicit manual command.

# CarrierPress v0.5.0 Manual Release Checklist

This is a human checklist for final `v0.5.0` release handling. These commands
are examples for the operator. They are not run by project scripts.

Project scripts must not create tags, push tags, publish a GitHub release,
upload artifacts, install packages, use `sudo`, or run hardware transmit
actions.

## Before Tagging

```sh
git status --short
git log --oneline -5
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
make pkg-config-smoke
make packaging-help
make packaging-surface-audit
make transmit-control-safety-audit
make validation-help
make test-matrix-help
make operator-workflow-safety-audit
./examples/enthusiast-quickstart.sh
./examples/operator-safe-demo.sh
./examples/dsp-preset-review.sh
sh scripts/release-evidence.sh
make dist-check
make release-check
```

Record the results in
[`release-evidence-v0.5.0-template.md`](release-evidence-v0.5.0-template.md).

## Guarded Mock Validation

Run this sequence serially. `make transmit-control-mock-test` runs `make clean`
internally.

```sh
make clean
make WITH_TRANSMIT_CONTROL=1
make WITH_TRANSMIT_CONTROL=1 test
make transmit-control-safety-audit
make transmit-control-mock-test
```

Guarded transmit-control remains mock-only. No hardware backend exists.

## Tarball Inspection

```sh
sha256sum -c build/dist/carrierpress-0.5.0.tar.gz.sha256
tar -tzf build/dist/carrierpress-0.5.0.tar.gz | head
tar -xOf build/dist/carrierpress-0.5.0.tar.gz carrierpress-0.5.0/Makefile | grep 'VERSION.*0.5.0'
tar -xOf build/dist/carrierpress-0.5.0.tar.gz carrierpress-0.5.0/docs/release-notes-v0.5.0.md | head
```

## Manual Tag Commands

Do not run these from project scripts.

```sh
git tag -a v0.5.0 -m "CarrierPress v0.5.0"
git push origin v0.5.0
```

## Manual GitHub Release Checklist

- Review [`release-notes-v0.5.0.md`](release-notes-v0.5.0.md).
- Review [`release-evidence-v0.5.0-template.md`](release-evidence-v0.5.0-template.md).
- Verify `build/dist/carrierpress-0.5.0.tar.gz`.
- Verify `build/dist/carrierpress-0.5.0.tar.gz.sha256`.
- Attach artifacts manually if desired.
- Publish manually if desired.
- Confirm no project script performs publication.

## Safety Boundary

- Ordinary builds remain non-transmit.
- `WITH_TRANSMIT_CONTROL=1` remains mock-only.
- Hardware TX backend remains absent.
- CAT write/control remains absent.
- hamlib/flrig PTT calls remain absent.
- serial/GPIO/VOX TX control remains absent.
- Frequency and mode setting remain absent.
- Release evidence is not RF, transmitter, legal, or regulatory proof.

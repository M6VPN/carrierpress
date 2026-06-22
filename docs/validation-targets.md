# CarrierPress Validation Targets

This guide classifies local validation targets after the v0.4.1 safety-gate
work. It is focused on scheduling and mutation boundaries, not release
publication.

Ordinary and optional build profiles are documented in `docs/test-matrix.md`.
For a concise matrix summary, run:

```sh
make test-matrix-help
```

Run clean-mutating targets serially. Do not run them alongside `make -j test`
or any other build or test target.

## Non-cleaning Validation Targets

These targets do not intentionally remove `build/`, `build/stage`, or
`build/dist`. They may compile missing prerequisites or write normal build
outputs when those prerequisites are absent.

- `make test`
- `make -j test`
- `make validate`
- `make quality`
- `make quality-json`
- `make professional-check`
- `make public-core-header-smoke`
- `make public-tooling-header-smoke`
- `make public-compat-header-smoke`
- `make public-header-smoke`
- `make example-libcarrierpress`
- `make pkg-config-smoke`
- `make transmit-control-safety-audit`
- `make operator-workflow-safety-audit`
- `make validation-help`
- `make test-matrix-help`

`make transmit-control-safety-audit` is intended to be mutation-safe. It scans
source paths and prints a pass or failure result. It does not build, clean,
publish, install packages, or access hardware.

`make operator-workflow-safety-audit` is intended to be mutation-safe. It scans
operator-facing source and examples for active transmit controls, package
installation, release publication, and backend control calls. It does not
build, clean, publish, install packages, or access hardware.

## Build-output Targets

These targets write ordinary build outputs but do not intentionally clean or
rewrite release staging directories.

- `make`
- `make WITH_TRANSMIT_CONTROL=1`
- `make WITH_SNDFILE=1`
- `make WITH_SNDFILE=1 WITH_PORTAUDIO=1`
- `make WITH_GUI=1`
- `make WITH_GUI=1 WITH_FFTW=1`
- `make WITH_TUI=1`
- `make WITH_SNDIO=1`
- `make WITH_FLRIG=1`
- `make WITH_HAMLIB=1`
- `make autodetect`

Optional feature builds remain opt-in. Missing optional dependency failures
should name the package or library and must not affect the base build.

## Clean-mutating and Serial-only Targets

Run these targets serially. Do not run them concurrently with other make
targets.

- `make clean`
- `make release-check`
- `make transmit-control-mock-test`
- `make install-smoke`
- `make install-manifest`
- `make dist`
- `make dist-check`

`make clean` removes build artifacts, the CLI binary, the static library, and
legacy test artifacts.

`make release-check` runs `scripts/release-check.sh`, which starts with
`make clean` and then runs the base validation sequence.

`make transmit-control-mock-test` runs `make clean` internally before guarded
mock validation. Run it after other validation, not in parallel with it.

`make install-smoke` rewrites `build/stage`. `make install-manifest` depends on
`install-smoke` and rewrites the same staging tree.

`make dist` and `make dist-check` rewrite `build/dist`. `make dist-check`
archives committed `HEAD`, so commit release-prep changes before treating its
archive as release evidence.

## Recommended Ordinary Validation

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
make transmit-control-safety-audit
make operator-workflow-safety-audit
make release-check
```

`make release-check` is serial-only because it runs `make clean`. Keep it at
the end of a local validation pass unless you intentionally want to reset build
outputs first.

## Guarded Mock Validation

Run guarded mock validation serially:

```sh
make clean
make WITH_TRANSMIT_CONTROL=1
make WITH_TRANSMIT_CONTROL=1 test
make transmit-control-safety-audit
make transmit-control-mock-test
```

Do not run `make transmit-control-mock-test` concurrently with anything else.
It runs `make clean` internally.

## Release Archive Validation

Run archive validation only after committing the intended release-prep tree:

```sh
make clean
make dist-check
sha256sum -c build/dist/carrierpress-*.tar.gz.sha256
```

Do not use `make dist-check` from an uncommitted release-prep tree when the
goal is to validate committed archive contents. `git archive` reads committed
`HEAD`.

## Local Evidence Helper

`scripts/release-evidence.sh` is a non-mutating local reporter. It prints
current evidence paths and recommended commands. It must not create tags, push
tags, publish releases, upload artifacts, use `sudo`, install packages, or
perform transmit or CAT write/control actions.

Use:

```sh
sh scripts/release-evidence.sh
```

For a concise target guide:

```sh
make validation-help
make test-matrix-help
```

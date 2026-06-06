# Validation

CarrierPress uses deterministic validation fixtures to catch DSP regressions before
operator testing. The validation target is not a quality claim. It is a repeatable
gate that checks the current block chain stays finite, bounded, and consistent
across default, second multiband, AM, and SSB modes.

Run:

```sh
make validate
```

Run the measurement-oriented QA report:

```sh
make quality
```

Run the stricter professional validation gate:

```sh
make professional-check
```

The first validation executable generates these synthetic sources:

- silence
- 1 kHz tone
- sweep
- deterministic noise
- clipped sine
- DC offset
- stereo imbalance
- 50 Hz hum plus program tone

Each source is processed through:

- default chain
- multiband, static bass EQ, and second multiband chain
- dehummer, multiband, static bass EQ, and AM shortwave chain
- dehummer, multiband, static bass EQ, and SSB narrow chain

The report checks:

- all output samples are finite
- final output stays under the configured limiter ceiling
- AM negative peaks stay under the AM negative peak limit
- SSB output stays under the SSB peak limit

M8.2 adds a second validation executable for chain-quality invariants. It runs
these fixed profiles:

- default chain
- 50 Hz dehummer
- 60 Hz dehummer
- multiband plus static bass EQ
- second multiband
- AM voice, shortwave, and wide
- SSB speech, narrow, and wide

It processes silence, low-level program, stepped speech-like levels, a
music-like harmonic mix, burst transients, clipped sine, DC offset, 50 Hz hum,
60 Hz hum, high-frequency tone, and stereo imbalance. It checks finite output,
limiter bounds, AM negative peak bounds, SSB peak bounds, silence stability, DC
offset reduction, dehummer reduction, AM/SSB low-pass rejection, stereo
stability, and AGC gain limits.

The unit test suite also covers the M7.3 linear playout resampler for same-rate
copying, upsampling, downsampling, invalid configuration, stereo stability, and
non-finite input rejection.

M7.4 adds unit coverage for live/playout config parity, shared monitor snapshot
fields, and AM/SSB TUI bank locking. These tests verify command routing and
state reporting, not hardware audio behavior.

M8.1 adds unit coverage for static bass EQ shelf behavior, disabled passthrough,
invalid config rejection, stereo stability, and finite output under non-finite
input.

M8.2 adds the chain-quality validation gate. It is intended to catch regressions
before tuning, hardware tests, or new DSP stages are added.

M8.3 adds a deterministic audio QA report. It processes these sources:

- silence
- speech-like stepped levels
- music-like harmonic content
- clipped sine
- DC offset
- 50 Hz hum plus program tone
- 60 Hz hum plus program tone
- burst transients
- high-frequency tone
- stereo imbalance

The report covers default, 50 Hz dehummer, 60 Hz dehummer, declipper,
natural dynamics plus low-level boost, auto EQ analysis, multiband plus bass
EQ, second multiband, AM shortwave, and SSB narrow profiles. Each line
prints:

- input and output RMS
- input and output peak
- output minimum and maximum sample value
- input and output crest factor
- input and output DC offset
- 50 Hz and 60 Hz single-bin hum measurements
- output left and right RMS
- clipping indicator metrics
- low-ceiling and transient indicator metrics
- high-frequency-loss indicator metrics
- analysis source profile and reason flags
- declipper repaired samples, repaired runs, maximum delta, and bypass reason
- natural dynamics gain reduction
- low-level boost gain
- auto EQ source hint, RMS, spectral tilt, and broad tonal weights
- bass EQ recommendation preset, gains, confidence, and validity
- pass or fail status

The M8.3 thresholds are conservative regression checks for finite output,
limiter ceiling, silence stability, DC reduction, hum reduction, AM/SSB
low-pass rejection, and stereo stability. They are not tuning targets and do not
replace listening tests, spectrum checks, or transmitter/load tests.

M9.2 extends `make quality` with calibrated restoration-analysis fixtures for
hard clipping, low-ceiling clipping, transient rejection, AM-limited bandwidth,
and SSB or voice-limited bandwidth. The target fails if obvious clipping is not
flagged, if a short burst is treated as sustained hard clipping, or if the
limited-band source profiles regress.

M9.3 extends `make quality` with declipper-gating fixtures. Hard-clipped and
low-ceiling clipped fixtures must produce repaired samples when the declipper
profile is active. Burst fixtures must remain bypassed. These checks validate
bounded behavior and bypass logic only.

M9.4 adds unit coverage for natural dynamics and low-level boost disabled
passthrough, invalid config rejection, linked stereo gain, finite output under
non-finite input, loud-block reduction, quiet-program boost, and silence gating.
It also extends `make quality` with a dynamics profile so silence stays gated
and stepped speech exercises at least one pre-AGC dynamics stage.

M9.5 adds `make professional-check`. This is a stricter pass/fail target for
the current chain. It runs deterministic fixtures through default, dehummer,
declipper, natural dynamics plus low-level boost, auto EQ analysis, multiband
plus bass EQ, second multiband, AM, and SSB profiles. It fails on non-finite
output, limiter violations, AM/SSB peak-limit failures, silence instability,
weak DC or hum reduction, weak high-pass or low-pass behavior, stereo
instability, AGC gain-limit regressions, restoration-analysis regressions, auto
EQ analysis regressions, bass EQ recommendation regressions,
declipper-gating regressions, and missing dynamics-stage activity.

M10.3 adds an analysis-only auto EQ tap. Unit tests cover disabled operation,
invalid config rejection, silence, bass-heavy, thin, bright, limited-band,
stereo, and non-finite input cases. `make quality` and
`make professional-check` include auto EQ profiles that verify finite metrics
and reject obvious limited-band misclassification as bright.

M10.4 adds bounded bass EQ recommendations derived from auto EQ metrics. Unit
tests cover silence, bass-heavy, thin, dark, bright, bounded output, and
non-finite metrics. `make quality` and `make professional-check` fail if auto
EQ profiles do not produce finite bounded recommendation output.

These fields are diagnostic and regression signals only. They do not prove
clipping, lossy encoding, clean source quality, or restored source quality.

Validation does not prove regulatory compliance, occupied bandwidth compliance,
sample-rate conversion quality, or broadcast-processor quality. Live hardware
tests, off-air monitoring, spectrum measurements, and transmitter-limit checks
are still required before using CarrierPress with any transmitter.

# Validation

CarrierPress uses deterministic validation fixtures to catch DSP regressions before
operator testing. The validation target is not a quality claim. It is a repeatable
gate that checks the current block chain stays finite, bounded, and consistent
across default, AM, and SSB modes.

Run:

```sh
make validate
```

Run the measurement-oriented QA report:

```sh
make quality
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

The report covers default, 50 Hz dehummer, 60 Hz dehummer, multiband plus bass
EQ, AM shortwave, and SSB narrow profiles. Each line prints:

- input and output RMS
- input and output peak
- output minimum and maximum sample value
- input and output crest factor
- input and output DC offset
- 50 Hz and 60 Hz single-bin hum measurements
- output left and right RMS
- clipping indicator metrics
- high-frequency-loss indicator metrics
- pass or fail status

The M8.3 thresholds are conservative regression checks for finite output,
limiter ceiling, silence stability, DC reduction, hum reduction, AM/SSB
low-pass rejection, and stereo stability. They are not tuning targets and do not
replace listening tests, spectrum checks, or transmitter/load tests.

M9.1 extends `make quality` with restoration analysis fields. These fields are
diagnostic only. They do not prove clipping, lossy encoding, or clean source
quality, and they do not apply repair.

Validation does not prove regulatory compliance, occupied bandwidth compliance,
sample-rate conversion quality, or broadcast-processor quality. Live hardware
tests, off-air monitoring, spectrum measurements, and transmitter-limit checks
are still required before using CarrierPress with any transmitter.

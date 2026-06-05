# Validation

CarrierPress uses deterministic validation fixtures to catch DSP regressions before
operator testing. The validation target is not a quality claim. It is a repeatable
gate that checks the current block chain stays finite, bounded, and consistent
across default, AM, and SSB modes.

Run:

```sh
make validate
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

Validation does not prove regulatory compliance, occupied bandwidth compliance,
sample-rate conversion quality, or broadcast-processor quality. Live hardware
tests, off-air monitoring, spectrum measurements, and transmitter-limit checks
are still required before using CarrierPress with any transmitter.

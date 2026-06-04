# Validation

CarrierPress uses deterministic validation fixtures to catch DSP regressions before
operator testing. The validation target is not a quality claim. It is a repeatable
gate that checks the current block chain stays finite, bounded, and consistent
across default, AM, and SSB modes.

Run:

```sh
make validate
```

The current validation executable generates these synthetic sources:

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
- dehummer, multiband, and AM shortwave chain
- dehummer, multiband, and SSB narrow chain

The report checks:

- all output samples are finite
- final output stays under the configured limiter ceiling
- AM negative peaks stay under the AM negative peak limit
- SSB output stays under the SSB peak limit

Validation does not prove regulatory compliance, occupied bandwidth compliance,
or broadcast-processor quality. Live hardware tests, off-air monitoring,
spectrum measurements, and transmitter-limit checks are still required before
using CarrierPress with any transmitter.

# Restoration Research

CarrierPress M9 restoration work starts with detection, not repair. The M9.2
analyzer is a diagnostic tap used to identify source material that may need
later clean-room declipping or delossifier research.

## Placement

The analyzer observes audio after the DC blocker and dehummer, before AGC:

```text
input
DC blocker
dehummer
restoration analysis tap
AGC
multiband compressor 1
bass EQ
AM or SSB output chain
limiter
meter
output
```

This placement measures source-like behavior before gain riding, multiband
compression, output shaping, and limiting change the signal statistics.

## Metrics

The analyzer currently reports:

| Metric                         | Purpose                                      |
| ------------------------------ | -------------------------------------------- |
| clipped sample ratio           | Fraction of samples near the clip threshold  |
| flat run count                 | Runs of repeated near-peak samples           |
| peak repeat count              | Repeated near-peak values                    |
| observed peak                  | Highest absolute sample observed             |
| crest factor                   | Observed peak divided by RMS                 |
| high-frequency ratio           | Simple first-difference activity estimate    |
| clipping confidence            | Conservative hard-clipping suspicion score   |
| low-ceiling clipping confidence | Flattened low-ceiling suspicion score        |
| transient confidence           | Short near-peak event suspicion score        |
| lossy confidence               | High-frequency-loss suspicion score          |
| source profile                 | Wideband or limited-band source hint         |
| reason flags                   | Bit flags for the active diagnostic reasons  |

The metrics are bounded and finite. NaN and infinity input samples are treated
as invalid observations, reported through the `finite` field, and do not produce
NaN output metrics.

M9.2 adds calibrated fixtures for hard clipping, low-ceiling clipping, short
transient bursts, AM-limited bandwidth, and SSB or voice-limited bandwidth. The
fixtures are quality gates for analyzer behavior. They are not listening tests
or proof of source history.

## Current Limits

The analyzer does not modify samples. It does not reconstruct clipped peaks,
replace missing high-frequency content, infer codec history, or undo noise
reduction artifacts.

The high-frequency-loss score is only a simple indicator. A narrowband voice
recording, AM-limited file, or SSB-limited file can naturally have low
high-frequency activity. A high score should prompt review, not automatic
repair.

The clipping score is conservative. Short bursts and transient peaks are
reported separately from sustained hard clipping. Low-ceiling clipping is a
separate suspicion score based on repeated flat behavior and low crest factor,
not proof of damage.

## Deferred Work

Later M9 research may evaluate:

- Clean-room declipping using bounded envelope reconstruction.
- Confidence-gated repair that can be bypassed fully.
- Lossy-source detection that separates bandwidth limits from codec artifacts.
- Measurement fixtures that compare repaired output against known synthetic
  damage.
- Operator controls for enabling repair only when measured evidence supports it.

Any repair stage must stay optional, deterministic, block-based, malloc-free in
process functions, and safe for live callbacks. CarrierPress must not copy
proprietary restoration algorithms, presets, names, text, or implementation
details.

# Restoration Research

CarrierPress M9 restoration work starts with detection and conservative repair
gating. The M9.3 analyzer is a diagnostic tap used to identify source material
that may need clean-room declipping or delossifier research. The optional
declipper is a bounded research prototype for clear hard-clipping and
low-ceiling clipping cases.

## Placement

The analyzer observes audio after the DC blocker and dehummer. The optional
declipper runs after the analyzer and before AGC:

```text
input
DC blocker
dehummer
restoration analysis tap
declipper
AGC
multiband compressor 1
bass EQ
AM or SSB output chain
limiter
meter
output
```

This placement measures source-like behavior before gain riding, multiband
compression, output shaping, and limiting change the signal statistics. It also
keeps conservative repair ahead of AGC, so the gain rider does not amplify
flat-topped damage before the repair gate has a chance to bypass or act.

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

## Declipper Prototype

Enable the declipper with `--declipper`. The CLI also enables analysis because
repair decisions depend on analyzer metrics. The stage is disabled by default.

The declipper currently:

- repairs only short clipped runs in analysis-confident blocks
- handles hard clipping and low-ceiling clipping indicators
- bypasses transient-like and low-confidence blocks
- clamps output to the normal safe ceiling
- reports repaired samples, repaired runs, maximum sample delta, bypass reason,
  and finite-output state

This is a clean-room bounded interpolation prototype. It is intentionally
conservative and is expected to miss many real clipped sources.

## Current Limits

The analyzer does not modify samples by itself. The optional declipper does not
reconstruct all clipped peaks, replace missing high-frequency content, infer
codec history, or undo noise reduction artifacts.

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

- Better clean-room declipping using bounded envelope reconstruction.
- More complete confidence-gated repair that can be bypassed fully.
- Lossy-source detection that separates bandwidth limits from codec artifacts.
- Measurement fixtures that compare repaired output against known synthetic
  damage.
- Operator controls for enabling repair only when measured evidence supports it.

Any repair stage must stay optional, deterministic, block-based, malloc-free in
process functions, and safe for live callbacks. CarrierPress must not copy
proprietary restoration algorithms, presets, names, text, or implementation
details.

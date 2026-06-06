# DSP Chain

The intended final CarrierPress chain is:

```text
input
DC blocker
dehummer
declipper
delossifier
natural dynamics
low-level boost
AGC
multiband compressor 1
auto EQ / bass EQ
immersive bass / true bass
multiband compressor 2
AM or SSB output shaping
final limiter / clipper
output
```

v0.1 implements only the portable skeleton pieces: DC blocker, dehummer, meter, gated AGC, conservative natural dynamics and low-level boost stages, first multiband compressor scaffold, static bass EQ foundation, second multiband polish scaffold, AM output-chain foundation, SSB output-chain foundation, restoration analysis, a conservative declipper research stage, final limiter, block API, synthetic CLI test, optional WAV processing, and optional PortAudio live audio.

## M10.2 Multiband Compressor 2 Behavior

The active M10.2 chain adds an optional second multiband stage after bass EQ:

```text
input
DC blocker
dehummer
restoration analysis tap
declipper
natural dynamics
low-level boost
AGC
multiband compressor 1
bass EQ
multiband compressor 2
AM or SSB output chain
limiter
meter
output
```

The second multiband stage is disabled by default. It reuses the same 2 to 4
band crossover and compressor scaffold as the first multiband stage, but applies
gentler speech and music presets intended for final polish before AM or SSB
output shaping. It is not a final loudness processor and does not add 5 to 9
band support yet.

## M9.4 Natural Dynamics And Low-Level Boost Behavior

The active M9.4 chain adds optional program dynamics stages before AGC:

```text
input
DC blocker
dehummer
restoration analysis tap
declipper
natural dynamics
low-level boost
AGC
multiband compressor 1
bass EQ
AM or SSB output chain
limiter
meter
output
```

Natural dynamics is disabled by default. When enabled, it is a conservative
linked wideband compressor intended to smooth sudden loud blocks before the AGC
gain rider. Low-level boost is also disabled by default. When enabled, it raises
quiet non-silent program material within a bounded gain limit and gates silence
so noise floor and dead air do not climb endlessly.

These stages are first foundations for the final natural dynamics and
low-level boost goals. They are not multiband loudness processing and do not
claim final broadcast-processor behavior.

## M9.3 Declipper Behavior

The active M9.3 chain adds an optional repair stage after analysis:

```text
input
DC blocker
dehummer
restoration analysis tap
declipper
natural dynamics
low-level boost
AGC
multiband compressor 1
bass EQ
AM or SSB output chain
limiter
meter
output
```

The declipper is disabled by default. Enable it with `--declipper`. The CLI
enables analysis automatically because repair is gated by the analyzer metrics.
The stage only attempts conservative bounded repair on blocks with confident
hard-clipping or low-ceiling clipping indicators. It bypasses transient-like
blocks, low-confidence blocks, and non-finite analysis.

M9.3 is not a final restoration processor. It does not repair codecs,
reconstruct all clipped peaks, replace missing detail, or implement the
delossifier.

## M8.1 Bass EQ Behavior

The active M8.1 chain is:

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

The bass EQ stage is disabled by default. When enabled, it applies conservative
low-shelf and high-shelf filters after the first multiband compressor and before
AM or SSB shaping. This gives later AM/SSB presets a simple tone-shaping stage
without adding automatic EQ, subharmonic synthesis, immersive bass, or true bass
processing yet.

## M9.2 Restoration Analysis Behavior

The active M9.2 chain adds an analysis tap:

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

The restoration analyzer is disabled by default. When enabled with `--analyze`,
it observes the signal after DC blocking and dehumming, before AGC gain riding.
It reports clipping indicators, low-ceiling clipping suspicion, transient
rejection, source profile hints, repeated near-peak flat runs, peak repeats,
observed peak, crest factor, and a simple high-frequency activity ratio. It
does not change samples.

This placement keeps source-condition analysis ahead of level riding and
compression. It is a research foundation for later declipper and delossifier
work, not a restoration processor.

## M7 SSB Behavior

The active M7 chain is:

```text
input
DC blocker
dehummer
AGC
multiband compressor 1
bass EQ
SSB output chain
limiter
meter
output
```

The SSB output chain is disabled by default and is mutually exclusive with the
AM output chain. When enabled, it runs before the final limiter so speech
bandwidth limiting, optional phase rotation, and symmetric peak control happen
before the last safety clamp.

M7 is baseband audio-chain processing only. It does not generate USB or LSB RF
signals, key a transmitter, control CAT, or claim occupied-bandwidth compliance.

## M6 AM Behavior

The active M6 chain is:

```text
input
DC blocker
dehummer
AGC
multiband compressor 1
bass EQ
AM output chain
limiter
meter
output
```

The AM output chain is disabled by default. When enabled, it runs before the final limiter so audio bandwidth limiting, phase rotation, and AM-oriented peak control happen before the last safety clamp.

Negative peak control matters because excessive negative modulation can overdrive an AM transmitter into carrier cutoff or distortion. CarrierPress limits negative peaks separately from positive peaks. Positive asymmetry is allowed only through explicit config and remains an experimental audio-chain option, not a regulatory compliance claim.

## M5 Multiband Behavior

The active M5 chain is:

```text
input
DC blocker
dehummer
AGC
multiband compressor 1
limiter
meter
output
```

The first multiband stage is disabled by default. When enabled, it sits after AGC so the band compressors see a more stable input level, then the final limiter catches recombined peaks. M5 supports 2 to 4 bands now. The data structures are sized for a later 2 to 9 band design.

The crossover scaffold uses cascaded 2nd-order low-pass filters and subtractive band creation. This is stable and bounded for the current tests, but it is not a final Linkwitz-Riley implementation. Per-band compression is conservative and exposes per-band RMS, peak, and gain-reduction meters.

## M4 Dehummer Behavior

The dehummer uses cascaded fixed-frequency biquad notch filters for 50 Hz or 60 Hz mains hum and configured harmonics. It is disabled by default. Hum reduction before AGC is important because otherwise the AGC detector can treat steady hum as program energy and ride gain around the interference.

Narrow notches are safer than aggressive wide filtering. The dehummer reduces steady hum tones and harmonics, but it does not remove all noise and is not forensic restoration.

## M3 AGC Behavior

The M3 AGC is still a single-band input gain rider. It does not replace multiband compression or final clipping work planned later.

Current behavior:

- RMS detector with linked mono/stereo gain.
- Fast attack for sudden loud input.
- Slower release for normal recovery.
- Hold time before release after gain reduction.
- Gate threshold that freezes gain movement on low-level input.
- Silence threshold that prevents silence from raising gain to max.
- Linear and dB gain metering plus gate state reporting.

The other stages remain research and milestone work, not placeholder modules.

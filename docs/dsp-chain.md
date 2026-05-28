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
final limiter / clipper
AM or SSB output shaping
output
```

v0.1 implements only the portable skeleton pieces: DC blocker, dehummer, meter, gated AGC, first multiband compressor scaffold, final limiter, block API, synthetic CLI test, optional WAV processing, and optional PortAudio live audio.

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

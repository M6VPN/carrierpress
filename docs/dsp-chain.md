# DSP Chain

The intended final CarrierPress chain is:

```text
input
DC blocker
declipper
delossifier
dehummer
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

v0.1 implements only the portable skeleton pieces: DC blocker, meter, gated AGC, final limiter, block API, synthetic CLI test, optional WAV processing, and optional PortAudio live audio.

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

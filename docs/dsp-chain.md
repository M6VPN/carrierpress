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

v0.1 implements only the portable skeleton pieces: DC blocker, meter, slow AGC, final limiter, block API, and synthetic CLI test. The other stages are research and milestone work, not placeholder modules.

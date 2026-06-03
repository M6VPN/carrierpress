# SSB Processing

CarrierPress M7 adds an optional SSB output-chain foundation. This stage shapes
baseband audio before the final limiter. It is not an RF exciter, USB/LSB
modulator, transmitter controller, CAT interface, or compliance tool.

Users are responsible for complying with licence terms, transmitter limits,
occupied bandwidth limits, and local radio regulations.

## Chain Position

The SSB output chain runs after AGC and the first multiband compressor, then
before the final limiter:

```text
input
DC blocker
dehummer
AGC
multiband compressor 1
SSB output chain
limiter
meter
output
```

AM and SSB output chains are mutually exclusive. They both shape audio before
the final safety limiter, but their peak handling and bandwidth goals differ.

## High-Pass And Low-Pass

The high-pass filter removes very low frequency energy that does not help SSB
speech intelligibility and can waste transmitter headroom.

The low-pass filter limits audio bandwidth before the final limiter. Narrower
settings can improve speech focus. Wider settings are useful for local testing
or already well-controlled source material.

## Phase Rotation

The optional phase rotator uses cascaded all-pass biquad sections. It can make
speech peaks more even before symmetric peak limiting. This is a simple,
deterministic foundation, not a final speech processor.

## Peak Handling

SSB mode uses symmetric peak limiting. It does not allow AM-style positive
asymmetry. The final limiter still runs after SSB processing as a last safety
clamp.

## Presets

The M7 presets are conservative starting points:

| Preset       | Purpose                                      |
| ------------ | -------------------------------------------- |
| `ssb-speech` | General voice intelligibility starting point |
| `ssb-narrow` | Tighter voice bandwidth                      |
| `ssb-wide`   | Wider lab and local testing preset           |
| `ssb-gentle` | Lighter shaping for already processed files  |

## Deferred Work

M7 does not add USB or LSB modulation, RF filtering, VOX, CAT control,
transmitter keying, noise reduction, de-essing, or measured occupied-bandwidth
certification.

# AM Processing

CarrierPress M6 adds an AM-oriented audio output chain for legal transmitters and dummy-load testing. It processes audio only. It is not an RF exciter, RF modulator, transmitter controller, proof-of-performance system, or certified compliance tool.

Users are responsible for complying with radio regulations, transmitter licence limits, occupied bandwidth limits, and local operating rules.

## Chain Position

The AM chain runs after the DC blocker, dehummer, AGC, and first multiband compressor. It runs before the final limiter:

```text
input
DC blocker
dehummer
AGC
multiband compressor 1
AM output chain
limiter
meter
output
```

This position lets the AGC and first multiband stage stabilize program level before AM-specific bandwidth and peak handling. The final limiter remains after AM shaping as a last bounded-output stage.

## High-Pass And Low-Pass

The high-pass filter removes very low frequency energy that can waste modulation headroom and stress later stages. The low-pass filter limits audio bandwidth before the transmitter path.

The presets use conservative bandwidths. `am-shortwave` starts around a 5 kHz audio low-pass. `am-voice` is narrower. `am-wide` is for lab or dummy-load testing, still with peak protection.

## Phase Rotation

The phase rotator uses cascaded all-pass sections. It changes waveform shape without being a tone control. The goal is to reduce extreme waveform asymmetry before peak control, which can make negative peak limiting less intrusive on some speech or program material.

M6 keeps this simple and testable. It is not a final broadcast processor phase-rotator design.

## Positive And Negative Peaks

AM transmitters can be sensitive to negative peaks because excessive negative modulation can approach carrier cutoff and create distortion or unwanted side effects. CarrierPress controls negative peaks separately from positive peaks.

Positive asymmetry is disabled by default. It can be enabled explicitly with `--am-asymmetry FLOAT`. The module allows up to 200 percent positive capability for experiments, but that is not a default and is not a compliance claim.

## Deferred Work

C-QUAM AM stereo is deferred because it needs a separate stereo encoder path and careful compatibility testing.

NRSC-style processing is deferred until it can be measured and tested properly. M6 has bandwidth limiting and conservative presets, but it is not a certified NRSC implementation.

# Professional Validation

`make professional-check` is the M9.5 regression gate for the current
CarrierPress chain. It is stricter than `make quality` because it is intended to
fail on measured regressions instead of printing a broad diagnostic report.

The target is deterministic and uses synthetic fixtures only. It does not use
sound-card hardware, read external media, or require optional PortAudio,
libsndfile, or ncurses support.

## Profiles

The gate covers:

- default chain
- 50 Hz dehummer
- 60 Hz dehummer
- declipper
- natural dynamics plus low-level boost
- auto EQ analysis
- multiband plus bass EQ
- second multiband
- AM safe, shortwave, wide, and voice presets
- SSB speech, narrow, and wide presets

## Fixtures

The fixtures cover:

- silence
- stepped speech-like audio
- harmonic music-like audio
- hard-clipped sine
- low-ceiling clipped sine
- burst transient
- DC offset
- 50 Hz hum
- 60 Hz hum
- low-frequency tone
- high-frequency tone
- AM-limited source hint
- SSB-limited source hint
- stereo imbalance

## Checks

The gate fails on:

- non-finite output
- final limiter ceiling violations
- AM negative peak-limit violations
- SSB peak-limit violations
- silence instability
- AGC gain outside configured limits
- weak DC offset reduction
- weak 50 Hz or 60 Hz hum reduction
- weak AM or SSB high-pass and low-pass behavior
- stereo instability
- restoration analyzer source-profile regressions
- auto EQ analyzer regressions
- declipper repair or bypass gating regressions
- missing natural dynamics or low-level boost activity on stepped speech

## Limits

This target is not a listening test, spectrum analyzer, transmitter test, RF
measurement, or compliance test. Passing it does not prove broadcast-processor
quality, restored-source quality, occupied-bandwidth compliance, or legal
transmitter operation.

Before using CarrierPress with a transmitter, users still need hardware tests,
off-air monitoring, dummy-load testing, spectrum measurements, and checks
against their licence and transmitter limits.

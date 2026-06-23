# CarrierPress DSP Product Presets

CarrierPress DSP presets are starting points for local audio processing,
speech and music cleanup, AM/SSB preparation, monitoring, and offline report
comparison. They are intended for radio and audio enthusiasts who want
repeatable baseband audio-chain settings.

CarrierPress reports and listening notes are engineering and regression
evidence only. They are not RF measurements, transmitter compliance proof,
licence-compliance proof, occupied-bandwidth proof, regulatory certification,
or broadcast-quality proof.

This P36A slice reviews names, comments, docs, examples, and regression
workflow. It does not change DSP algorithms or profile parameter values.

## Profile Families

### `profiles/am-safe.profile`

Use this as the conservative AM starting point when the source is speech or
mixed programme audio and you want a restrained chain before making local
adjustments.

Enabled stages:

- 50 Hz dehummer with four harmonics.
- Speech multiband stage.
- Natural dynamics.
- Restoration analysis.
- `am-safe` shaping.

Listen for clear speech, stable level, low fatigue, and lack of obvious
pumping. Watch report input/output RMS, peak, crest, clip ratio, hum metrics,
and lossy or low-ceiling confidence. Do not use it as proof that a transmitter
or station is configured legally.

### `profiles/am-shortwave.profile`

Use this when you want a denser shortwave-style AM starting point for speech
or programme audio that benefits from stronger polish.

Enabled stages:

- 50 Hz dehummer with four harmonics.
- Two speech multiband stages.
- Speech bass EQ.
- Natural dynamics.
- Low-level boost.
- Restoration analysis.
- `am-shortwave` shaping.

Listen for intelligibility, density, bass control, high-frequency harshness,
and pumping. Watch output peak, crest factor, hum metrics, and restoration
analysis flags. If the result sounds tiring or crowded, compare against
`am-safe`.

### `profiles/ssb-speech.profile`

Use this as the speech-focused SSB audio starting point for local processing
and receive-side listening checks.

Enabled stages:

- 50 Hz dehummer with three harmonics.
- Speech multiband stage.
- Speech bass EQ.
- Natural dynamics.
- Low-level boost.
- Restoration analysis.
- `ssb-speech` shaping.

Listen for intelligibility, sibilance, low-mid thickness, clipping impression,
and fatigue over time. Watch output peak, crest factor, clip ratio, and
high-frequency analysis. This profile does not key a radio or make any
on-air legality claim.

### `profiles/file-cleanup.profile`

Use this for offline WAV cleanup when the goal is light restoration and
level-management before reviewing a processed file.

Enabled stages:

- 50 Hz dehummer with four harmonics.
- Music multiband stage.
- Warm bass EQ.
- Natural dynamics.
- Restoration analysis.
- Conservative declipper.

Listen for reduced hum, natural dynamics, unchanged musical balance, and
absence of obvious declipper artifacts. Watch clip ratio, declipper counters,
crest factor, DC offset, and hum metrics. Do not use it as proof of restoration
quality.

## AM Preset UX

AM presets shape the baseband audio chain. They do not generate RF, set
transmitter bandwidth, control a transmitter, or certify station operation.

- `am-safe` is the conservative baseline. It keeps the low-pass moderate,
  uses a phase rotator, and keeps peak limits even.
- `am-shortwave` is denser and more speech-forward. It uses a slightly higher
  high-pass, more phase rotation, and a lower negative peak limit.
- `am-wide` is the open-sounding AM option for local comparison. It is not a
  bandwidth or compliance claim.
- `am-voice` is the narrow voice-oriented option for speech comparison.

Start with `am-safe`, then compare against `am-shortwave` or `am-voice` only
when the source and listening goal justify the extra density.

## SSB Preset UX

SSB presets shape speech-oriented baseband audio. They do not control a radio,
write CAT commands, or prove on-air suitability.

- `ssb-speech` is the balanced speech starting point.
- `ssb-narrow` is more constrained and denser.
- `ssb-wide` keeps more low and high speech content for local comparison.
- `ssb-gentle` disables the phase rotator and is useful when the source is
  already controlled.

Listen for speech clarity, listener fatigue, low-mid buildup, sibilance,
pumping, and clipping impression. Compare reports and listening notes before
changing defaults.

## Listening and Regression Workflow

Generate deterministic quality evidence:

```sh
mkdir -p build
make -s quality-json > build/quality-report.json
./carrierpress --report-summary build/quality-report.json
./carrierpress --report-compare build/quality-report.json build/quality-report.json
```

Inspect shipped profiles:

```sh
./carrierpress --print-effective-config --profile profiles/am-safe.profile
./carrierpress --print-effective-config --profile profiles/am-shortwave.profile
./carrierpress --print-effective-config --profile profiles/ssb-speech.profile
./carrierpress --print-effective-config --profile profiles/file-cleanup.profile
```

When WAV support is available:

```sh
make WITH_SNDFILE=1
./carrierpress --input input.wav --output output.wav \
	--profile profiles/am-safe.profile --report output.report.json
./carrierpress --report-summary output.report.json
```

Record subjective notes with
[`listening-notes-template.md`](listening-notes-template.md). Keep those notes
next to the JSON reports and command lines used to generate them.

## A/B Comparison Guidance

Use the same source file for both paths. Keep command lines, input level,
sample rate, and listening level consistent. Compare reports first, then listen
at matched levels.

Good A/B notes include:

- Source material and commit/version.
- Exact profile and command line.
- Report path.
- Listening setup.
- Loudness, clarity, bass/low-mid balance, sibilance, pumping, clipping
  impression, and fatigue notes.

Reports and listening notes support engineering review. They do not replace
instrumented RF measurements, legal review, station checks, or transmitter
validation.

## Start Here

- Conservative AM: `profiles/am-safe.profile`
- Shortwave-style AM: `profiles/am-shortwave.profile`
- Speech SSB: `profiles/ssb-speech.profile`
- Offline cleanup: `profiles/file-cleanup.profile`

For resolved settings:

```sh
./carrierpress --print-effective-config --profile profiles/am-safe.profile
```

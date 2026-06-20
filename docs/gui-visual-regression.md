# GUI Visual Regression

CarrierPress uses SDL3 for the optional GUI monitor. The GUI is a host-side
monitor only. It does not alter DSP behaviour, audio samples, CAT state, or
transmitter control.

## Build Matrix

Use these builds when checking the GUI monitor:

```sh
make WITH_GUI=1
make WITH_GUI=1 WITH_FFTW=1
make WITH_GUI=1 WITH_PORTAUDIO=1 WITH_FFTW=1
make WITH_GUI=1 WITH_SNDFILE=1 WITH_PORTAUDIO=1 WITH_FFTW=1
```

The base `make` and `make test` paths do not require SDL3 or FFTW.

## Deterministic Demo

Use the fixed 960x540 demo view for screenshots and manual visual checks:

```sh
./carrierpress --gui-demo --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off
```

To save one deterministic BMP frame without audio hardware:

```sh
./carrierpress --gui-demo-screenshot build/gui-demo.bmp --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off
```

The screenshot command renders one synthetic monitor frame, saves the BMP, and
exits. It requires `WITH_GUI=1`. With `WITH_FFTW=1`, the screenshot includes
the spectrum panel. Without FFTW, the spectrum panel should show the unavailable
state.

## What To Inspect

- The window opens at the expected fixed layout size.
- Transport, mode, meter, status, CAT, chain, waveform, and spectrum panels are
  visible.
- Input peak, input RMS, output peak, and output RMS meters render.
- The processed-output waveform renders in the waveform panel.
- The processed-output spectrum renders when built with `WITH_FFTW=1`.
- Mock, flrig, hamlib, disabled, unavailable, and error CAT states remain
  readable when manually exercised.
- `q`, Escape, and the window close button exit cleanly in interactive demo,
  live, and playout modes.

## What Is Not A Failure

- Small font, raster, antialiasing, or color differences between SDL renderers.
- Platform-specific window decoration or scaling differences.
- Missing live audio hardware when using `--gui-demo` or
  `--gui-demo-screenshot`.
- A missing spectrum trace in a build without `WITH_FFTW=1`.

## Suggested Manual Check

```sh
make clean
make WITH_GUI=1 WITH_FFTW=1
./carrierpress --gui-demo-screenshot build/gui-demo.bmp --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off
./carrierpress --gui-demo --cat-backend mock --cat-frequency-hz 14230000 --cat-mode USB --cat-ptt off
```

Compare the saved BMP against the previous accepted screenshot by visual
inspection. Do not require pixel-perfect comparison in normal tests or CI unless
a later task adds a guarded, platform-specific visual test target.

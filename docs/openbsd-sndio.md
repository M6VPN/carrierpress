# OpenBSD and sndio Validation

CarrierPress keeps Linux with PortAudio as the active live-audio host path.
OpenBSD and sndio support are optional and manually validated unless a stable
OpenBSD runner is added later.

CarrierPress is baseband audio processing software. It does not generate RF,
does not key a transmitter, and does not prove transmitter compliance,
licence compliance, legal bandwidth, regulatory approval, or broadcast quality.

## Scope

- Linux PortAudio live audio and playout remain the normal active host path.
- sndio is optional and must be selected explicitly.
- The base build does not require sndio headers or libraries.
- OpenBSD hardware validation is manual for now.
- GUI output-device restart is PortAudio-first. sndio GUI device workflow
  constraints are documented in
  [`sndio-gui-device-workflow.md`](sndio-gui-device-workflow.md).
- CAT status remains read-only and PTT control is not implemented.

## Build Notes

Use the normal project make workflow. The base build should work without sndio:

```sh
make clean
make
make test
make validate
make quality
make quality-json
make professional-check
```

If sndio development headers and libraries are available, build the optional
sndio backend:

```sh
make clean
make WITH_SNDIO=1
make WITH_SNDIO=1 test
```

Prerequisites are a C compiler, make, and the sndio development headers and
libraries for `WITH_SNDIO=1`. If optional dependency checks fail, install the
missing package manually through the platform package tools. CarrierPress
project scripts do not install packages.

## sndio Device Notes

sndio uses named devices. Use `--device NAME` for sndio selection:

```sh
./carrierpress --live --audio-backend sndio --device default
./carrierpress --live --audio-backend sndio --device snd/0
```

`--input-device N` and `--output-device N` are PortAudio-oriented numeric
options. Do not use them as the primary sndio controls.

GUI output-device switching currently uses the PortAudio numeric-device model.
sndio GUI output-device restart remains deferred until a named-device request
model, restart-result API, and OpenBSD manual validation evidence are added.

## Manual Validation Checklist

Run the base validation first:

- Build with `make`.
- Run `make test`.
- Run `make -j test`.
- Run `make validate`.
- Run `make quality`.
- Run `make quality-json`.
- Run `make professional-check`.
- Run `make release-check`.

Then validate the optional sndio path:

- Build with `make WITH_SNDIO=1`.
- Run `make WITH_SNDIO=1 test`.
- Run `./carrierpress --self-test`.
- Run live mode only on a safe local audio device or loopback.
- Confirm live mode starts and stops cleanly with `Ctrl-C`.
- Confirm meter values update when audio is present.
- Confirm unsupported sample rates or channel counts are rejected clearly.
- Confirm mono and stereo settings behave as expected.
- Confirm TUI display remains readable if `WITH_TUI=1` is also built.
- Confirm Linux PortAudio commands remain documented and unchanged.
- Confirm GUI output-device restart remains documented as PortAudio-first unless
  a future sndio-specific restart path is selected.

Suggested live commands:

```sh
./carrierpress --live --audio-backend sndio --device default
./carrierpress --live --audio-backend sndio --device snd/0 --channels 2 --block-size 256
```

## Playout Notes

Linux WAV playout remains the documented PortAudio path. On OpenBSD, validate
the supported local path manually and record any backend-specific behavior
before treating it as a release gate.

Playlist checking does not require PortAudio or sndio:

```sh
./carrierpress --playlist-check examples/playout-playlist.txt
```

## Known Limitations

- No OpenBSD CI runner is configured.
- No automated sndio hardware validation exists.
- sndio live-audio validation is manual.
- A successful audio test is not transmitter or regulatory compliance proof.
- PTT control and CAT write/control commands remain deferred.

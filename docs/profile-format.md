# CarrierPress Profile Format

CarrierPress profiles are dependency-free text files for repeatable audio-chain
settings. Use `--profile PATH` to load a profile before running self-test, WAV
processing, playout, live mode, TUI, or GUI monitor commands.

Profiles do not control CAT, PTT, rig frequency, rig mode, transmit state, or
station-control state. CarrierPress profiles are audio workflow files only.

## File Rules

- UTF-8 text.
- One `key = value` pair per line.
- Blank lines are allowed.
- Lines beginning with `#` are comments.
- Inline comments are not parsed.
- Unknown keys are errors.
- Duplicate keys are errors.
- Invalid values are errors.
- Parse errors report line numbers.

## Required Keys

| Key    | Values                         |
| ------ | ------------------------------ |
| `name` | Non-empty profile display name |
| `mode` | `neutral`, `am`, `ssb`, `file-cleanup` |

`description` is optional.

## Supported Audio Keys

| Key                    | Values                                      |
| ---------------------- | ------------------------------------------- |
| `dehummer`             | `off`, `on`                                 |
| `hum_frequency`        | `50`, `60`                                  |
| `hum_harmonics`        | `1` to `16`                                 |
| `multiband`            | `off`, `speech`, `music`                    |
| `multiband_bands`      | `2`, `3`, `4`                               |
| `multiband2`           | `off`, `speech`, `music`                    |
| `multiband2_bands`     | `2`, `3`, `4`                               |
| `bass_eq`              | `off`, `warm`, `music`, `speech`            |
| `natural_dynamics`     | `off`, `on`                                 |
| `low_level_boost`      | `off`, `on`                                 |
| `restoration_analysis` | `off`, `on`                                 |
| `declipper`            | `off`, `on`                                 |
| `am_preset`            | `off`, `am-safe`, `am-shortwave`, `am-wide`, `am-voice` |
| `ssb_preset`           | `off`, `ssb-speech`, `ssb-narrow`, `ssb-wide`, `ssb-gentle` |

The `bass_eq = custom` form is not accepted because there is no profile-level
custom bass EQ parameter set yet.

## Mode Rules

- `mode = am` must not enable an SSB preset.
- `mode = ssb` must not enable an AM preset.
- `mode = neutral` and `mode = file-cleanup` must not force AM or SSB.
- `am_preset = off` and `ssb_preset = off` are allowed to make intent explicit.

## Forbidden Keys

Profiles reject station-control and radio-control keys. The forbidden list
includes:

- `ptt`
- `transmit`
- `tx`
- `cat_ptt`
- `frequency`
- `rig_frequency`
- `rig_mode`
- `mode_control`
- `flrig`
- `hamlib`
- `cat_backend`
- `cat_host`
- `cat_port`
- `rig`
- `rig_path`
- `rig_model`
- `radio`
- `station_control`

This keeps profile files separate from CAT readback and from any future T5
PTT-control work.

## Runtime Usage

Load a profile with `--profile PATH`:

```sh
./carrierpress --profile profiles/am-safe.profile --self-test
./carrierpress --profile profiles/ssb-speech.profile --self-test
./carrierpress --profile profiles/file-cleanup.profile --input in.wav --output out.wav
```

Profiles are applied when `--profile` appears in the command line. Defaults are
created first, the profile is applied next, and later command-line options
override profile values.

This means command order matters:

```sh
./carrierpress --profile profiles/am-safe.profile --am-preset am-voice --self-test
./carrierpress --profile profiles/ssb-speech.profile --ssb-preset ssb-narrow --self-test
```

Options before `--profile` may be overwritten by the loaded profile. Only one
profile may be loaded in this M11 slice.

An explicit later `--am` or `--am-preset` selects AM and disables SSB. An
explicit later `--ssb` or `--ssb-preset` selects SSB and disables AM. CarrierPress
still rejects any final configuration that has both AM and SSB enabled.

## Example

```text
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/profiles/am-safe.profile

name = AM Safe
description = Conservative mono-safe AM processing starting point
mode = am
dehummer = on
hum_frequency = 50
hum_harmonics = 4
multiband = speech
multiband_bands = 3
natural_dynamics = on
am_preset = am-safe
ssb_preset = off
```

See the `profiles/` directory for complete example profiles.

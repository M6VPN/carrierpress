# CarrierPress Profile Format

CarrierPress profiles are dependency-free text files for repeatable audio-chain
settings. M11A defines and validates profiles only. Runtime profile loading
through the main CLI is planned for a later M11 slice.

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

The `bass_eq = custom` form is not accepted in M11A because there is no
profile-level custom bass EQ parameter set yet.

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

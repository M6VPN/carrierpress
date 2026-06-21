# CarrierPress Config File Format

CarrierPress config files are dependency-free text files for ordinary host and
workflow defaults. They are separate from profiles: profiles hold audio-chain
settings, while config files hold run preferences such as backend choice,
sample format, meter cadence, UI preference, and a default profile path.

M12A defines and tests the format. Runtime `--config PATH` loading is planned
for M12B.

Config files do not control CAT, PTT, rig frequency, rig mode, transmit state,
flrig, hamlib, radio state, or station-control state.

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

## Supported Keys

| Key                 | Values                                      |
| ------------------- | ------------------------------------------- |
| `profile`           | Non-empty profile path                      |
| `audio_backend`     | `auto`, `jack`, `alsa`, `pulse`, `sndio`, `default` |
| `device`            | Non-empty device name hint                  |
| `input_device`      | `-1` or a non-negative PortAudio index      |
| `output_device`     | `-1` or a non-negative PortAudio index      |
| `sample_rate`       | `8000` to `192000`                          |
| `channels`          | `1`, `2`                                    |
| `block_size`        | `16` to `4096`                              |
| `meter_interval_ms` | `100` to `60000`                            |
| `tui`               | `off`, `on`                                 |
| `gui`               | `off`, `on`                                 |

`device` cannot be combined with `input_device` or `output_device`. Device
indices are syntax-checked only; hardware availability is still checked by the
selected audio backend at runtime.

## Forbidden Keys

Ordinary config files reject station-control, CAT-control, and audio-chain
profile keys. The forbidden list includes:

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
- `am_preset`
- `ssb_preset`
- `dehummer`
- `hum_frequency`
- `hum_harmonics`
- `multiband`
- `multiband_bands`
- `multiband2`
- `multiband2_bands`
- `bass_eq`
- `natural_dynamics`
- `low_level_boost`
- `restoration_analysis`
- `declipper`

This keeps config files limited to host workflow defaults. Audio-chain settings
belong in `profiles/`, and any future PTT control remains deferred to the T5
safety gate.

## Default Search Paths

Runtime loading is deferred to M12B, but the intended search order is:

1. Explicit `--config PATH`.
2. `$CARRIERPRESS_CONFIG`.
3. `$XDG_CONFIG_HOME/carrierpress/config`.
4. `$HOME/.config/carrierpress/config`.

There is no implicit system-wide config path planned for now.

## Command-line Precedence

When runtime loading is added, defaults should be created first, config files
should be applied only when explicitly loaded or found by the documented search
rules, and later command-line options should override config values.

If a config file names a `profile`, that profile should load before later
command-line profile or DSP options. Config files must not override explicit
station-control safety boundaries.

## Examples

See the `configs/` directory:

- `configs/default.conf`
- `configs/live-pulse.conf`
- `configs/gui-demo.conf`
- `configs/playout.conf`

Example:

```text
# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/configs/default.conf

profile = profiles/am-safe.profile
audio_backend = auto
sample_rate = 48000
channels = 2
block_size = 256
meter_interval_ms = 1000
tui = off
gui = off
```

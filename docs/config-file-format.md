# CarrierPress Config File Format

CarrierPress config files are dependency-free text files for ordinary host and
workflow defaults. They are separate from profiles: profiles hold audio-chain
settings, while config files hold run preferences such as backend choice,
sample format, meter cadence, UI preference, and a default profile path.

M12B adds explicit runtime loading with `--config PATH`.

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

## Runtime Usage

Use `--config PATH` to load a config file at that point in the command line:

```sh
./carrierpress --config configs/default.conf --self-test
./carrierpress --config configs/live-pulse.conf --live
./carrierpress --config configs/gui-demo.conf --gui-demo
./carrierpress --config configs/playout.conf --play input.wav
```

Validate a config file without running DSP or opening audio devices:

```sh
./carrierpress --validate-config configs/default.conf
```

If the config contains `profile = PATH`, CarrierPress loads and validates that
profile immediately after applying the config. The profile still uses the
normal profile safety rules and cannot control CAT, PTT, rig frequency, rig
mode, transmit state, or station control.

Only one config file may be loaded in this runtime slice.

## Default Search Paths

Explicit `--config PATH` is the supported runtime path for now. Automatic
default config loading remains planned. The intended future search order is:

1. Explicit `--config PATH`.
2. `$CARRIERPRESS_CONFIG`.
3. `$XDG_CONFIG_HOME/carrierpress/config`.
4. `$HOME/.config/carrierpress/config`.

There is no implicit system-wide config path planned for now.

## Command-line Precedence

Precedence is order-sensitive:

1. CarrierPress creates built-in defaults.
2. `--config PATH` applies host and workflow settings at the point it appears.
3. A profile named by the config loads immediately after the config.
4. Later command-line options override the config and config-loaded profile.

Options before `--config` may be overwritten by the config. For example,
`--config configs/default.conf --sample-rate 44100` uses `44100`, while
`--sample-rate 44100 --config configs/default.conf` may use the config sample
rate.

If a config enables one monitor UI, a later explicit `--gui` or `--tui`
selects the later UI and disables the other one. If both TUI and GUI are
enabled inside one config file, validation fails.

Use `--print-effective-config` to inspect the final settings after defaults,
config files, profiles, and later command-line overrides:

```sh
./carrierpress --config configs/default.conf --print-effective-config
./carrierpress --config configs/default.conf --profile profiles/ssb-speech.profile --print-effective-config
```

Effective config inspection is documented in
[`docs/effective-config.md`](effective-config.md).

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

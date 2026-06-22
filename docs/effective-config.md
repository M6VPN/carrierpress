# CarrierPress Effective Config Inspection

CarrierPress can validate profile/config files and print the final resolved
audio workflow configuration without opening audio devices or running DSP.

These inspection commands are for ordinary host/audio workflow settings only.
They do not display CAT backend settings, PTT state, rig frequency, rig mode,
transmit state, flrig, hamlib, or station-control fields.

## Commands

Validate a profile:

```sh
./carrierpress --validate-profile profiles/am-safe.profile
```

Validate a config file and its referenced profile, if any:

```sh
./carrierpress --validate-config configs/default.conf
```

Print the final effective settings:

```sh
./carrierpress --config configs/default.conf --print-effective-config
./carrierpress --profile profiles/am-safe.profile --print-effective-config
./carrierpress --config configs/default.conf --profile profiles/ssb-speech.profile --print-effective-config
```

## Precedence

Resolution is order-sensitive:

1. CarrierPress creates built-in defaults.
2. `--config PATH` applies host/workflow settings where it appears.
3. A profile named by the config loads immediately after that config.
4. `--profile PATH` applies profile settings where it appears.
5. Later command-line options override earlier config/profile values.

Options before `--config` or `--profile` may be overwritten by that file.
Options after them override them. `--print-effective-config` shows the final
resolved result.

## Output

The output is stable plain text using `key=value` lines:

```text
carrierpress_effective_config=1
version=0.4.0
config_path=configs/default.conf
profile_path=profiles/am-safe.profile
profile_name=AM Safe
profile_mode=am
audio_backend=auto
device=
input_device=-1
output_device=-1
sample_rate=48000
channels=2
block_size=256
meter_interval_ms=1000
tui=off
gui=off
dehummer=on
hum_frequency=50
hum_harmonics=4
multiband=on
multiband_bands=3
multiband_preset=speech
multiband2=off
multiband2_bands=3
multiband2_preset=speech
bass_eq=off
bass_eq_preset=flat
natural_dynamics=on
low_level_boost=off
restoration_analysis=off
declipper=off
am=on
am_preset=am-safe
ssb=off
ssb_preset=ssb-speech
report_path=
```

Only fields that map to existing public config structs are printed. Internal
processor state is not printed here.

The optional TUI and GUI monitors show a compact operator summary with active
config/profile/report/cue context. That display is for live operation. This
command remains the stable text inspection path for scripts and audits.

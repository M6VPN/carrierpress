# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/profiles/vhf-fm-voice.profile

# VHF FM voice playout profile for local voice audio preparation.
# Uses mono voice shaping around 120 Hz to 3000 Hz with gentler assumptions
# than HF SSB. This profile is not a data-mode profile.
name = VHF FM Voice
description = VHF FM voice playout profile
mode = ssb
dehummer = off
multiband = speech
multiband_bands = 3
multiband2 = off
bass_eq = off
natural_dynamics = on
low_level_boost = off
restoration_analysis = off
declipper = off
ssb_preset = vhf-fm-voice

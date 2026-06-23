# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/profiles/ssb-speech.profile

# SSB speech audio-chain starting profile for local speech processing.
# Enables dehummer, speech multiband, speech bass EQ, natural dynamics,
# low-level boost, restoration analysis, and ssb-speech shaping.
# Listen for intelligibility, sibilance, low-mid buildup, clipping impression,
# and fatigue. This shapes baseband audio only and does not key or control a
# transmitter.
name = SSB Speech
description = Speech-focused SSB audio processing starting point
mode = ssb
dehummer = on
hum_frequency = 50
hum_harmonics = 3
multiband = speech
multiband_bands = 3
multiband2 = off
bass_eq = speech
natural_dynamics = on
low_level_boost = on
restoration_analysis = on
declipper = off
am_preset = off
ssb_preset = ssb-speech

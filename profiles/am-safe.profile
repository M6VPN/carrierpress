# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/profiles/am-safe.profile

# Conservative AM audio-chain starting profile for speech or mixed programme
# material. Enables dehummer, speech multiband, natural dynamics, restoration
# analysis, and am-safe shaping.
# Listen for stable level, clear speech, low fatigue, and no obvious pumping.
# Watch report peak/RMS, crest, hum, and clip metrics. This is not transmitter
# compliance proof or regulatory approval.
name = AM Safe
description = Conservative mono-safe AM processing starting point
mode = am
dehummer = on
hum_frequency = 50
hum_harmonics = 4
multiband = speech
multiband_bands = 3
multiband2 = off
bass_eq = off
natural_dynamics = on
low_level_boost = off
restoration_analysis = on
declipper = off
am_preset = am-safe
ssb_preset = off

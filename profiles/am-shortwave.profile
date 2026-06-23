# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/profiles/am-shortwave.profile

# Shortwave-style AM audio-chain starting profile for denser speech-forward
# local processing. Enables dehummer, two speech multiband stages, speech bass
# EQ, natural dynamics, low-level boost, restoration analysis, and
# am-shortwave shaping.
# Listen for intelligibility, density, bass control, and harshness. This is a
# practical starting point, not a compliance certificate.
name = AM Shortwave
description = Conservative shortwave AM processing starting point
mode = am
dehummer = on
hum_frequency = 50
hum_harmonics = 4
multiband = speech
multiband_bands = 3
multiband2 = speech
multiband2_bands = 3
bass_eq = speech
natural_dynamics = on
low_level_boost = on
restoration_analysis = on
declipper = off
am_preset = am-shortwave
ssb_preset = off

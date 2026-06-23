# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/profiles/file-cleanup.profile

# Offline WAV cleanup starting profile for light restoration and level
# management. Enables dehummer, music multiband, warm bass EQ, natural
# dynamics, restoration analysis, and conservative declipper.
# Listen for reduced hum, natural dynamics, and lack of declipper artifacts.
# This does not prove restoration quality or legal transmission bandwidth.
name = File Cleanup
description = Gentle offline cleanup and level-management starting point
mode = file-cleanup
dehummer = on
hum_frequency = 50
hum_harmonics = 4
multiband = music
multiband_bands = 3
multiband2 = off
bass_eq = warm
natural_dynamics = on
low_level_boost = off
restoration_analysis = on
declipper = on
am_preset = off
ssb_preset = off

# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/profiles/hf-ssb-narrow.profile

# Narrow HF SSB bulletin voice profile for weak or noisy paths.
# Uses 200 Hz to 2400 Hz SSB shaping and denser speech processing than the
# default voice profile. Monitor fatigue and distortion before any RF use.
name = HF SSB Narrow
description = Narrow HF SSB bulletin speech profile
mode = ssb
dehummer = off
multiband = speech
multiband_bands = 3
multiband2 = speech
multiband2_bands = 3
bass_eq = speech
natural_dynamics = on
low_level_boost = off
restoration_analysis = off
declipper = off
ssb_preset = hf-ssb-narrow

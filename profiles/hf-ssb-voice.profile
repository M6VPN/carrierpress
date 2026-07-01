# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/profiles/hf-ssb-voice.profile

# HF SSB bulletin voice profile for local speech playout.
# Uses mono speech processing, 150 Hz to 2700 Hz SSB shaping, speech multiband,
# speech EQ, and natural dynamics. Start with radio speech processing off and
# keep rig ALC low to moderate.
name = HF SSB Voice
description = HF SSB bulletin speech profile
mode = ssb
dehummer = off
multiband = speech
multiband_bands = 3
multiband2 = off
bass_eq = speech
natural_dynamics = on
low_level_boost = off
restoration_analysis = off
declipper = off
ssb_preset = hf-ssb-voice

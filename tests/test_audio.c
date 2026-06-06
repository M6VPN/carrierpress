/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_audio.c */

#include <sys/types.h>

#include <stdio.h>

#include "cp_audio.h"
#include "cp_block.h"

static int	test_backend_parse(void);
static int	test_block_config_from_audio(void);
static int	test_device_selection(void);
static int	test_sample_rate_choice(void);
static int	test_validate_config(void);

int
main(void)
{
	if (!test_validate_config())
		return 1;
	if (!test_backend_parse())
		return 1;
	if (!test_block_config_from_audio())
		return 1;
	if (!test_device_selection())
		return 1;
	if (!test_sample_rate_choice())
		return 1;

	return 0;
}

static int
test_backend_parse(void)
{
	enum cp_audio_backend backend;

	if (cp_audio_backend_from_string("jack", &backend) != CP_AUDIO_OK ||
	    backend != CP_AUDIO_BACKEND_JACK)
		return 0;
	if (cp_audio_backend_from_string("bad", &backend) !=
	    CP_AUDIO_ERR_BACKEND)
		return 0;

	return 1;
}

static int
test_block_config_from_audio(void)
{
	struct cp_audio_config audio_config;
	struct cp_block_config block_config;

	cp_audio_default_config(&audio_config);
	audio_config.channels = CP_CHANNELS_MONO;
	audio_config.sample_rate = 44100.0;
	audio_config.dehummer_enabled = 1;
	audio_config.hum_base_frequency = 60.0f;
	audio_config.hum_harmonic_count = 3;
	audio_config.multiband_enabled = 1;
	audio_config.multiband_band_count = 3;
	audio_config.multiband_preset = CP_MULTIBAND_PRESET_MUSIC;
	audio_config.restoration_config.enabled = 1;
	audio_config.restoration_config.clip_threshold = 0.97f;
	audio_config.declipper_config.enabled = 1;
	audio_config.declipper_config.repair_strength = 0.25f;
	audio_config.bass_eq_config.enabled = 1;
	(void)cp_bass_eq_apply_preset(&audio_config.bass_eq_config, "warm");
	cp_am_apply_preset(&audio_config.am_config, "am-shortwave");
	audio_config.am_config.enabled = 1;

	if (cp_block_config_from_audio(&block_config, &audio_config,
	    audio_config.channels,
	    (cp_sample_t)audio_config.sample_rate) != CP_OK) {
		printf("test_audio: block config conversion failed\n");
		return 0;
	}
	if (block_config.channels != CP_CHANNELS_MONO ||
	    block_config.sample_rate != 44100.0f ||
	    !block_config.dehummer_enabled ||
	    block_config.hum_base_frequency != 60.0f ||
	    block_config.hum_harmonic_count != 3 ||
	    !block_config.multiband_enabled ||
	    block_config.multiband_band_count != 3 ||
	    block_config.multiband_preset != CP_MULTIBAND_PRESET_MUSIC ||
	    !block_config.restoration_config.enabled ||
	    block_config.restoration_config.clip_threshold != 0.97f ||
	    block_config.restoration_config.channel_count != CP_CHANNELS_MONO ||
	    !block_config.declipper_config.enabled ||
	    block_config.declipper_config.repair_strength != 0.25f ||
	    block_config.declipper_config.channel_count != CP_CHANNELS_MONO ||
	    !block_config.bass_eq_config.enabled ||
	    block_config.bass_eq_config.preset != CP_BASS_EQ_PRESET_WARM ||
	    block_config.bass_eq_config.channel_count != CP_CHANNELS_MONO ||
	    !block_config.am_config.enabled ||
	    block_config.am_config.channel_count != CP_CHANNELS_MONO ||
	    block_config.ssb_config.enabled) {
		printf("test_audio: block config parity mismatch\n");
		return 0;
	}
	if (cp_block_config_from_audio(&block_config, &audio_config, 3,
	    48000.0f) != CP_ERR_CHANNELS) {
		printf("test_audio: invalid block config channel accepted\n");
		return 0;
	}
	audio_config.ssb_config.enabled = 1;
	if (cp_block_config_from_audio(&block_config, &audio_config,
	    CP_CHANNELS_MONO, 48000.0f) != CP_ERR_RANGE) {
		printf("test_audio: AM and SSB block config accepted\n");
		return 0;
	}

	return 1;
}

static int
test_device_selection(void)
{
	struct cp_audio_device_candidate candidates[] = {
		{ 0, "hw:0,0", "ALSA", 2, 0, 44100.0, 1, 0 },
		{ 1, "system", "JACK", 2, 2, 48000.0, 0, 0 },
		{ 2, "pulse", "ALSA", 32, 32, 44100.0, 0, 0 },
		{ 3, "default", "ALSA", 32, 32, 44100.0, 0, 1 }
	};
	struct cp_audio_config config;
	int device;

	cp_audio_default_config(&config);
	if (cp_audio_select_device_candidate(&config, candidates, 4,
	    &device) != CP_AUDIO_OK || device != 1) {
		printf("test_audio: auto did not prefer JACK\n");
		return 0;
	}

	cp_audio_default_config(&config);
	config.device_name = "default";
	if (cp_audio_select_device_candidate(&config, candidates, 4,
	    &device) != CP_AUDIO_OK || device != 3) {
		printf("test_audio: named device selection failed\n");
		return 0;
	}

	cp_audio_default_config(&config);
	config.backend = CP_AUDIO_BACKEND_PULSE;
	if (cp_audio_select_device_candidate(&config, candidates, 4,
	    &device) != CP_AUDIO_OK || device != 2) {
		printf("test_audio: pulse selection failed\n");
		return 0;
	}

	cp_audio_default_config(&config);
	config.device_name = "pulse";
	config.input_device = 2;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_DEVICE) {
		printf("test_audio: device name conflict accepted\n");
		return 0;
	}

	return 1;
}

static int
test_sample_rate_choice(void)
{
	struct cp_audio_config config;
	double sample_rate;

	cp_audio_default_config(&config);
	if (cp_audio_choose_sample_rate(&config, 44100.0, 48000.0, 1, 0, 0,
	    &sample_rate) != CP_AUDIO_OK || sample_rate != 48000.0)
		return 0;

	cp_audio_default_config(&config);
	if (cp_audio_choose_sample_rate(&config, 44100.0, 48000.0, 0, 1, 0,
	    &sample_rate) != CP_AUDIO_OK || sample_rate != 44100.0) {
		printf("test_audio: implicit rate fallback failed\n");
		return 0;
	}

	cp_audio_default_config(&config);
	config.sample_rate_explicit = 1;
	if (cp_audio_choose_sample_rate(&config, 44100.0, 48000.0, 0, 1, 1,
	    &sample_rate) != CP_AUDIO_ERR_RATE) {
		printf("test_audio: explicit rate fallback accepted\n");
		return 0;
	}

	return 1;
}

static int
test_validate_config(void)
{
	struct cp_audio_config config;

	cp_audio_default_config(&config);
	if (cp_audio_validate_config(&config) != CP_AUDIO_OK) {
		printf("test_audio: default config rejected\n");
		return 0;
	}

	config.sample_rate = CP_AUDIO_MIN_SAMPLE_RATE - 1.0;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_RATE) {
		printf("test_audio: invalid sample rate accepted\n");
		return 0;
	}

	cp_audio_default_config(&config);
	config.channels = 3;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_CHANNEL) {
		printf("test_audio: invalid channel count accepted\n");
		return 0;
	}

	cp_audio_default_config(&config);
	config.block_size = CP_AUDIO_MAX_BLOCK_SIZE + 1;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_BLOCK) {
		printf("test_audio: invalid block size accepted\n");
		return 0;
	}

	cp_audio_default_config(&config);
	config.input_device = -2;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_DEVICE) {
		printf("test_audio: invalid device accepted\n");
		return 0;
	}

	cp_audio_default_config(&config);
	config.meter_interval_ms = CP_AUDIO_MIN_METER_MS - 1;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_METER) {
		printf("test_audio: invalid meter interval accepted\n");
		return 0;
	}

	cp_audio_default_config(&config);
	config.multiband_band_count = CP_MULTIBAND_M5_MAX_BANDS + 1;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_MB) {
		printf("test_audio: invalid multiband band count accepted\n");
		return 0;
	}

	cp_audio_default_config(&config);
	config.bass_eq_config.enabled = 1;
	config.bass_eq_config.low_gain_db = CP_BASS_EQ_MAX_GAIN_DB + 1.0f;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_BASS_EQ) {
		printf("test_audio: invalid bass EQ config accepted\n");
		return 0;
	}

	cp_audio_default_config(&config);
	config.declipper_config.enabled = 1;
	config.declipper_config.repair_strength = 2.0f;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_DECLIPPER) {
		printf("test_audio: invalid declipper config accepted\n");
		return 0;
	}

	cp_audio_default_config(&config);
	config.am_config.enabled = 1;
	config.am_config.lowpass_hz = config.am_config.sample_rate;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_AM) {
		printf("test_audio: invalid AM config accepted\n");
		return 0;
	}

	cp_audio_default_config(&config);
	config.am_config.enabled = 1;
	config.ssb_config.enabled = 1;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_SSB) {
		printf("test_audio: AM and SSB together accepted\n");
		return 0;
	}

	cp_audio_default_config(&config);
	config.ssb_config.enabled = 1;
	config.ssb_config.lowpass_hz = config.ssb_config.sample_rate;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_SSB) {
		printf("test_audio: invalid SSB config accepted\n");
		return 0;
	}

	cp_audio_default_config(&config);
	config.restoration_config.enabled = 1;
	config.restoration_config.clip_threshold = 2.0f;
	if (cp_audio_validate_config(&config) != CP_AUDIO_ERR_RESTORATION) {
		printf("test_audio: invalid analysis config accepted\n");
		return 0;
	}

	return 1;
}

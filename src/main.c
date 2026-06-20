/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/main.c */

#include <sys/types.h>

#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "carrierpress.h"
#include "cp_cat.h"
#include "cp_playout.h"
#include "cp_portaudio.h"
#include "cp_sndio.h"
#include "cp_wav.h"

#define CP_SELF_TEST_BLOCK	128
#define CP_SELF_TEST_FRAMES	4800
#define CP_SELF_TEST_RATE	48000.0f
#define CP_SELF_TEST_TONE	1000.0f
#define CP_SELF_TEST_LEVEL	0.30f
#define CP_TWO_PI		6.28318530717958647692f
#define CP_WAV_BLOCK_FRAMES	512

static volatile sig_atomic_t stop_requested = 0;

static void	handle_signal(int);
static int	parse_am_preset(struct cp_am_config *, const char *);
static int	parse_bass_eq_preset(struct cp_bass_eq_config *,
		    const char *);
static int	parse_double_arg(const char *, double *);
static int	parse_int_arg(const char *, int *);
static int	parse_multiband_preset(const char *,
		    enum cp_multiband_preset *);
static int	parse_ssb_preset(struct cp_ssb_config *, const char *);
static int	parse_size_arg(const char *, size_t *);
static int	parse_uint_arg(const char *, unsigned int *);
static int	parse_uint64_arg(const char *, uint64_t *);
static int	run_cat_status(const struct cp_cat_config *);
static int	run_list_devices(void);
static int	run_live_audio(const struct cp_audio_config *,
		    const struct cp_cat_config *);
static int	run_playout_file(const char *, const struct cp_audio_config *,
		    const struct cp_block_config *, const struct cp_cat_config *,
		    volatile sig_atomic_t *);
static int	run_playout_playlist(const char *, const struct cp_audio_config *,
		    const struct cp_block_config *, const struct cp_cat_config *,
		    volatile sig_atomic_t *);
static int	run_wav_process(const char *, const char *,
		    const struct cp_block_config *);
static int	run_self_test(const struct cp_block_config *);
static void	print_auto_eq_metrics(const struct cp_auto_eq_metrics *);
static void	print_bass_eq_recommendation(
		    const struct cp_auto_eq_metrics *);
static void	print_restoration_metrics(const struct cp_restoration_metrics *);
static void	usage(const char *);

int
main(int argc, char *argv[])
{
	const char *input_path;
	const char *output_path;
	const char *play_path;
	const char *playlist_path;
	struct cp_audio_config audio_config;
	struct cp_block_config block_config;
	struct cp_cat_config cat_config;
	double parsed_double;
	uint64_t parsed_uint64;
	size_t parsed_size;
	int arg;
	int cat_status_mode;
	int channels_explicit;
	int live_mode;
	int list_devices;
	int self_test_mode;

	input_path   = NULL;
	output_path  = NULL;
	play_path    = NULL;
	playlist_path = NULL;
	cat_status_mode = 0;
	channels_explicit = 0;
	live_mode    = 0;
	list_devices = 0;
	self_test_mode = 0;
	cp_audio_default_config(&audio_config);
	cp_block_default_config(&block_config, CP_CHANNELS_MONO);
	cp_cat_default_config(&cat_config);
	block_config.sample_rate = CP_SELF_TEST_RATE;

	for (arg = 1; arg < argc; arg++) {
		if (strcmp(argv[arg], "--self-test") == 0) {
			self_test_mode = 1;
		} else if (strcmp(argv[arg], "--input") == 0 && arg + 1 < argc) {
			input_path = argv[++arg];
		} else if (strcmp(argv[arg], "--output") == 0 &&
		    arg + 1 < argc) {
			output_path = argv[++arg];
		} else if (strcmp(argv[arg], "--play") == 0 &&
		    arg + 1 < argc) {
			play_path = argv[++arg];
		} else if (strcmp(argv[arg], "--playlist") == 0 &&
		    arg + 1 < argc) {
			playlist_path = argv[++arg];
		} else if (strcmp(argv[arg], "--list-devices") == 0) {
			list_devices = 1;
		} else if (strcmp(argv[arg], "--live") == 0) {
			live_mode = 1;
		} else if (strcmp(argv[arg], "--cat-backend") == 0 &&
		    arg + 1 < argc) {
			if (cp_cat_backend_from_string(argv[++arg],
			    &cat_config.backend) != CP_OK) {
				usage(argv[0]);
				return 1;
			}
			cat_config.enabled = cat_config.backend !=
			    CP_CAT_BACKEND_NONE;
		} else if (strcmp(argv[arg], "--cat-frequency-hz") == 0 &&
		    arg + 1 < argc) {
			if (!parse_uint64_arg(argv[++arg], &parsed_uint64)) {
				usage(argv[0]);
				return 1;
			}
			cat_config.mock_frequency_hz = parsed_uint64;
		} else if (strcmp(argv[arg], "--cat-mode") == 0 &&
		    arg + 1 < argc) {
			if (cp_cat_mode_set(cat_config.mock_mode,
			    sizeof(cat_config.mock_mode), argv[++arg]) !=
			    CP_OK) {
				usage(argv[0]);
				return 1;
			}
		} else if (strcmp(argv[arg], "--cat-ptt") == 0 &&
		    arg + 1 < argc) {
			if (cp_cat_ptt_from_string(argv[++arg],
			    &cat_config.mock_ptt) != CP_OK) {
				usage(argv[0]);
				return 1;
			}
		} else if (strcmp(argv[arg], "--cat-status") == 0) {
			cat_status_mode = 1;
		} else if (strcmp(argv[arg], "--analyze") == 0) {
			block_config.restoration_config.enabled = 1;
			audio_config.restoration_config.enabled = 1;
		} else if (strcmp(argv[arg], "--declipper") == 0) {
			block_config.restoration_config.enabled = 1;
			audio_config.restoration_config.enabled = 1;
			block_config.declipper_config.enabled = 1;
			audio_config.declipper_config.enabled = 1;
		} else if (strcmp(argv[arg], "--declipper-strength") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.declipper_config.repair_strength =
			    (cp_sample_t)parsed_double;
			audio_config.declipper_config.repair_strength =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--declipper-max-samples") == 0 &&
		    arg + 1 < argc) {
			if (!parse_size_arg(argv[++arg], &parsed_size)) {
				usage(argv[0]);
				return 1;
			}
			block_config.declipper_config.max_repair_samples =
			    parsed_size;
			audio_config.declipper_config.max_repair_samples =
			    parsed_size;
		} else if (strcmp(argv[arg], "--auto-eq-analyze") == 0) {
			block_config.auto_eq_config.enabled = 1;
			audio_config.auto_eq_config.enabled = 1;
		} else if (strcmp(argv[arg], "--bass-eq-recommend") == 0) {
			block_config.auto_eq_config.enabled = 1;
			audio_config.auto_eq_config.enabled = 1;
		} else if (strcmp(argv[arg], "--natural-dynamics") == 0) {
			block_config.natural_dynamics_config.enabled = 1;
			audio_config.natural_dynamics_config.enabled = 1;
		} else if (strcmp(argv[arg],
		    "--natural-threshold-db") == 0 && arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.natural_dynamics_config.threshold_db =
			    (cp_sample_t)parsed_double;
			audio_config.natural_dynamics_config.threshold_db =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--natural-ratio") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.natural_dynamics_config.ratio =
			    (cp_sample_t)parsed_double;
			audio_config.natural_dynamics_config.ratio =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg],
		    "--natural-max-reduction-db") == 0 && arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.natural_dynamics_config.
			    max_gain_reduction_db = (cp_sample_t)parsed_double;
			audio_config.natural_dynamics_config.
			    max_gain_reduction_db = (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--low-level-boost") == 0) {
			block_config.low_level_boost_config.enabled = 1;
			audio_config.low_level_boost_config.enabled = 1;
		} else if (strcmp(argv[arg],
		    "--low-level-target-rms") == 0 && arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.low_level_boost_config.target_rms =
			    (cp_sample_t)parsed_double;
			audio_config.low_level_boost_config.target_rms =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg],
		    "--low-level-max-boost-db") == 0 && arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.low_level_boost_config.max_boost_db =
			    (cp_sample_t)parsed_double;
			audio_config.low_level_boost_config.max_boost_db =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--low-level-gate-db") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.low_level_boost_config.gate_threshold_db =
			    (cp_sample_t)parsed_double;
			audio_config.low_level_boost_config.gate_threshold_db =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg],
		    "--low-level-silence-db") == 0 && arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.low_level_boost_config.silence_threshold_db =
			    (cp_sample_t)parsed_double;
			audio_config.low_level_boost_config.silence_threshold_db =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--audio-backend") == 0 &&
		    arg + 1 < argc) {
			if (cp_audio_backend_from_string(argv[++arg],
			    &audio_config.backend) != CP_AUDIO_OK) {
				usage(argv[0]);
				return 1;
			}
		} else if (strcmp(argv[arg], "--device") == 0 &&
		    arg + 1 < argc) {
			audio_config.device_name = argv[++arg];
		} else if (strcmp(argv[arg], "--tui") == 0) {
#ifdef CP_WITH_TUI
			audio_config.tui_enabled = 1;
#else
			printf("TUI support not enabled. Rebuild with WITH_TUI=1.\n");
			return 1;
#endif
		} else if (strcmp(argv[arg], "--input-device") == 0 &&
		    arg + 1 < argc) {
			if (!parse_int_arg(argv[++arg], &audio_config.input_device)) {
				usage(argv[0]);
				return 1;
			}
		} else if (strcmp(argv[arg], "--output-device") == 0 &&
		    arg + 1 < argc) {
			if (!parse_int_arg(argv[++arg],
			    &audio_config.output_device)) {
				usage(argv[0]);
				return 1;
			}
		} else if (strcmp(argv[arg], "--sample-rate") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg],
			    &audio_config.sample_rate)) {
				usage(argv[0]);
				return 1;
			}
			audio_config.sample_rate_explicit = 1;
		} else if (strcmp(argv[arg], "--channels") == 0 &&
		    arg + 1 < argc) {
			if (!parse_size_arg(argv[++arg], &audio_config.channels)) {
				usage(argv[0]);
				return 1;
			}
			channels_explicit = 1;
		} else if (strcmp(argv[arg], "--block-size") == 0 &&
		    arg + 1 < argc) {
			if (!parse_size_arg(argv[++arg],
			    &audio_config.block_size)) {
				usage(argv[0]);
				return 1;
			}
		} else if (strcmp(argv[arg], "--meter-interval-ms") == 0 &&
		    arg + 1 < argc) {
			if (!parse_uint_arg(argv[++arg],
			    &audio_config.meter_interval_ms)) {
				usage(argv[0]);
				return 1;
			}
		} else if (strcmp(argv[arg], "--dehummer") == 0) {
			block_config.dehummer_enabled = 1;
			audio_config.dehummer_enabled = 1;
		} else if (strcmp(argv[arg], "--hum-frequency") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.hum_base_frequency = (cp_sample_t)parsed_double;
			audio_config.hum_base_frequency = (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--hum-harmonics") == 0 &&
		    arg + 1 < argc) {
			if (!parse_size_arg(argv[++arg], &parsed_size)) {
				usage(argv[0]);
				return 1;
			}
			block_config.hum_harmonic_count = parsed_size;
			audio_config.hum_harmonic_count = parsed_size;
		} else if (strcmp(argv[arg], "--hum-q") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.hum_q_factor = (cp_sample_t)parsed_double;
			audio_config.hum_q_factor = (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--multiband") == 0) {
			block_config.multiband_enabled = 1;
			audio_config.multiband_enabled = 1;
		} else if (strcmp(argv[arg], "--multiband-bands") == 0 &&
		    arg + 1 < argc) {
			if (!parse_size_arg(argv[++arg], &parsed_size)) {
				usage(argv[0]);
				return 1;
			}
			block_config.multiband_band_count = parsed_size;
			audio_config.multiband_band_count = parsed_size;
		} else if (strcmp(argv[arg], "--multiband-preset") == 0 &&
		    arg + 1 < argc) {
			if (!parse_multiband_preset(argv[++arg],
			    &block_config.multiband_preset)) {
				usage(argv[0]);
				return 1;
			}
			audio_config.multiband_preset =
			    block_config.multiband_preset;
		} else if (strcmp(argv[arg], "--multiband2") == 0) {
			block_config.multiband2_enabled = 1;
			audio_config.multiband2_enabled = 1;
		} else if (strcmp(argv[arg], "--multiband2-bands") == 0 &&
		    arg + 1 < argc) {
			if (!parse_size_arg(argv[++arg], &parsed_size)) {
				usage(argv[0]);
				return 1;
			}
			block_config.multiband2_band_count = parsed_size;
			audio_config.multiband2_band_count = parsed_size;
		} else if (strcmp(argv[arg], "--multiband2-preset") == 0 &&
		    arg + 1 < argc) {
			if (!parse_multiband_preset(argv[++arg],
			    &block_config.multiband2_preset)) {
				usage(argv[0]);
				return 1;
			}
			audio_config.multiband2_preset =
			    block_config.multiband2_preset;
		} else if (strcmp(argv[arg], "--bass-eq") == 0) {
			block_config.bass_eq_config.enabled = 1;
			audio_config.bass_eq_config.enabled = 1;
		} else if (strcmp(argv[arg], "--bass-eq-preset") == 0 &&
		    arg + 1 < argc) {
			if (!parse_bass_eq_preset(&block_config.bass_eq_config,
			    argv[++arg])) {
				usage(argv[0]);
				return 1;
			}
			audio_config.bass_eq_config =
			    block_config.bass_eq_config;
		} else if (strcmp(argv[arg], "--bass-gain-db") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.bass_eq_config.low_gain_db =
			    (cp_sample_t)parsed_double;
			audio_config.bass_eq_config.low_gain_db =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--bass-frequency") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.bass_eq_config.low_shelf_hz =
			    (cp_sample_t)parsed_double;
			audio_config.bass_eq_config.low_shelf_hz =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--presence-gain-db") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.bass_eq_config.high_gain_db =
			    (cp_sample_t)parsed_double;
			audio_config.bass_eq_config.high_gain_db =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--am") == 0) {
			block_config.am_config.enabled = 1;
			audio_config.am_config.enabled = 1;
		} else if (strcmp(argv[arg], "--am-preset") == 0 &&
		    arg + 1 < argc) {
			if (!parse_am_preset(&block_config.am_config,
			    argv[++arg])) {
				usage(argv[0]);
				return 1;
			}
			audio_config.am_config = block_config.am_config;
		} else if (strcmp(argv[arg], "--am-lowpass") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.am_config.lowpass_hz =
			    (cp_sample_t)parsed_double;
			audio_config.am_config.lowpass_hz =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--am-highpass") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.am_config.highpass_hz =
			    (cp_sample_t)parsed_double;
			audio_config.am_config.highpass_hz =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--am-phase-rotator") == 0) {
			block_config.am_config.phase_rotator_enabled = 1;
			audio_config.am_config.phase_rotator_enabled = 1;
		} else if (strcmp(argv[arg], "--am-positive-peak") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.am_config.positive_peak_limit =
			    (cp_sample_t)parsed_double;
			audio_config.am_config.positive_peak_limit =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--am-negative-peak") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.am_config.negative_peak_limit =
			    (cp_sample_t)parsed_double;
			audio_config.am_config.negative_peak_limit =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--am-asymmetry") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.am_config.asymmetry_enabled = 1;
			block_config.am_config.asymmetry_ratio =
			    (cp_sample_t)parsed_double;
			audio_config.am_config.asymmetry_enabled = 1;
			audio_config.am_config.asymmetry_ratio =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--ssb") == 0) {
			block_config.ssb_config.enabled = 1;
			audio_config.ssb_config.enabled = 1;
		} else if (strcmp(argv[arg], "--ssb-preset") == 0 &&
		    arg + 1 < argc) {
			if (!parse_ssb_preset(&block_config.ssb_config,
			    argv[++arg])) {
				usage(argv[0]);
				return 1;
			}
			audio_config.ssb_config = block_config.ssb_config;
		} else if (strcmp(argv[arg], "--ssb-lowpass") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.ssb_config.lowpass_hz =
			    (cp_sample_t)parsed_double;
			audio_config.ssb_config.lowpass_hz =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--ssb-highpass") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.ssb_config.highpass_hz =
			    (cp_sample_t)parsed_double;
			audio_config.ssb_config.highpass_hz =
			    (cp_sample_t)parsed_double;
		} else if (strcmp(argv[arg], "--ssb-phase-rotator") == 0) {
			block_config.ssb_config.phase_rotator_enabled = 1;
			audio_config.ssb_config.phase_rotator_enabled = 1;
		} else if (strcmp(argv[arg], "--ssb-peak") == 0 &&
		    arg + 1 < argc) {
			if (!parse_double_arg(argv[++arg], &parsed_double)) {
				usage(argv[0]);
				return 1;
			}
			block_config.ssb_config.peak_limit =
			    (cp_sample_t)parsed_double;
			audio_config.ssb_config.peak_limit =
			    (cp_sample_t)parsed_double;
		} else {
			usage(argv[0]);
			return 1;
		}
	}

	if (cp_audio_config_set_format(&audio_config, audio_config.channels,
	    audio_config.sample_rate) != CP_AUDIO_OK) {
		usage(argv[0]);
		return 1;
	}
	if (block_config.am_config.enabled && block_config.ssb_config.enabled) {
		usage(argv[0]);
		return 1;
	}

	if (cat_status_mode) {
		if (input_path != NULL || output_path != NULL ||
		    play_path != NULL || playlist_path != NULL ||
		    live_mode || list_devices || self_test_mode) {
			usage(argv[0]);
			return 1;
		}

		return run_cat_status(&cat_config);
	}

	if (self_test_mode) {
		if (input_path != NULL || output_path != NULL ||
		    play_path != NULL || playlist_path != NULL ||
		    live_mode || list_devices) {
			usage(argv[0]);
			return 1;
		}

		return run_self_test(&block_config);
	}

	if (play_path != NULL || playlist_path != NULL) {
		if (input_path != NULL || output_path != NULL || live_mode ||
		    list_devices ||
		    (play_path != NULL && playlist_path != NULL)) {
			usage(argv[0]);
			return 1;
		}
		if (audio_config.input_device != CP_AUDIO_DEFAULT_DEVICE ||
		    channels_explicit) {
			usage(argv[0]);
			return 1;
		}
		if (play_path != NULL)
			return run_playout_file(play_path, &audio_config,
			    &block_config, &cat_config, &stop_requested);

		return run_playout_playlist(playlist_path, &audio_config,
		    &block_config, &cat_config, &stop_requested);
	}

	if (input_path != NULL || output_path != NULL) {
		if (live_mode || list_devices) {
			usage(argv[0]);
			return 1;
		}
		if (input_path == NULL || output_path == NULL) {
			usage(argv[0]);
			return 1;
		}

		return run_wav_process(input_path, output_path, &block_config);
	}

	if (list_devices) {
		if (live_mode) {
			usage(argv[0]);
			return 1;
		}

		return run_list_devices();
	}

	if (live_mode)
		return run_live_audio(&audio_config, &cat_config);

	usage(argv[0]);
	return 1;
}

static void
handle_signal(int signal_number)
{
	(void)signal_number;

	stop_requested = 1;
}

static int
parse_am_preset(struct cp_am_config *config, const char *text)
{
	if (config == NULL || text == NULL)
		return 0;
	if (cp_am_apply_preset(config, text) != CP_OK)
		return 0;

	return 1;
}

static int
parse_bass_eq_preset(struct cp_bass_eq_config *config, const char *text)
{
	if (config == NULL || text == NULL)
		return 0;
	if (cp_bass_eq_apply_preset(config, text) != CP_OK)
		return 0;

	return 1;
}

static int
parse_double_arg(const char *text, double *value)
{
	char *end;
	double parsed;

	if (text == NULL || value == NULL || text[0] == '\0')
		return 0;

	errno  = 0;
	parsed = strtod(text, &end);
	if (errno != 0 || end == text || *end != '\0')
		return 0;
	if (!isfinite(parsed))
		return 0;

	*value = parsed;
	return 1;
}

static int
parse_int_arg(const char *text, int *value)
{
	char *end;
	long parsed;

	if (text == NULL || value == NULL || text[0] == '\0')
		return 0;

	errno  = 0;
	parsed = strtol(text, &end, 10);
	if (errno != 0 || end == text || *end != '\0')
		return 0;
	if (parsed < INT32_MIN || parsed > INT32_MAX)
		return 0;

	*value = (int)parsed;
	return 1;
}

static int
parse_multiband_preset(const char *text, enum cp_multiband_preset *preset)
{
	if (text == NULL || preset == NULL)
		return 0;
	if (strcmp(text, "speech") == 0) {
		*preset = CP_MULTIBAND_PRESET_SPEECH;
		return 1;
	}
	if (strcmp(text, "music") == 0) {
		*preset = CP_MULTIBAND_PRESET_MUSIC;
		return 1;
	}

	return 0;
}

static int
parse_ssb_preset(struct cp_ssb_config *config, const char *text)
{
	if (config == NULL || text == NULL)
		return 0;
	if (cp_ssb_apply_preset(config, text) != CP_OK)
		return 0;

	return 1;
}

static int
parse_size_arg(const char *text, size_t *value)
{
	char *end;
	unsigned long parsed;

	if (text == NULL || value == NULL || text[0] == '\0' ||
	    text[0] == '-')
		return 0;

	errno  = 0;
	parsed = strtoul(text, &end, 10);
	if (errno != 0 || end == text || *end != '\0')
		return 0;
	if (parsed > SIZE_MAX)
		return 0;

	*value = (size_t)parsed;
	return 1;
}

static int
parse_uint_arg(const char *text, unsigned int *value)
{
	char *end;
	unsigned long parsed;

	if (text == NULL || value == NULL || text[0] == '\0' ||
	    text[0] == '-')
		return 0;

	errno  = 0;
	parsed = strtoul(text, &end, 10);
	if (errno != 0 || end == text || *end != '\0')
		return 0;
	if (parsed > UINT32_MAX)
		return 0;

	*value = (unsigned int)parsed;
	return 1;
}

static int
parse_uint64_arg(const char *text, uint64_t *value)
{
	char *end;
	unsigned long long parsed;

	if (text == NULL || value == NULL || text[0] == '\0' ||
	    text[0] == '-')
		return 0;

	errno = 0;
	parsed = strtoull(text, &end, 10);
	if (errno != 0 || end == text || *end != '\0')
		return 0;
	*value = (uint64_t)parsed;
	return 1;
}

static int
run_cat_status(const struct cp_cat_config *config)
{
	struct cp_cat_snapshot snapshot;
	char buffer[128];
	int status;

	status = cp_cat_snapshot_update(config, &snapshot);
	if (status != CP_OK)
		return 1;
	status = cp_cat_snapshot_format(&snapshot, buffer, sizeof(buffer));
	if (status != CP_OK)
		return 1;

	printf("%s\n", buffer);
	return 0;
}

static int
run_list_devices(void)
{
#ifdef CP_WITH_PORTAUDIO
	int status;

	status = cp_portaudio_list_devices();
	if (status != CP_PORTAUDIO_OK) {
		printf("carrierpress: PortAudio failed: %s\n",
		    cp_portaudio_status_string(status));
		return 1;
	}

	return 0;
#else
	printf("PortAudio support not enabled. Rebuild with WITH_PORTAUDIO=1.\n");
	return 1;
#endif
}

static int
run_live_audio(const struct cp_audio_config *config,
	const struct cp_cat_config *cat_config)
{
	int status;

	(void)cat_config;

	status = cp_audio_validate_config(config);
	if (status != CP_AUDIO_OK) {
		printf("carrierpress: invalid live audio config: %s\n",
		    cp_audio_status_string(status));
		return 1;
	}

	stop_requested = 0;
	if (signal(SIGINT, handle_signal) == SIG_ERR ||
	    signal(SIGTERM, handle_signal) == SIG_ERR) {
		printf("carrierpress: could not install signal handler\n");
		return 1;
	}

#ifdef CP_WITH_SNDIO
	if (config->backend == CP_AUDIO_BACKEND_SNDIO) {
		status = cp_sndio_run(config, &stop_requested);
		if (status != CP_SNDIO_OK) {
			printf("carrierpress: sndio failed: %s\n",
			    cp_sndio_status_string(status));
			return 1;
		}

		return 0;
	}
#else
	if (config->backend == CP_AUDIO_BACKEND_SNDIO) {
		printf("sndio support not enabled. Rebuild with WITH_SNDIO=1.\n");
		return 1;
	}
#endif

#ifdef CP_WITH_PORTAUDIO
	status = cp_portaudio_run(config, cat_config, &stop_requested);
	if (status != CP_PORTAUDIO_OK) {
		printf("carrierpress: PortAudio failed: %s\n",
		    cp_portaudio_status_string(status));
		return 1;
	}

	return 0;
#else
#ifdef CP_WITH_SNDIO
	if (config->backend == CP_AUDIO_BACKEND_AUTO ||
	    config->backend == CP_AUDIO_BACKEND_DEFAULT) {
		status = cp_sndio_run(config, &stop_requested);
		if (status != CP_SNDIO_OK) {
			printf("carrierpress: sndio failed: %s\n",
			    cp_sndio_status_string(status));
			return 1;
		}

		return 0;
	}
#endif
	printf("PortAudio support not enabled. Rebuild with WITH_PORTAUDIO=1.\n");
	return 1;
#endif
}

static int
run_playout_file(const char *path, const struct cp_audio_config *audio_config,
	const struct cp_block_config *block_config,
	const struct cp_cat_config *cat_config,
	volatile sig_atomic_t *stop_flag)
{
#ifdef CP_WITH_PLAYOUT
	struct cp_playout_config config;
	int status;

	stop_requested = 0;
	if (signal(SIGINT, handle_signal) == SIG_ERR ||
	    signal(SIGTERM, handle_signal) == SIG_ERR) {
		printf("carrierpress: could not install signal handler\n");
		return 1;
	}

	cp_playout_default_config(&config);
	config.audio_config = *audio_config;
	config.block_config = *block_config;
	config.cat_config = *cat_config;
	config.block_frames = audio_config->block_size;
	config.meter_interval_ms = audio_config->meter_interval_ms;
	config.stop_requested = stop_flag;

	status = cp_playout_run_file(path, &config);
	if (status != CP_PLAYOUT_OK) {
		printf("carrierpress: playout failed: %s\n",
		    cp_playout_status_string(status));
		return 1;
	}

	return 0;
#else
	(void)path;
	(void)audio_config;
	(void)block_config;
	(void)cat_config;
	(void)stop_flag;

	printf("Playout support not enabled. Rebuild with WITH_SNDFILE=1 "
	    "WITH_PORTAUDIO=1.\n");
	return 1;
#endif
}

static int
run_playout_playlist(const char *path,
	const struct cp_audio_config *audio_config,
	const struct cp_block_config *block_config,
	const struct cp_cat_config *cat_config,
	volatile sig_atomic_t *stop_flag)
{
#ifdef CP_WITH_PLAYOUT
	struct cp_playout_config config;
	int status;

	stop_requested = 0;
	if (signal(SIGINT, handle_signal) == SIG_ERR ||
	    signal(SIGTERM, handle_signal) == SIG_ERR) {
		printf("carrierpress: could not install signal handler\n");
		return 1;
	}

	cp_playout_default_config(&config);
	config.audio_config = *audio_config;
	config.block_config = *block_config;
	config.cat_config = *cat_config;
	config.block_frames = audio_config->block_size;
	config.meter_interval_ms = audio_config->meter_interval_ms;
	config.stop_requested = stop_flag;

	status = cp_playout_run_playlist(path, &config);
	if (status != CP_PLAYOUT_OK) {
		printf("carrierpress: playout failed: %s\n",
		    cp_playout_status_string(status));
		return 1;
	}

	return 0;
#else
	(void)path;
	(void)audio_config;
	(void)block_config;
	(void)cat_config;
	(void)stop_flag;

	printf("Playout support not enabled. Rebuild with WITH_SNDFILE=1 "
	    "WITH_PORTAUDIO=1.\n");
	return 1;
#endif
}

static int
run_wav_process(const char *input_path, const char *output_path,
	const struct cp_block_config *config)
{
#ifdef CP_WITH_SNDFILE
	struct cp_wav_report report;
	int status;

	status = cp_wav_process_file_config_full_report(input_path,
	    output_path, CP_WAV_BLOCK_FRAMES, config, &report);
	if (status != CP_WAV_OK) {
		printf("carrierpress: WAV processing failed: %s\n",
		    cp_wav_status_string(status));
		return 1;
	}
	if (config != NULL && config->restoration_config.enabled)
		print_restoration_metrics(&report.restoration_metrics);
	if (config != NULL && config->auto_eq_config.enabled) {
		print_auto_eq_metrics(&report.auto_eq_metrics);
		print_bass_eq_recommendation(&report.auto_eq_metrics);
	}

	return 0;
#else
	(void)input_path;
	(void)output_path;
	(void)config;

	printf("WAV support not enabled. Rebuild with WITH_SNDFILE=1.\n");
	return 1;
#endif
}

static int
run_self_test(const struct cp_block_config *self_config)
{
	cp_sample_t input[CP_SELF_TEST_BLOCK];
	cp_sample_t output[CP_SELF_TEST_BLOCK];
	cp_sample_t scratch[CP_SELF_TEST_BLOCK];
	struct cp_block_config config;
	struct cp_block_processor processor;
	cp_sample_t phase;
	cp_sample_t step;
	size_t block_frames;
	size_t band;
	size_t frame;
	size_t offset;
	int status;

	if (self_config == NULL)
		cp_block_default_config(&config, CP_CHANNELS_MONO);
	else
		config = *self_config;
	config.channels = CP_CHANNELS_MONO;
	config.sample_rate = CP_SELF_TEST_RATE;
	config.declipper_config.channel_count = CP_CHANNELS_MONO;
	config.declipper_config.sample_rate = CP_SELF_TEST_RATE;
	config.auto_eq_config.channel_count = CP_CHANNELS_MONO;
	config.auto_eq_config.sample_rate = CP_SELF_TEST_RATE;
	config.natural_dynamics_config.channel_count = CP_CHANNELS_MONO;
	config.natural_dynamics_config.sample_rate = CP_SELF_TEST_RATE;
	config.low_level_boost_config.channel_count = CP_CHANNELS_MONO;
	config.low_level_boost_config.sample_rate = CP_SELF_TEST_RATE;
	status = cp_block_init(&processor, &config);
	if (status != CP_OK) {
		printf("carrierpress: init failed: %d\n", status);
		return 1;
	}

	phase = 0.0f;
	step  = (CP_TWO_PI * CP_SELF_TEST_TONE) / CP_SELF_TEST_RATE;

	for (offset = 0; offset < CP_SELF_TEST_FRAMES; offset += block_frames) {
		block_frames = CP_SELF_TEST_FRAMES - offset;
		if (block_frames > CP_SELF_TEST_BLOCK)
			block_frames = CP_SELF_TEST_BLOCK;

		for (frame = 0; frame < block_frames; frame++) {
			input[frame] = sinf(phase) * CP_SELF_TEST_LEVEL;
			phase += step;
			if (phase >= CP_TWO_PI)
				phase -= CP_TWO_PI;
		}

		status = cp_block_process(&processor, input, output, scratch,
		    CP_SELF_TEST_BLOCK, block_frames);
		if (status != CP_OK) {
			printf("carrierpress: process failed: %d\n", status);
			return 1;
		}
	}

	printf("CarrierPress self-test\n");
	printf("dehummer=%s hum_frequency=%0.1f hum_harmonics=%zu hum_q=%0.1f\n",
	    config.dehummer_enabled ? "on" : "off",
	    config.hum_base_frequency, config.hum_harmonic_count,
	    config.hum_q_factor);
	printf("multiband=%s bands=%zu preset=%s\n",
	    config.multiband_enabled ? "on" : "off",
	    processor.multiband.band_count,
	    cp_multiband_preset_string(processor.multiband.config.preset));
	for (band = 0; band < processor.multiband.band_count; band++) {
		printf("band%zu_rms=%0.6f band%zu_gr_db=%0.2f\n", band + 1,
		    processor.multiband.band_rms[band], band + 1,
		    processor.multiband.band_gain_reduction_db[band]);
	}
	printf("multiband2=%s bands=%zu preset=%s\n",
	    config.multiband2_enabled ? "on" : "off",
	    processor.multiband2.band_count,
	    cp_multiband_preset_string(processor.multiband2.config.preset));
	for (band = 0; band < processor.multiband2.band_count; band++) {
		printf("band2_%zu_rms=%0.6f band2_%zu_gr_db=%0.2f\n",
		    band + 1, processor.multiband2.band_rms[band], band + 1,
		    processor.multiband2.band_gain_reduction_db[band]);
	}
	printf("bass_eq=%s preset=%s bass_hz=%0.1f bass_gain_db=%0.2f "
	    "presence_hz=%0.1f presence_gain_db=%0.2f\n",
	    processor.bass_eq.config.enabled ? "on" : "off",
	    cp_bass_eq_preset_string(processor.bass_eq.config.preset),
	    processor.bass_eq.config.low_shelf_hz,
	    processor.bass_eq.config.low_gain_db,
	    processor.bass_eq.config.high_shelf_hz,
	    processor.bass_eq.config.high_gain_db);
	printf("natural_dynamics=%s threshold_db=%0.2f ratio=%0.2f "
	    "gain=%0.6f gain_reduction_db=%0.2f rms=%0.6f\n",
	    processor.natural_dynamics.config.enabled ? "on" : "off",
	    processor.natural_dynamics.config.threshold_db,
	    processor.natural_dynamics.config.ratio,
	    processor.natural_dynamics.gain,
	    processor.natural_dynamics.gain_reduction_db,
	    processor.natural_dynamics.last_rms);
	printf("low_level_boost=%s target_rms=%0.6f max_boost_db=%0.2f "
	    "gain=%0.6f gain_db=%0.2f rms=%0.6f state=%s\n",
	    processor.low_level_boost.config.enabled ? "on" : "off",
	    processor.low_level_boost.config.target_rms,
	    processor.low_level_boost.config.max_boost_db,
	    processor.low_level_boost.gain,
	    processor.low_level_boost.gain_db,
	    processor.low_level_boost.last_rms,
	    cp_agc_state_string(processor.low_level_boost.gate_state));
	printf("input_peak=%0.6f input_rms=%0.6f\n",
	    processor.input_meter.peak[0], processor.input_meter.rms[0]);
	printf("am=%s preset=%s highpass=%0.1f lowpass=%0.1f "
	    "phase_rotator=%s positive_peak=%0.2f negative_peak=%0.2f "
	    "asymmetry=%s asymmetry_ratio=%0.2f\n",
	    processor.am.config.enabled ? "on" : "off",
	    processor.am.config.preset_name,
	    processor.am.config.highpass_hz,
	    processor.am.config.lowpass_hz,
	    processor.am.config.phase_rotator_enabled ? "on" : "off",
	    processor.am.config.positive_peak_limit,
	    processor.am.config.negative_peak_limit,
	    processor.am.config.asymmetry_enabled ? "on" : "off",
	    processor.am.config.asymmetry_ratio);
	printf("ssb=%s preset=%s highpass=%0.1f lowpass=%0.1f "
	    "phase_rotator=%s peak=%0.2f\n",
	    processor.ssb.config.enabled ? "on" : "off",
	    processor.ssb.config.preset_name,
	    processor.ssb.config.highpass_hz,
	    processor.ssb.config.lowpass_hz,
	    processor.ssb.config.phase_rotator_enabled ? "on" : "off",
	    processor.ssb.config.peak_limit);
	printf("output_peak=%0.6f output_rms=%0.6f gain=%0.6f "
	    "gain_db=%0.2f agc_state=%s\n",
	    processor.output_meter.peak[0], processor.output_meter.rms[0],
	    processor.agc.gain, processor.agc.gain_db,
	    cp_agc_state_string(processor.agc.gate_state));
	if (processor.restoration.config.enabled)
		print_restoration_metrics(&processor.restoration.metrics);
	if (processor.declipper.config.enabled) {
		printf("declipper=on strength=%0.2f max_samples=%zu "
		    "repaired_samples=%zu repaired_runs=%zu max_delta=%0.6f "
		    "bypass=%s finite=%s\n",
		    processor.declipper.config.repair_strength,
		    processor.declipper.config.max_repair_samples,
		    processor.declipper.metrics.repaired_sample_count,
		    processor.declipper.metrics.repaired_run_count,
		    processor.declipper.metrics.max_repair_delta,
		    cp_declipper_bypass_reason_string(
		    processor.declipper.metrics.bypass_reason),
		    processor.declipper.metrics.finite ? "yes" : "no");
	}
	if (processor.auto_eq.config.enabled) {
		print_auto_eq_metrics(&processor.auto_eq.metrics);
		print_bass_eq_recommendation(&processor.auto_eq.metrics);
	}

	return 0;
}

static void
print_auto_eq_metrics(const struct cp_auto_eq_metrics *metrics)
{
	size_t band;

	if (metrics == NULL)
		return;

	printf("auto_eq=on source=%s total_rms=%0.6f spectral_tilt_db=%0.2f "
	    "low_weight=%0.6f presence_weight=%0.6f high_weight=%0.6f "
	    "finite=%s\n",
	    cp_auto_eq_source_hint_string(metrics->source_hint),
	    metrics->total_rms,
	    metrics->spectral_tilt_db,
	    metrics->low_frequency_weight,
	    metrics->presence_weight,
	    metrics->high_frequency_weight,
	    metrics->finite ? "yes" : "no");
	for (band = 0; band < CP_AUTO_EQ_BAND_COUNT; band++) {
		printf("auto_eq_band%zu_rms=%0.6f "
		    "auto_eq_band%zu_relative_db=%0.2f enabled=%s\n",
		    band + 1,
		    metrics->band_rms[band],
		    band + 1,
		    metrics->band_relative_db[band],
		    metrics->band_enabled[band] ? "yes" : "no");
	}
}

static void
print_bass_eq_recommendation(const struct cp_auto_eq_metrics *metrics)
{
	struct cp_bass_eq_recommendation recommendation;

	if (metrics == NULL)
		return;
	if (cp_bass_eq_recommend(metrics, &recommendation) != CP_OK)
		return;

	printf("bass_eq_recommend=%s preset=%s bass_gain_db=%0.2f "
	    "presence_gain_db=%0.2f output_gain_db=%0.2f "
	    "confidence=%0.6f source=%s\n",
	    recommendation.valid ? "valid" : "invalid",
	    cp_bass_eq_preset_string(recommendation.preset),
	    recommendation.low_gain_db,
	    recommendation.high_gain_db,
	    recommendation.output_gain_db,
	    recommendation.confidence,
	    cp_auto_eq_source_hint_string(recommendation.source_hint));
}

static void
print_restoration_metrics(const struct cp_restoration_metrics *metrics)
{
	if (metrics == NULL)
		return;

	printf("analysis_profile=%s analysis_reason_flags=0x%08x "
	    "analysis_clip_ratio=%0.6f analysis_hf_ratio=%0.6f "
	    "analysis_clip_confidence=%0.6f "
	    "analysis_low_ceiling_confidence=%0.6f "
	    "analysis_transient_confidence=%0.6f "
	    "analysis_lossy_confidence=%0.6f flat_ratio=%0.6f "
	    "peak_repeat_ratio=%0.6f analysis_peak=%0.6f "
	    "analysis_crest=%0.6f flat_runs=%zu peak_repeats=%zu finite=%s\n",
	    cp_restoration_source_profile_string(metrics->source_profile),
	    metrics->reason_flags,
	    metrics->clipped_sample_ratio,
	    metrics->high_frequency_ratio,
	    metrics->clipping_confidence,
	    metrics->low_ceiling_clipping_confidence,
	    metrics->transient_confidence,
	    metrics->lossy_confidence,
	    metrics->flat_run_ratio,
	    metrics->peak_repeat_ratio,
	    metrics->observed_peak,
	    metrics->crest_factor,
	    metrics->flat_run_count,
	    metrics->peak_repeat_count,
	    metrics->finite ? "yes" : "no");
}

static void
usage(const char *program)
{
	printf("usage: %s --self-test\n", program);
	printf("usage: %s --self-test --dehummer --hum-frequency 50 "
	    "--hum-harmonics 4\n", program);
	printf("usage: %s --self-test --analyze --declipper\n", program);
	printf("usage: %s --self-test --auto-eq-analyze\n", program);
	printf("usage: %s --self-test --bass-eq-recommend\n", program);
	printf("usage: %s --self-test --natural-dynamics "
	    "--low-level-boost\n", program);
	printf("usage: %s --self-test --ssb --ssb-preset ssb-speech\n",
	    program);
	printf("usage: %s --input input.wav --output output.wav\n", program);
	printf("usage: %s --play input.wav [--output-device N]\n", program);
	printf("usage: %s --playlist playlist.txt [--output-device N]\n",
	    program);
	printf("usage: %s --cat-backend mock --cat-frequency-hz 14230000 "
	    "--cat-mode USB --cat-ptt off --cat-status\n", program);
	printf("usage: %s --list-devices\n", program);
	printf("usage: %s --live [--input-device N] [--output-device N]\n",
	    program);
	printf("usage: %s --live --audio-backend auto|jack|alsa|pulse|sndio|default\n",
	    program);
	printf("usage: %s --live --device NAME\n", program);
	printf("usage: %s --live --sample-rate 48000 --channels 2 "
	    "--block-size 256\n", program);
	printf("usage: %s --live --meter-interval-ms 1000\n", program);
	printf("usage: %s --live --tui\n", program);
	printf("analysis option: --analyze\n");
	printf("auto EQ analysis option: --auto-eq-analyze "
	    "--bass-eq-recommend\n");
	printf("declipper options: --declipper --declipper-strength FLOAT "
	    "--declipper-max-samples N\n");
	printf("natural dynamics options: --natural-dynamics "
	    "--natural-threshold-db DB --natural-ratio FLOAT "
	    "--natural-max-reduction-db DB\n");
	printf("low-level boost options: --low-level-boost "
	    "--low-level-target-rms FLOAT --low-level-max-boost-db DB "
	    "--low-level-gate-db DB --low-level-silence-db DB\n");
	printf("dehummer options: --dehummer --hum-frequency 50|60 "
	    "--hum-harmonics N --hum-q Q\n");
	printf("multiband options: --multiband --multiband-bands 2|3|4 "
	    "--multiband-preset speech|music\n");
	printf("multiband2 options: --multiband2 --multiband2-bands 2|3|4 "
	    "--multiband2-preset speech|music\n");
	printf("bass EQ options: --bass-eq --bass-eq-preset "
	    "flat|speech|music|warm --bass-gain-db DB "
	    "--bass-frequency HZ --presence-gain-db DB\n");
	printf("AM options: --am --am-preset am-safe|am-shortwave|am-wide|am-voice "
	    "--am-lowpass HZ --am-highpass HZ --am-phase-rotator "
	    "--am-positive-peak FLOAT --am-negative-peak FLOAT "
	    "--am-asymmetry FLOAT\n");
	printf("SSB options: --ssb --ssb-preset "
	    "ssb-speech|ssb-narrow|ssb-wide|ssb-gentle --ssb-lowpass HZ "
	    "--ssb-highpass HZ --ssb-phase-rotator --ssb-peak FLOAT\n");
	printf("CAT options: --cat-backend none|mock|flrig|hamlib "
	    "--cat-frequency-hz N --cat-mode MODE "
	    "--cat-ptt off|on|unknown --cat-status\n");
}

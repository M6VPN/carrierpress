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
#include "cp_portaudio.h"
#include "cp_wav.h"

#define CP_SELF_TEST_BLOCK	128
#define CP_SELF_TEST_FRAMES	4800
#define CP_SELF_TEST_RATE	48000.0f
#define CP_SELF_TEST_TONE	1000.0f
#define CP_SELF_TEST_LEVEL	0.30f
#define CP_TWO_PI		6.28318530717958647692f
#define CP_WAV_BLOCK_FRAMES	512

#ifdef CP_WITH_PORTAUDIO
static volatile sig_atomic_t stop_requested = 0;

static void	handle_signal(int);
#endif
static int	parse_double_arg(const char *, double *);
static int	parse_int_arg(const char *, int *);
static int	parse_size_arg(const char *, size_t *);
static int	parse_uint_arg(const char *, unsigned int *);
static int	run_list_devices(void);
static int	run_live_audio(const struct cp_audio_config *);
static int	run_wav_process(const char *, const char *);
static int	run_self_test(void);
static void	usage(const char *);

int
main(int argc, char *argv[])
{
	const char *input_path;
	const char *output_path;
	struct cp_audio_config audio_config;
	int arg;
	int live_mode;
	int list_devices;

	if (argc == 2 && strcmp(argv[1], "--self-test") == 0)
		return run_self_test();

	input_path   = NULL;
	output_path  = NULL;
	live_mode    = 0;
	list_devices = 0;
	cp_audio_default_config(&audio_config);

	for (arg = 1; arg < argc; arg++) {
		if (strcmp(argv[arg], "--input") == 0 && arg + 1 < argc) {
			input_path = argv[++arg];
		} else if (strcmp(argv[arg], "--output") == 0 &&
		    arg + 1 < argc) {
			output_path = argv[++arg];
		} else if (strcmp(argv[arg], "--list-devices") == 0) {
			list_devices = 1;
		} else if (strcmp(argv[arg], "--live") == 0) {
			live_mode = 1;
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
		} else if (strcmp(argv[arg], "--channels") == 0 &&
		    arg + 1 < argc) {
			if (!parse_size_arg(argv[++arg], &audio_config.channels)) {
				usage(argv[0]);
				return 1;
			}
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
		} else {
			usage(argv[0]);
			return 1;
		}
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

		return run_wav_process(input_path, output_path);
	}

	if (list_devices) {
		if (live_mode) {
			usage(argv[0]);
			return 1;
		}

		return run_list_devices();
	}

	if (live_mode)
		return run_live_audio(&audio_config);

	usage(argv[0]);
	return 1;
}

#ifdef CP_WITH_PORTAUDIO
static void
handle_signal(int signal_number)
{
	(void)signal_number;

	stop_requested = 1;
}
#endif

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
run_live_audio(const struct cp_audio_config *config)
{
#ifdef CP_WITH_PORTAUDIO
	int status;

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

	status = cp_portaudio_run(config, &stop_requested);
	if (status != CP_PORTAUDIO_OK) {
		printf("carrierpress: PortAudio failed: %s\n",
		    cp_portaudio_status_string(status));
		return 1;
	}

	return 0;
#else
	(void)config;

	printf("PortAudio support not enabled. Rebuild with WITH_PORTAUDIO=1.\n");
	return 1;
#endif
}

static int
run_wav_process(const char *input_path, const char *output_path)
{
#ifdef CP_WITH_SNDFILE
	int status;

	status = cp_wav_process_file(input_path, output_path,
	    CP_WAV_BLOCK_FRAMES);
	if (status != CP_WAV_OK) {
		printf("carrierpress: WAV processing failed: %s\n",
		    cp_wav_status_string(status));
		return 1;
	}

	return 0;
#else
	(void)input_path;
	(void)output_path;

	printf("WAV support not enabled. Rebuild with WITH_SNDFILE=1.\n");
	return 1;
#endif
}

static int
run_self_test(void)
{
	cp_sample_t input[CP_SELF_TEST_BLOCK];
	cp_sample_t output[CP_SELF_TEST_BLOCK];
	cp_sample_t scratch[CP_SELF_TEST_BLOCK];
	struct cp_block_config config;
	struct cp_block_processor processor;
	cp_sample_t phase;
	cp_sample_t step;
	size_t block_frames;
	size_t frame;
	size_t offset;
	int status;

	cp_block_default_config(&config, CP_CHANNELS_MONO);
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
	printf("input_peak=%0.6f input_rms=%0.6f\n",
	    processor.input_meter.peak[0], processor.input_meter.rms[0]);
	printf("output_peak=%0.6f output_rms=%0.6f gain=%0.6f\n",
	    processor.output_meter.peak[0], processor.output_meter.rms[0],
	    processor.agc.gain);

	return 0;
}

static void
usage(const char *program)
{
	printf("usage: %s --self-test\n", program);
	printf("usage: %s --input input.wav --output output.wav\n", program);
	printf("usage: %s --list-devices\n", program);
	printf("usage: %s --live [--input-device N] [--output-device N]\n",
	    program);
	printf("usage: %s --live --sample-rate 48000 --channels 2 "
	    "--block-size 256\n", program);
	printf("usage: %s --live --meter-interval-ms 1000\n", program);
}

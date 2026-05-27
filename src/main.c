/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/main.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "carrierpress.h"
#include "cp_wav.h"

#define CP_SELF_TEST_BLOCK	128
#define CP_SELF_TEST_FRAMES	4800
#define CP_SELF_TEST_RATE	48000.0f
#define CP_SELF_TEST_TONE	1000.0f
#define CP_SELF_TEST_LEVEL	0.30f
#define CP_TWO_PI		6.28318530717958647692f
#define CP_WAV_BLOCK_FRAMES	512

static int	run_wav_process(const char *, const char *);
static int	run_self_test(void);
static void	usage(const char *);

int
main(int argc, char *argv[])
{
	const char *input_path;
	const char *output_path;
	int arg;

	if (argc == 2 && strcmp(argv[1], "--self-test") == 0)
		return run_self_test();

	input_path  = NULL;
	output_path = NULL;

	for (arg = 1; arg < argc; arg++) {
		if (strcmp(argv[arg], "--input") == 0 && arg + 1 < argc) {
			input_path = argv[++arg];
		} else if (strcmp(argv[arg], "--output") == 0 &&
		    arg + 1 < argc) {
			output_path = argv[++arg];
		} else {
			usage(argv[0]);
			return 1;
		}
	}

	if (input_path != NULL || output_path != NULL) {
		if (input_path == NULL || output_path == NULL) {
			usage(argv[0]);
			return 1;
		}

		return run_wav_process(input_path, output_path);
	}

	usage(argv[0]);
	return 1;
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
}

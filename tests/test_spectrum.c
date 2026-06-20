/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_spectrum.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "cp_spectrum.h"

#define TEST_RATE	48000.0
#define TEST_TONE_HZ	1000.0f
#define TEST_TWO_PI	6.28318530717958647692f

static int	test_analyzer_tone_peak(void);
static int	test_clear(void);
static int	test_invalid_args(void);
static int	test_silence(void);
static int	test_stereo_capture(void);

int
main(void)
{
	if (!test_clear())
		return 1;
	if (!test_invalid_args())
		return 1;
	if (!test_silence())
		return 1;
	if (!test_stereo_capture())
		return 1;
	if (!test_analyzer_tone_peak())
		return 1;

	return 0;
}

static int
test_analyzer_tone_peak(void)
{
	struct cp_spectrum_analyzer analyzer;
	struct cp_spectrum_input input;
	struct cp_spectrum_snapshot snapshot;
	cp_sample_t samples[CP_SPECTRUM_FFT_SIZE];
	double expected;
	size_t index;
	int status;

	for (index = 0; index < CP_SPECTRUM_FFT_SIZE; index++) {
		samples[index] = 0.7f * sinf(TEST_TWO_PI * TEST_TONE_HZ *
		    (float)index / (float)TEST_RATE);
	}

	status = cp_spectrum_analyzer_init(&analyzer);
	if (status != CP_OK) {
		printf("test_spectrum: analyzer init failed\n");
		return 0;
	}
	status = cp_spectrum_capture_input(&input, samples,
	    CP_SPECTRUM_FFT_SIZE, 1, TEST_RATE);
	if (status != CP_OK) {
		printf("test_spectrum: tone capture failed\n");
		cp_spectrum_analyzer_close(&analyzer);
		return 0;
	}
	status = cp_spectrum_analyze(&analyzer, &input, &snapshot);
	cp_spectrum_analyzer_close(&analyzer);
	if (status != CP_OK || !snapshot.valid) {
		printf("test_spectrum: tone analyze failed\n");
		return 0;
	}

	expected = (double)TEST_TONE_HZ *
	    (double)CP_SPECTRUM_FFT_SIZE / TEST_RATE;
	if (snapshot.peak_bin + 2 < (size_t)expected ||
	    snapshot.peak_bin > (size_t)expected + 2 ||
	    snapshot.peak_magnitude < CP_SPECTRUM_SCALE / 2) {
		printf("test_spectrum: tone peak failed\n");
		return 0;
	}

	return 1;
}

static int
test_clear(void)
{
	struct cp_spectrum_input input;
	struct cp_spectrum_snapshot snapshot;

	input.valid = 1;
	input.frame_count = 4;
	input.values[0] = 123;
	cp_spectrum_input_clear(&input);
	if (input.valid || input.frame_count != 0 || input.values[0] != 0) {
		printf("test_spectrum: input clear failed\n");
		return 0;
	}

	snapshot.valid = 1;
	snapshot.bin_count = 4;
	snapshot.magnitudes[0] = 123;
	cp_spectrum_clear(&snapshot);
	if (snapshot.valid || snapshot.bin_count != 0 ||
	    snapshot.magnitudes[0] != 0) {
		printf("test_spectrum: snapshot clear failed\n");
		return 0;
	}

	return 1;
}

static int
test_invalid_args(void)
{
	struct cp_spectrum_analyzer analyzer;
	struct cp_spectrum_input input;
	struct cp_spectrum_snapshot snapshot;
	cp_sample_t sample;

	sample = 0.0f;
	if (cp_spectrum_capture_input(NULL, &sample, 1, 1, TEST_RATE) !=
	    CP_ERR_NULL)
		return 0;
	if (cp_spectrum_capture_input(&input, NULL, 1, 1, TEST_RATE) !=
	    CP_ERR_NULL)
		return 0;
	if (cp_spectrum_capture_input(&input, &sample, 0, 1, TEST_RATE) !=
	    CP_ERR_RANGE)
		return 0;
	if (cp_spectrum_capture_input(&input, &sample, 1, 0, TEST_RATE) !=
	    CP_ERR_CHANNELS)
		return 0;
	if (cp_spectrum_analyze(NULL, &input, &snapshot) != CP_ERR_NULL)
		return 0;
	memset(&analyzer, 0, sizeof(analyzer));
	if (cp_spectrum_analyze(&analyzer, &input, &snapshot) !=
	    CP_ERR_RANGE)
		return 0;

	return 1;
}

static int
test_silence(void)
{
	struct cp_spectrum_analyzer analyzer;
	struct cp_spectrum_input input;
	struct cp_spectrum_snapshot snapshot;
	cp_sample_t samples[CP_SPECTRUM_FFT_SIZE];
	size_t index;
	int status;

	for (index = 0; index < CP_SPECTRUM_FFT_SIZE; index++)
		samples[index] = 0.0f;

	status = cp_spectrum_analyzer_init(&analyzer);
	if (status != CP_OK)
		return 0;
	status = cp_spectrum_capture_input(&input, samples,
	    CP_SPECTRUM_FFT_SIZE, 1, TEST_RATE);
	if (status == CP_OK)
		status = cp_spectrum_analyze(&analyzer, &input, &snapshot);
	cp_spectrum_analyzer_close(&analyzer);
	if (status != CP_OK || !snapshot.valid)
		return 0;
	for (index = 0; index < snapshot.bin_count; index++) {
		if (snapshot.magnitudes[index] != 0) {
			printf("test_spectrum: silence bins failed\n");
			return 0;
		}
	}

	return 1;
}

static int
test_stereo_capture(void)
{
	struct cp_spectrum_input input;
	cp_sample_t samples[8];

	samples[0] = 0.5f;
	samples[1] = -0.5f;
	samples[2] = 2.0f;
	samples[3] = 2.0f;
	samples[4] = NAN;
	samples[5] = 0.5f;
	samples[6] = -0.25f;
	samples[7] = -0.75f;

	if (cp_spectrum_capture_input(&input, samples, 4, 2, TEST_RATE) !=
	    CP_OK) {
		printf("test_spectrum: stereo capture failed\n");
		return 0;
	}
	if (!input.valid || input.frame_count != 4 ||
	    input.channel_count != 2 ||
	    input.values[0] != 0 ||
	    input.values[1] != CP_SPECTRUM_SCALE ||
	    input.values[2] != 2500 ||
	    input.values[3] != -5000) {
		printf("test_spectrum: stereo values failed\n");
		return 0;
	}

	return 1;
}

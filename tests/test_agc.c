/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_agc.c */

#include <sys/types.h>

#include <stdio.h>

#include "cp_agc.h"
#include "cp_meter.h"

#define TEST_FRAMES	64
#define TEST_INPUT_RMS	0.05f
#define TEST_TARGET_RMS	0.20f
#define TEST_MAX_GAIN	4.0f

int
main(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_agc agc;
	struct cp_meter meter;
	size_t frame;
	int status;

	for (frame = 0; frame < TEST_FRAMES; frame++)
		input[frame] = TEST_INPUT_RMS;

	status = cp_agc_init(&agc, CP_CHANNELS_MONO, TEST_TARGET_RMS,
	    TEST_MAX_GAIN, 1.0f, 1.0f, 1.0f);
	if (status != CP_OK) {
		printf("test_agc: init failed: %d\n", status);
		return 1;
	}

	status = cp_agc_process(&agc, input, output, TEST_FRAMES);
	if (status != CP_OK) {
		printf("test_agc: process failed: %d\n", status);
		return 1;
	}

	status = cp_meter_init(&meter, CP_CHANNELS_MONO);
	if (status != CP_OK) {
		printf("test_agc: meter init failed: %d\n", status);
		return 1;
	}

	status = cp_meter_process(&meter, output, TEST_FRAMES);
	if (status != CP_OK) {
		printf("test_agc: meter process failed: %d\n", status);
		return 1;
	}

	if (agc.gain <= 1.0f || agc.gain > TEST_MAX_GAIN) {
		printf("test_agc: gain out of range: %f\n", agc.gain);
		return 1;
	}

	if (meter.rms[0] <= TEST_INPUT_RMS) {
		printf("test_agc: rms did not increase: %f\n", meter.rms[0]);
		return 1;
	}

	if (meter.rms[0] > TEST_TARGET_RMS + 0.001f) {
		printf("test_agc: rms exceeded target: %f\n", meter.rms[0]);
		return 1;
	}

	return 0;
}

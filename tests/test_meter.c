/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_meter.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_meter.h"

#define TEST_EPSILON	0.0001f

static int	close_enough(cp_sample_t, cp_sample_t);

int
main(void)
{
	const cp_sample_t input[] = { 1.0f, -1.0f, 0.5f, -0.5f };
	struct cp_meter meter;
	cp_sample_t expected_rms;
	int status;

	status = cp_meter_init(&meter, CP_CHANNELS_MONO);
	if (status != CP_OK) {
		printf("test_meter: init failed: %d\n", status);
		return 1;
	}

	status = cp_meter_process(&meter, input, 4);
	if (status != CP_OK) {
		printf("test_meter: process failed: %d\n", status);
		return 1;
	}

	expected_rms = sqrtf(2.5f / 4.0f);

	if (!close_enough(meter.peak[0], 1.0f)) {
		printf("test_meter: peak mismatch: %f\n", meter.peak[0]);
		return 1;
	}
	if (!close_enough(meter.rms[0], expected_rms)) {
		printf("test_meter: rms mismatch: %f\n", meter.rms[0]);
		return 1;
	}

	return 0;
}

static int
close_enough(cp_sample_t a, cp_sample_t b)
{
	return fabsf(a - b) <= TEST_EPSILON;
}

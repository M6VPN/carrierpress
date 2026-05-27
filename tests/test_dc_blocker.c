/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_dc_blocker.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_dc_blocker.h"

#define TEST_DC_LEVEL	0.50f
#define TEST_FRAMES	1024
#define TEST_R		0.95f

int
main(void)
{
	cp_sample_t input[TEST_FRAMES];
	cp_sample_t output[TEST_FRAMES];
	struct cp_dc_blocker blocker;
	size_t frame;
	int status;

	for (frame = 0; frame < TEST_FRAMES; frame++)
		input[frame] = TEST_DC_LEVEL;

	status = cp_dc_blocker_init(&blocker, CP_CHANNELS_MONO, TEST_R);
	if (status != CP_OK) {
		printf("test_dc_blocker: init failed: %d\n", status);
		return 1;
	}

	status = cp_dc_blocker_process(&blocker, input, output, TEST_FRAMES);
	if (status != CP_OK) {
		printf("test_dc_blocker: process failed: %d\n", status);
		return 1;
	}

	if (fabsf(output[TEST_FRAMES - 1]) >= 0.01f) {
		printf("test_dc_blocker: dc not reduced: %f\n",
		    output[TEST_FRAMES - 1]);
		return 1;
	}

	if (fabsf(output[TEST_FRAMES - 1]) >= fabsf(output[0])) {
		printf("test_dc_blocker: dc output did not decay\n");
		return 1;
	}

	return 0;
}

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_limiter.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_limiter.h"

#define TEST_CEILING	0.50f
#define TEST_FRAMES	5

int
main(void)
{
	const cp_sample_t input[TEST_FRAMES] = {
		-1.0f, -0.6f, 0.0f, 0.6f, 1.0f
	};
	cp_sample_t output[TEST_FRAMES];
	struct cp_limiter limiter;
	size_t index;
	int status;

	status = cp_limiter_init(&limiter, CP_CHANNELS_MONO, TEST_CEILING);
	if (status != CP_OK) {
		printf("test_limiter: init failed: %d\n", status);
		return 1;
	}

	status = cp_limiter_process(&limiter, input, output, TEST_FRAMES);
	if (status != CP_OK) {
		printf("test_limiter: process failed: %d\n", status);
		return 1;
	}

	for (index = 0; index < TEST_FRAMES; index++) {
		if (fabsf(output[index]) > TEST_CEILING) {
			printf("test_limiter: ceiling exceeded: %f\n",
			    output[index]);
			return 1;
		}
	}

	if (output[0] != -TEST_CEILING || output[TEST_FRAMES - 1] != TEST_CEILING) {
		printf("test_limiter: clamp endpoints mismatch\n");
		return 1;
	}

	return 0;
}

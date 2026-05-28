/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_biquad.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_biquad.h"

#define TEST_FRAMES	48000
#define TEST_RATE	48000.0f
#define TEST_TWO_PI	6.28318530717958647692f

static cp_sample_t	measure_notch(cp_sample_t);
static int		test_invalid_lowpass(void);
static int		test_invalid_notch(void);
static int		test_notch_reduces_center(void);

int
main(void)
{
	if (!test_notch_reduces_center())
		return 1;
	if (!test_invalid_notch())
		return 1;
	if (!test_invalid_lowpass())
		return 1;

	return 0;
}

static cp_sample_t
measure_notch(cp_sample_t frequency)
{
	struct cp_biquad_coeff coeff;
	struct cp_biquad_state state;
	cp_sample_t input_rms;
	cp_sample_t output_rms;
	cp_sample_t output;
	cp_sample_t phase;
	cp_sample_t sample;
	cp_sample_t sum_in;
	cp_sample_t sum_out;
	size_t frame;

	if (cp_biquad_notch(&coeff, TEST_RATE, 50.0f, 35.0f) != CP_OK)
		return 1.0f;
	if (cp_biquad_reset(&state) != CP_OK)
		return 1.0f;

	phase   = 0.0f;
	sum_in  = 0.0f;
	sum_out = 0.0f;
	for (frame = 0; frame < TEST_FRAMES; frame++) {
		sample = sinf(phase);
		output = cp_biquad_process_sample(&coeff, &state, sample);
		if (frame >= TEST_FRAMES / 2) {
			sum_in  += sample * sample;
			sum_out += output * output;
		}
		phase += (TEST_TWO_PI * frequency) / TEST_RATE;
		if (phase >= TEST_TWO_PI)
			phase -= TEST_TWO_PI;
	}

	input_rms  = sqrtf(sum_in / (cp_sample_t)(TEST_FRAMES / 2));
	output_rms = sqrtf(sum_out / (cp_sample_t)(TEST_FRAMES / 2));

	return output_rms / input_rms;
}

static int
test_invalid_lowpass(void)
{
	struct cp_biquad_coeff coeff;

	if (cp_biquad_lowpass(NULL, TEST_RATE, 250.0f, 0.707f) !=
	    CP_ERR_NULL)
		return 0;
	if (cp_biquad_lowpass(&coeff, TEST_RATE, 0.0f, 0.707f) !=
	    CP_ERR_RANGE)
		return 0;
	if (cp_biquad_lowpass(&coeff, TEST_RATE, 250.0f, 0.0f) !=
	    CP_ERR_RANGE)
		return 0;

	return 1;
}

static int
test_invalid_notch(void)
{
	struct cp_biquad_coeff coeff;

	if (cp_biquad_notch(NULL, TEST_RATE, 50.0f, 35.0f) != CP_ERR_NULL)
		return 0;
	if (cp_biquad_notch(&coeff, TEST_RATE, 0.0f, 35.0f) != CP_ERR_RANGE)
		return 0;
	if (cp_biquad_notch(&coeff, TEST_RATE, 50.0f, 0.0f) != CP_ERR_RANGE)
		return 0;

	return 1;
}

static int
test_notch_reduces_center(void)
{
	cp_sample_t center_ratio;
	cp_sample_t nearby_ratio;

	center_ratio = measure_notch(50.0f);
	nearby_ratio = measure_notch(55.0f);

	if (center_ratio >= 0.35f) {
		printf("test_biquad: center notch too weak: %f\n",
		    center_ratio);
		return 0;
	}
	if (nearby_ratio <= 0.70f) {
		printf("test_biquad: nearby frequency damaged: %f\n",
		    nearby_ratio);
		return 0;
	}

	return 1;
}

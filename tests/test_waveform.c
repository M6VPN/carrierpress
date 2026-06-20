/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_waveform.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include "cp_waveform.h"

static int	test_capture_clamps(void);
static int	test_capture_mono(void);
static int	test_capture_stereo(void);
static int	test_clear(void);
static int	test_invalid_args(void);

int
main(void)
{
	if (!test_clear())
		return 1;
	if (!test_capture_mono())
		return 1;
	if (!test_capture_stereo())
		return 1;
	if (!test_capture_clamps())
		return 1;
	if (!test_invalid_args())
		return 1;

	return 0;
}

static int
test_capture_clamps(void)
{
	struct cp_waveform_snapshot snapshot;
	cp_sample_t samples[4];

	samples[0] = 2.0f;
	samples[1] = -2.0f;
	samples[2] = NAN;
	samples[3] = 0.25f;
	if (cp_waveform_capture(&snapshot, samples, 4, 1) != CP_OK) {
		printf("test_waveform: clamp capture failed\n");
		return 0;
	}
	if (snapshot.values[0] != CP_WAVEFORM_MAX_VALUE ||
	    snapshot.values[1] != CP_WAVEFORM_MIN_VALUE ||
	    snapshot.values[2] != 0 ||
	    snapshot.values[3] != 2500) {
		printf("test_waveform: clamp values failed\n");
		return 0;
	}

	return 1;
}

static int
test_capture_mono(void)
{
	struct cp_waveform_snapshot snapshot;
	cp_sample_t samples[4];

	samples[0] = -0.5f;
	samples[1] = 0.0f;
	samples[2] = 0.5f;
	samples[3] = 1.0f;
	if (cp_waveform_capture(&snapshot, samples, 4, 1) != CP_OK) {
		printf("test_waveform: mono capture failed\n");
		return 0;
	}
	if (!snapshot.valid || snapshot.point_count != 4 ||
	    snapshot.channel_count != 1 ||
	    snapshot.values[0] != -5000 ||
	    snapshot.values[2] != 5000 ||
	    snapshot.values[3] != CP_WAVEFORM_MAX_VALUE) {
		printf("test_waveform: mono values failed\n");
		return 0;
	}

	return 1;
}

static int
test_capture_stereo(void)
{
	struct cp_waveform_snapshot snapshot;
	cp_sample_t samples[8];

	samples[0] = 0.1f;
	samples[1] = 0.9f;
	samples[2] = -0.2f;
	samples[3] = -0.8f;
	samples[4] = 0.3f;
	samples[5] = 0.7f;
	samples[6] = -0.4f;
	samples[7] = -0.6f;
	if (cp_waveform_capture(&snapshot, samples, 4, 2) != CP_OK) {
		printf("test_waveform: stereo capture failed\n");
		return 0;
	}
	if (!snapshot.valid || snapshot.channel_count != 2 ||
	    snapshot.values[0] != 1000 ||
	    snapshot.values[1] != -2000 ||
	    snapshot.values[2] != 3000 ||
	    snapshot.values[3] != -4000) {
		printf("test_waveform: stereo channel 1 values failed\n");
		return 0;
	}

	return 1;
}

static int
test_clear(void)
{
	struct cp_waveform_snapshot snapshot;

	snapshot.valid = 1;
	snapshot.point_count = 2;
	snapshot.channel_count = 1;
	snapshot.values[0] = 123;
	cp_waveform_clear(&snapshot);
	if (snapshot.valid || snapshot.point_count != 0 ||
	    snapshot.channel_count != 0 || snapshot.values[0] != 0) {
		printf("test_waveform: clear failed\n");
		return 0;
	}

	return 1;
}

static int
test_invalid_args(void)
{
	struct cp_waveform_snapshot snapshot;
	cp_sample_t sample;

	sample = 0.0f;
	if (cp_waveform_capture(NULL, &sample, 1, 1) != CP_ERR_NULL)
		return 0;
	if (cp_waveform_capture(&snapshot, NULL, 1, 1) != CP_ERR_NULL)
		return 0;
	if (cp_waveform_capture(&snapshot, &sample, 0, 1) != CP_ERR_RANGE)
		return 0;
	if (cp_waveform_capture(&snapshot, &sample, 1, 0) !=
	    CP_ERR_CHANNELS)
		return 0;
	if (cp_waveform_capture(&snapshot, &sample, 1, CP_MAX_CHANNELS + 1) !=
	    CP_ERR_CHANNELS)
		return 0;
	if (fabsf(cp_waveform_value_to_sample(5000) - 0.5f) > 0.001f)
		return 0;
	if (cp_waveform_value_to_sample(20000) != CP_SAMPLE_MAX)
		return 0;

	return 1;
}

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_spectrum.c */

#include <sys/types.h>

#include <math.h>
#include <string.h>

#include "cp_spectrum.h"

#define CP_SPECTRUM_EPSILON	1.0e-12f
#define CP_SPECTRUM_TWO_PI	6.28318530717958647692f

static cp_sample_t	cp_spectrum_clamp_sample(cp_sample_t);
static cp_sample_t	cp_spectrum_input_value_to_sample(int);
static int		cp_spectrum_magnitude_to_value(float, float);
static int		cp_spectrum_sample_to_input_value(cp_sample_t);

int
cp_spectrum_analyze(struct cp_spectrum_analyzer *analyzer,
	const struct cp_spectrum_input *input,
	struct cp_spectrum_snapshot *snapshot)
{
	float magnitude;
	float max_magnitude;
	size_t bin;
	size_t index;
	size_t limit;
	size_t peak_bin;

	if (analyzer == NULL || input == NULL || snapshot == NULL)
		return CP_ERR_NULL;
	cp_spectrum_clear(snapshot);
	if (!analyzer->initialized || analyzer->plan == NULL)
		return CP_ERR_RANGE;
	if (!input->valid || input->frame_count == 0 ||
	    input->sample_rate_hz < CP_SPECTRUM_MIN_RATE)
		return CP_ERR_RANGE;

	for (index = 0; index < CP_SPECTRUM_FFT_SIZE; index++) {
		if (index < input->frame_count) {
			analyzer->input[index] = cp_spectrum_input_value_to_sample(
			    input->values[index]) * analyzer->window[index];
		} else {
			analyzer->input[index] = 0.0f;
		}
	}

	fftwf_execute(analyzer->plan);

	limit = CP_SPECTRUM_BINS;
	if (limit > (CP_SPECTRUM_FFT_SIZE / 2) + 1)
		limit = (CP_SPECTRUM_FFT_SIZE / 2) + 1;

	max_magnitude = 0.0f;
	peak_bin = 0;
	for (bin = 0; bin < limit; bin++) {
		magnitude = hypotf(analyzer->output[bin][0],
		    analyzer->output[bin][1]);
		if (isfinite(magnitude) && magnitude > max_magnitude) {
			max_magnitude = magnitude;
			peak_bin = bin;
		}
	}

	for (bin = 0; bin < limit; bin++) {
		magnitude = hypotf(analyzer->output[bin][0],
		    analyzer->output[bin][1]);
		snapshot->magnitudes[bin] = cp_spectrum_magnitude_to_value(
		    magnitude, max_magnitude);
	}

	snapshot->bin_count = limit;
	snapshot->sample_rate_hz = input->sample_rate_hz;
	snapshot->peak_bin = peak_bin;
	snapshot->peak_magnitude = snapshot->magnitudes[peak_bin];
	snapshot->valid = 1;

	return CP_OK;
}

void
cp_spectrum_analyzer_close(struct cp_spectrum_analyzer *analyzer)
{
	if (analyzer == NULL)
		return;

	if (analyzer->plan != NULL)
		fftwf_destroy_plan(analyzer->plan);
	memset(analyzer, 0, sizeof(*analyzer));
}

int
cp_spectrum_analyzer_init(struct cp_spectrum_analyzer *analyzer)
{
	size_t index;

	if (analyzer == NULL)
		return CP_ERR_NULL;

	memset(analyzer, 0, sizeof(*analyzer));
	for (index = 0; index < CP_SPECTRUM_FFT_SIZE; index++) {
		analyzer->window[index] = 0.5f - 0.5f * cosf(
		    CP_SPECTRUM_TWO_PI * (float)index /
		    (float)(CP_SPECTRUM_FFT_SIZE - 1));
	}
	analyzer->plan = fftwf_plan_dft_r2c_1d(CP_SPECTRUM_FFT_SIZE,
	    analyzer->input, analyzer->output, FFTW_ESTIMATE);
	if (analyzer->plan == NULL)
		return CP_ERR_BUFFER;

	analyzer->initialized = 1;
	return CP_OK;
}

int
cp_spectrum_capture_input(struct cp_spectrum_input *input,
	const cp_sample_t *samples, size_t frames, size_t channels,
	double sample_rate_hz)
{
	cp_sample_t mixed;
	size_t channel;
	size_t frame;
	size_t limit;

	if (input == NULL || samples == NULL)
		return CP_ERR_NULL;
	cp_spectrum_input_clear(input);
	if (frames == 0 || sample_rate_hz < CP_SPECTRUM_MIN_RATE)
		return CP_ERR_RANGE;
	if (channels == 0 || channels > CP_MAX_CHANNELS)
		return CP_ERR_CHANNELS;

	limit = frames < CP_SPECTRUM_FFT_SIZE ? frames :
	    CP_SPECTRUM_FFT_SIZE;
	for (frame = 0; frame < limit; frame++) {
		mixed = 0.0f;
		for (channel = 0; channel < channels; channel++) {
			mixed += cp_spectrum_clamp_sample(
			    samples[frame * channels + channel]);
		}
		mixed /= (cp_sample_t)channels;
		input->values[frame] = cp_spectrum_sample_to_input_value(mixed);
	}

	input->frame_count = limit;
	input->channel_count = channels;
	input->sample_rate_hz = sample_rate_hz;
	input->valid = 1;

	return CP_OK;
}

void
cp_spectrum_clear(struct cp_spectrum_snapshot *snapshot)
{
	if (snapshot == NULL)
		return;

	memset(snapshot, 0, sizeof(*snapshot));
}

void
cp_spectrum_input_clear(struct cp_spectrum_input *input)
{
	if (input == NULL)
		return;

	memset(input, 0, sizeof(*input));
}

static cp_sample_t
cp_spectrum_clamp_sample(cp_sample_t sample)
{
	if (!isfinite(sample))
		return 0.0f;
	if (sample > CP_SAMPLE_MAX)
		return CP_SAMPLE_MAX;
	if (sample < CP_SAMPLE_MIN)
		return CP_SAMPLE_MIN;

	return sample;
}

static cp_sample_t
cp_spectrum_input_value_to_sample(int value)
{
	if (value > CP_SPECTRUM_SCALE)
		value = CP_SPECTRUM_SCALE;
	if (value < -CP_SPECTRUM_SCALE)
		value = -CP_SPECTRUM_SCALE;

	return (cp_sample_t)value / (cp_sample_t)CP_SPECTRUM_SCALE;
}

static int
cp_spectrum_magnitude_to_value(float magnitude, float max_magnitude)
{
	float ratio;

	if (!isfinite(magnitude) || !isfinite(max_magnitude) ||
	    max_magnitude <= CP_SPECTRUM_EPSILON)
		return 0;

	ratio = magnitude / max_magnitude;
	if (ratio < 0.0f)
		ratio = 0.0f;
	if (ratio > 1.0f)
		ratio = 1.0f;

	return (int)(ratio * (float)CP_SPECTRUM_SCALE);
}

static int
cp_spectrum_sample_to_input_value(cp_sample_t sample)
{
	cp_sample_t clamped;

	clamped = cp_spectrum_clamp_sample(sample);
	return (int)(clamped * (cp_sample_t)CP_SPECTRUM_SCALE);
}

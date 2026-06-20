/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_waveform.c */

#include <sys/types.h>

#include <math.h>
#include <string.h>

#include "cp_waveform.h"

static cp_sample_t	cp_waveform_clamp_sample(cp_sample_t);
static int		cp_waveform_sample_to_value(cp_sample_t);

int
cp_waveform_capture(struct cp_waveform_snapshot *snapshot,
	const cp_sample_t *samples, size_t frames, size_t channels)
{
	cp_sample_t best;
	cp_sample_t current;
	cp_sample_t current_abs;
	cp_sample_t best_abs;
	size_t end;
	size_t frame;
	size_t point;
	size_t points;
	size_t start;

	if (snapshot == NULL || samples == NULL)
		return CP_ERR_NULL;
	cp_waveform_clear(snapshot);
	if (frames == 0)
		return CP_ERR_RANGE;
	if (channels == 0 || channels > CP_MAX_CHANNELS)
		return CP_ERR_CHANNELS;

	points = frames < CP_WAVEFORM_POINTS ? frames : CP_WAVEFORM_POINTS;
	for (point = 0; point < points; point++) {
		start = point * frames / points;
		end = (point + 1) * frames / points;
		if (end <= start)
			end = start + 1;
		best = 0.0f;
		best_abs = 0.0f;
		for (frame = start; frame < end && frame < frames; frame++) {
			current = cp_waveform_clamp_sample(
			    samples[frame * channels]);
			current_abs = fabsf(current);
			if (current_abs >= best_abs) {
				best = current;
				best_abs = current_abs;
			}
		}
		snapshot->values[point] = cp_waveform_sample_to_value(best);
	}

	snapshot->point_count = points;
	snapshot->channel_count = channels;
	snapshot->valid = 1;

	return CP_OK;
}

void
cp_waveform_clear(struct cp_waveform_snapshot *snapshot)
{
	if (snapshot == NULL)
		return;

	memset(snapshot, 0, sizeof(*snapshot));
}

cp_sample_t
cp_waveform_value_to_sample(int value)
{
	if (value > CP_WAVEFORM_MAX_VALUE)
		value = CP_WAVEFORM_MAX_VALUE;
	if (value < CP_WAVEFORM_MIN_VALUE)
		value = CP_WAVEFORM_MIN_VALUE;

	return (cp_sample_t)value / (cp_sample_t)CP_WAVEFORM_SCALE;
}

static cp_sample_t
cp_waveform_clamp_sample(cp_sample_t sample)
{
	if (!isfinite(sample))
		return 0.0f;
	if (sample > CP_SAMPLE_MAX)
		return CP_SAMPLE_MAX;
	if (sample < CP_SAMPLE_MIN)
		return CP_SAMPLE_MIN;

	return sample;
}

static int
cp_waveform_sample_to_value(cp_sample_t sample)
{
	cp_sample_t clamped;

	clamped = cp_waveform_clamp_sample(sample);
	return (int)(clamped * (cp_sample_t)CP_WAVEFORM_SCALE);
}

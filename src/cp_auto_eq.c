/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_auto_eq.c */

#include <sys/types.h>

#include <math.h>
#include <string.h>

#include "cp_auto_eq.h"

#define CP_AUTO_EQ_Q			0.70710678f
#define CP_AUTO_EQ_ENERGY_FLOOR		1.0e-12
#define CP_AUTO_EQ_SILENCE_RMS		0.0001f
#define CP_AUTO_EQ_BASS_HEAVY_WEIGHT	0.55f
#define CP_AUTO_EQ_THIN_LOW_WEIGHT	0.18f
#define CP_AUTO_EQ_THIN_HIGH_WEIGHT	0.30f
#define CP_AUTO_EQ_DARK_HIGH_WEIGHT	0.12f
#define CP_AUTO_EQ_BRIGHT_HIGH_WEIGHT	0.45f
#define CP_AUTO_EQ_LIMITED_HIGH_WEIGHT	0.12f
#define CP_AUTO_EQ_LIMITED_LOW_WEIGHT	0.20f

static cp_sample_t	cp_auto_eq_clean(cp_sample_t);
static cp_sample_t	cp_auto_eq_db(cp_sample_t, cp_sample_t);
static void		cp_auto_eq_finish_window(struct cp_auto_eq *);
static int		cp_auto_eq_init_band(struct cp_auto_eq *, size_t);
static enum cp_auto_eq_source_hint cp_auto_eq_source_hint(
			    const struct cp_auto_eq_metrics *);
static void		cp_auto_eq_zero_window(struct cp_auto_eq *);
static int		cp_auto_eq_validate_config(
			    const struct cp_auto_eq_config *);

static cp_sample_t
cp_auto_eq_clean(cp_sample_t sample)
{
	if (!isfinite(sample))
		return 0.0f;
	if (fabsf(sample) < 0.00000000000000000001f)
		return 0.0f;

	return sample;
}

static cp_sample_t
cp_auto_eq_db(cp_sample_t value, cp_sample_t reference)
{
	cp_sample_t ratio;

	if (!isfinite(value) || !isfinite(reference) || value <= 0.0f ||
	    reference <= 0.0f)
		return CP_AUTO_EQ_DB_FLOOR;

	ratio = value / reference;
	if (ratio <= 0.0f)
		return CP_AUTO_EQ_DB_FLOOR;

	value = 20.0f * log10f(ratio);
	if (!isfinite(value) || value < CP_AUTO_EQ_DB_FLOOR)
		return CP_AUTO_EQ_DB_FLOOR;

	return value;
}

static void
cp_auto_eq_finish_window(struct cp_auto_eq *auto_eq)
{
	double band_sum;
	cp_sample_t high_energy;
	cp_sample_t low_energy;
	size_t band;

	if (auto_eq == NULL)
		return;

	if (auto_eq->total_sample_count == 0 ||
	    auto_eq->total_energy <= CP_AUTO_EQ_ENERGY_FLOOR) {
		for (band = 0; band < CP_AUTO_EQ_BAND_COUNT; band++) {
			auto_eq->metrics.band_rms[band] = 0.0f;
			auto_eq->metrics.band_relative_db[band] =
			    CP_AUTO_EQ_DB_FLOOR;
		}
		auto_eq->metrics.total_rms = 0.0f;
		auto_eq->metrics.spectral_tilt_db = 0.0f;
		auto_eq->metrics.low_frequency_weight = 0.0f;
		auto_eq->metrics.presence_weight = 0.0f;
		auto_eq->metrics.high_frequency_weight = 0.0f;
		auto_eq->metrics.source_hint = CP_AUTO_EQ_SOURCE_SILENCE;
		return;
	}

	auto_eq->metrics.total_rms = (cp_sample_t)sqrt(
	    auto_eq->total_energy / (double)auto_eq->total_sample_count);

	band_sum = 0.0;
	for (band = 0; band < CP_AUTO_EQ_BAND_COUNT; band++) {
		if (!auto_eq->metrics.band_enabled[band]) {
			auto_eq->metrics.band_rms[band] = 0.0f;
			auto_eq->metrics.band_relative_db[band] =
			    CP_AUTO_EQ_DB_FLOOR;
			continue;
		}
		auto_eq->metrics.band_rms[band] = (cp_sample_t)sqrt(
		    auto_eq->band_energy[band] /
		    (double)auto_eq->total_sample_count);
		auto_eq->metrics.band_relative_db[band] = cp_auto_eq_db(
		    auto_eq->metrics.band_rms[band],
		    auto_eq->metrics.total_rms);
		band_sum += auto_eq->band_energy[band];
	}

	if (band_sum <= CP_AUTO_EQ_ENERGY_FLOOR) {
		auto_eq->metrics.low_frequency_weight = 0.0f;
		auto_eq->metrics.presence_weight = 0.0f;
		auto_eq->metrics.high_frequency_weight = 0.0f;
	} else {
		auto_eq->metrics.low_frequency_weight = (cp_sample_t)(
		    (auto_eq->band_energy[CP_AUTO_EQ_BAND_BASS] +
		    auto_eq->band_energy[CP_AUTO_EQ_BAND_LOW_MID]) /
		    band_sum);
		auto_eq->metrics.presence_weight = (cp_sample_t)(
		    auto_eq->band_energy[CP_AUTO_EQ_BAND_PRESENCE] /
		    band_sum);
		auto_eq->metrics.high_frequency_weight = (cp_sample_t)(
		    auto_eq->band_energy[CP_AUTO_EQ_BAND_HIGH] / band_sum);
	}

	low_energy = auto_eq->metrics.band_rms[CP_AUTO_EQ_BAND_BASS];
	high_energy = auto_eq->metrics.band_rms[CP_AUTO_EQ_BAND_HIGH];
	if (!auto_eq->metrics.band_enabled[CP_AUTO_EQ_BAND_HIGH])
		high_energy =
		    auto_eq->metrics.band_rms[CP_AUTO_EQ_BAND_PRESENCE];
	auto_eq->metrics.spectral_tilt_db =
	    cp_auto_eq_db(high_energy, low_energy);
	auto_eq->metrics.source_hint =
	    cp_auto_eq_source_hint(&auto_eq->metrics);
}

static int
cp_auto_eq_init_band(struct cp_auto_eq *auto_eq, size_t band)
{
	cp_sample_t nyquist;
	int status;

	if (auto_eq == NULL || band >= CP_AUTO_EQ_BAND_COUNT)
		return CP_ERR_RANGE;

	nyquist = auto_eq->config.sample_rate * 0.5f;
	if (auto_eq->high_cut[band] >= nyquist) {
		auto_eq->metrics.band_enabled[band] = 0;
		return CP_OK;
	}

	auto_eq->metrics.band_enabled[band] = 1;
	status = cp_biquad_highpass(&auto_eq->highpass_coeff[band],
	    auto_eq->config.sample_rate, auto_eq->low_cut[band],
	    CP_AUTO_EQ_Q);
	if (status != CP_OK)
		return status;

	return cp_biquad_lowpass(&auto_eq->lowpass_coeff[band],
	    auto_eq->config.sample_rate, auto_eq->high_cut[band],
	    CP_AUTO_EQ_Q);
}

static enum cp_auto_eq_source_hint
cp_auto_eq_source_hint(const struct cp_auto_eq_metrics *metrics)
{
	cp_sample_t high_weight;

	if (metrics == NULL)
		return CP_AUTO_EQ_SOURCE_UNKNOWN;
	if (metrics->total_rms <= CP_AUTO_EQ_SILENCE_RMS)
		return CP_AUTO_EQ_SOURCE_SILENCE;

	high_weight = metrics->presence_weight + metrics->high_frequency_weight;
	if (high_weight <= CP_AUTO_EQ_LIMITED_HIGH_WEIGHT &&
	    metrics->low_frequency_weight <= CP_AUTO_EQ_LIMITED_LOW_WEIGHT)
		return CP_AUTO_EQ_SOURCE_LIMITED_BAND;
	if (metrics->low_frequency_weight >= CP_AUTO_EQ_BASS_HEAVY_WEIGHT)
		return CP_AUTO_EQ_SOURCE_BASS_HEAVY;
	if (metrics->low_frequency_weight <= CP_AUTO_EQ_THIN_LOW_WEIGHT &&
	    high_weight >= CP_AUTO_EQ_THIN_HIGH_WEIGHT)
		return CP_AUTO_EQ_SOURCE_THIN;
	if (high_weight <= CP_AUTO_EQ_DARK_HIGH_WEIGHT)
		return CP_AUTO_EQ_SOURCE_DARK;
	if (high_weight >= CP_AUTO_EQ_BRIGHT_HIGH_WEIGHT)
		return CP_AUTO_EQ_SOURCE_BRIGHT;

	return CP_AUTO_EQ_SOURCE_BALANCED;
}

static void
cp_auto_eq_zero_window(struct cp_auto_eq *auto_eq)
{
	size_t band;

	if (auto_eq == NULL)
		return;

	auto_eq->total_energy = 0.0;
	auto_eq->total_sample_count = 0;
	auto_eq->window_frames_seen = 0;
	for (band = 0; band < CP_AUTO_EQ_BAND_COUNT; band++)
		auto_eq->band_energy[band] = 0.0;
}

static int
cp_auto_eq_validate_config(const struct cp_auto_eq_config *config)
{
	if (config == NULL)
		return CP_ERR_NULL;
	if (config->channel_count != CP_CHANNELS_MONO &&
	    config->channel_count != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (!isfinite(config->sample_rate) ||
	    config->sample_rate < CP_AUTO_EQ_MIN_SAMPLE_RATE ||
	    config->sample_rate > CP_AUTO_EQ_MAX_SAMPLE_RATE)
		return CP_ERR_RANGE;
	if (config->analysis_window_frames <
	    CP_AUTO_EQ_MIN_WINDOW_FRAMES ||
	    config->analysis_window_frames >
	    CP_AUTO_EQ_MAX_WINDOW_FRAMES)
		return CP_ERR_RANGE;

	return CP_OK;
}

void
cp_auto_eq_default_config(struct cp_auto_eq_config *config)
{
	if (config == NULL)
		return;

	config->enabled = CP_AUTO_EQ_DEFAULT_ENABLED;
	config->sample_rate = CP_AUTO_EQ_DEFAULT_SAMPLE_RATE;
	config->channel_count = CP_AUTO_EQ_DEFAULT_CHANNELS;
	config->analysis_window_frames =
	    CP_AUTO_EQ_DEFAULT_WINDOW_FRAMES;
}

const struct cp_auto_eq_metrics *
cp_auto_eq_get_metrics(const struct cp_auto_eq *auto_eq)
{
	if (auto_eq == NULL)
		return NULL;

	return &auto_eq->metrics;
}

int
cp_auto_eq_init(struct cp_auto_eq *auto_eq,
	const struct cp_auto_eq_config *config)
{
	size_t band;
	int status;

	if (auto_eq == NULL || config == NULL)
		return CP_ERR_NULL;

	status = cp_auto_eq_validate_config(config);
	if (status != CP_OK)
		return status;

	(void)memset(auto_eq, 0, sizeof(*auto_eq));
	auto_eq->config = *config;
	auto_eq->metrics.finite = 1;
	auto_eq->metrics.source_hint = CP_AUTO_EQ_SOURCE_UNKNOWN;
	auto_eq->low_cut[CP_AUTO_EQ_BAND_BASS] = 80.0f;
	auto_eq->high_cut[CP_AUTO_EQ_BAND_BASS] = 250.0f;
	auto_eq->low_cut[CP_AUTO_EQ_BAND_LOW_MID] = 250.0f;
	auto_eq->high_cut[CP_AUTO_EQ_BAND_LOW_MID] = 800.0f;
	auto_eq->low_cut[CP_AUTO_EQ_BAND_MID] = 800.0f;
	auto_eq->high_cut[CP_AUTO_EQ_BAND_MID] = 2500.0f;
	auto_eq->low_cut[CP_AUTO_EQ_BAND_PRESENCE] = 2500.0f;
	auto_eq->high_cut[CP_AUTO_EQ_BAND_PRESENCE] = 4500.0f;
	auto_eq->low_cut[CP_AUTO_EQ_BAND_HIGH] = 4500.0f;
	auto_eq->high_cut[CP_AUTO_EQ_BAND_HIGH] = 9000.0f;

	for (band = 0; band < CP_AUTO_EQ_BAND_COUNT; band++) {
		status = cp_auto_eq_init_band(auto_eq, band);
		if (status != CP_OK)
			return status;
	}

	return cp_auto_eq_reset(auto_eq);
}

const char *
cp_auto_eq_source_hint_string(enum cp_auto_eq_source_hint hint)
{
	switch (hint) {
	case CP_AUTO_EQ_SOURCE_SILENCE:
		return "silence";
	case CP_AUTO_EQ_SOURCE_BASS_HEAVY:
		return "bass-heavy";
	case CP_AUTO_EQ_SOURCE_THIN:
		return "thin";
	case CP_AUTO_EQ_SOURCE_DARK:
		return "dark";
	case CP_AUTO_EQ_SOURCE_BRIGHT:
		return "bright";
	case CP_AUTO_EQ_SOURCE_BALANCED:
		return "balanced";
	case CP_AUTO_EQ_SOURCE_LIMITED_BAND:
		return "limited-band";
	case CP_AUTO_EQ_SOURCE_UNKNOWN:
	default:
		return "unknown";
	}
}

int
cp_auto_eq_process(struct cp_auto_eq *auto_eq, const cp_sample_t *input,
	size_t frames)
{
	cp_sample_t sample;
	cp_sample_t band_sample;
	size_t band;
	size_t channel;
	size_t frame;
	size_t index;

	if (auto_eq == NULL || input == NULL)
		return CP_ERR_NULL;
	if (auto_eq->config.channel_count != CP_CHANNELS_MONO &&
	    auto_eq->config.channel_count != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;
	if (frames == 0)
		return CP_ERR_RANGE;
	if (!auto_eq->config.enabled)
		return CP_OK;

	for (frame = 0; frame < frames; frame++) {
		if (auto_eq->window_frames_seen >=
		    auto_eq->config.analysis_window_frames) {
			cp_auto_eq_finish_window(auto_eq);
			cp_auto_eq_zero_window(auto_eq);
		}

		for (channel = 0; channel < auto_eq->config.channel_count;
		    channel++) {
			index = (frame * auto_eq->config.channel_count) +
			    channel;
			if (!isfinite(input[index]))
				auto_eq->metrics.finite = 0;
			sample = cp_auto_eq_clean(input[index]);
			auto_eq->total_energy +=
			    (double)sample * (double)sample;
			auto_eq->total_sample_count++;
			for (band = 0; band < CP_AUTO_EQ_BAND_COUNT; band++) {
				if (!auto_eq->metrics.band_enabled[band])
					continue;
				band_sample = cp_biquad_process_sample(
				    &auto_eq->highpass_coeff[band],
				    &auto_eq->highpass_state[band][channel],
				    sample);
				band_sample = cp_biquad_process_sample(
				    &auto_eq->lowpass_coeff[band],
				    &auto_eq->lowpass_state[band][channel],
				    band_sample);
				auto_eq->band_energy[band] +=
				    (double)band_sample *
				    (double)band_sample;
			}
		}
		auto_eq->window_frames_seen++;
	}

	cp_auto_eq_finish_window(auto_eq);
	return CP_OK;
}

int
cp_auto_eq_reset(struct cp_auto_eq *auto_eq)
{
	size_t band;
	size_t channel;
	int status;

	if (auto_eq == NULL)
		return CP_ERR_NULL;
	if (auto_eq->config.channel_count != CP_CHANNELS_MONO &&
	    auto_eq->config.channel_count != CP_CHANNELS_STEREO)
		return CP_ERR_CHANNELS;

	for (band = 0; band < CP_AUTO_EQ_BAND_COUNT; band++) {
		for (channel = 0; channel < auto_eq->config.channel_count;
		    channel++) {
			status = cp_biquad_reset(
			    &auto_eq->highpass_state[band][channel]);
			if (status != CP_OK)
				return status;
			status = cp_biquad_reset(
			    &auto_eq->lowpass_state[band][channel]);
			if (status != CP_OK)
				return status;
		}
		auto_eq->metrics.band_rms[band] = 0.0f;
		auto_eq->metrics.band_relative_db[band] =
		    CP_AUTO_EQ_DB_FLOOR;
	}

	auto_eq->metrics.total_rms = 0.0f;
	auto_eq->metrics.spectral_tilt_db = 0.0f;
	auto_eq->metrics.low_frequency_weight = 0.0f;
	auto_eq->metrics.presence_weight = 0.0f;
	auto_eq->metrics.high_frequency_weight = 0.0f;
	auto_eq->metrics.source_hint = CP_AUTO_EQ_SOURCE_UNKNOWN;
	auto_eq->metrics.finite = 1;
	cp_auto_eq_zero_window(auto_eq);

	return CP_OK;
}

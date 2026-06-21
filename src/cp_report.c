/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_report.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "cp_report.h"
#include "cp_version.h"

static double	cp_report_abs(double);
static double	cp_report_crest(double, double);
static double	cp_report_dc(double, uint64_t);
static double	cp_report_rms(double, uint64_t);
static int	cp_report_finite(cp_sample_t);
static void	cp_report_json_bool(FILE *, const char *, int, int);
static void	cp_report_json_double(FILE *, const char *, double, int);
static void	cp_report_json_null_string(FILE *);
static void	cp_report_json_string(FILE *, const char *);
static void	cp_report_json_string_field(FILE *, const char *,
		    const char *, int);
static int	cp_report_write_metrics(FILE *,
		    const struct cp_report_processed_file *);
static int	cp_report_write_stages(FILE *, const struct cp_block_config *);

void
cp_report_metrics_init(struct cp_report_metrics *metrics)
{
	if (metrics == NULL)
		return;

	(void)memset(metrics, 0, sizeof(*metrics));
	metrics->finite = 1;
}

int
cp_report_metrics_update(struct cp_report_metrics *metrics,
	const cp_sample_t *input, const cp_sample_t *output, size_t frames,
	size_t channels)
{
	cp_sample_t input_sample;
	cp_sample_t output_sample;
	size_t index;
	size_t samples;

	if (metrics == NULL || input == NULL || output == NULL)
		return CP_REPORT_ERR_NULL;
	if (channels != CP_CHANNELS_MONO && channels != CP_CHANNELS_STEREO)
		return CP_REPORT_ERR_RANGE;
	if (frames > SIZE_MAX / channels)
		return CP_REPORT_ERR_RANGE;

	samples = frames * channels;
	for (index = 0; index < samples; index++) {
		input_sample = input[index];
		output_sample = output[index];
		if (!cp_report_finite(input_sample) ||
		    !cp_report_finite(output_sample)) {
			metrics->finite = 0;
			continue;
		}
		metrics->input_square_sum +=
		    (double)input_sample * (double)input_sample;
		metrics->output_square_sum +=
		    (double)output_sample * (double)output_sample;
		metrics->input_sum += input_sample;
		metrics->output_sum += output_sample;
		if (!metrics->have_samples ||
		    cp_report_abs(input_sample) > metrics->input_peak)
			metrics->input_peak =
			    (cp_sample_t)cp_report_abs(input_sample);
		if (!metrics->have_samples ||
		    cp_report_abs(output_sample) > metrics->output_peak)
			metrics->output_peak =
			    (cp_sample_t)cp_report_abs(output_sample);
		if (!metrics->have_samples || output_sample < metrics->output_min)
			metrics->output_min = output_sample;
		if (!metrics->have_samples || output_sample > metrics->output_max)
			metrics->output_max = output_sample;
		metrics->have_samples = 1;
	}

	metrics->frames += (uint64_t)frames;
	metrics->samples += (uint64_t)samples;
	metrics->channels = channels;

	return CP_REPORT_OK;
}

const char *
cp_report_status_string(int status)
{
	switch (status) {
	case CP_REPORT_OK:
		return "ok";
	case CP_REPORT_ERR_NULL:
		return "missing report argument";
	case CP_REPORT_ERR_RANGE:
		return "invalid report range";
	case CP_REPORT_ERR_OPEN:
		return "could not open report file";
	case CP_REPORT_ERR_WRITE:
		return "could not write report file";
	default:
		return "unknown report error";
	}
}

int
cp_report_write_processed_file_json(const char *path,
	const struct cp_report_processed_file *report)
{
	FILE *file;
	int status;

	if (path == NULL || report == NULL || report->metrics == NULL)
		return CP_REPORT_ERR_NULL;

	file = fopen(path, "w");
	if (file == NULL)
		return CP_REPORT_ERR_OPEN;

	fprintf(file, "{\n");
	fprintf(file, "  \"carrierpress_report\": \"processed_file\",\n");
	fprintf(file, "  \"schema_version\": %d,\n", CP_REPORT_SCHEMA_VERSION);
	fprintf(file, "  \"version\": ");
	cp_report_json_string(file, CP_VERSION_STRING);
	fprintf(file, ",\n");
	cp_report_json_string_field(file, "input", report->input_path, 1);
	cp_report_json_string_field(file, "output", report->output_path, 1);
	fprintf(file, "  \"sample_rate_hz\": %zu,\n", report->sample_rate_hz);
	fprintf(file, "  \"channels\": %zu,\n", report->channels);
	fprintf(file, "  \"frames\": %llu,\n",
	    (unsigned long long)report->frames);
	fprintf(file, "  \"status\": \"ok\",\n");
	fprintf(file, "  \"profile\": {\n");
	cp_report_json_string_field(file, "path", report->profile_path, 1);
	cp_report_json_string_field(file, "name", report->profile_name, 0);
	fprintf(file, "  },\n");
	status = cp_report_write_metrics(file, report);
	if (status != CP_REPORT_OK) {
		fclose(file);
		return status;
	}
	fprintf(file, ",\n");
	status = cp_report_write_stages(file, report->block_config);
	if (status != CP_REPORT_OK) {
		fclose(file);
		return status;
	}
	fprintf(file, "\n");
	fprintf(file, "}\n");

	if (ferror(file) || fclose(file) != 0)
		return CP_REPORT_ERR_WRITE;

	return CP_REPORT_OK;
}

static double
cp_report_abs(double value)
{
	return value < 0.0 ? -value : value;
}

static double
cp_report_crest(double peak, double rms)
{
	if (rms <= 0.0 || !isfinite(peak) || !isfinite(rms))
		return 0.0;

	return peak / rms;
}

static double
cp_report_dc(double sum, uint64_t samples)
{
	if (samples == 0)
		return 0.0;

	return cp_report_abs(sum / (double)samples);
}

static int
cp_report_finite(cp_sample_t sample)
{
	return isfinite((double)sample);
}

static void
cp_report_json_bool(FILE *file, const char *key, int value, int comma)
{
	fprintf(file, "    ");
	cp_report_json_string(file, key);
	fprintf(file, ": %s%s\n", value ? "true" : "false",
	    comma ? "," : "");
}

static void
cp_report_json_double(FILE *file, const char *key, double value, int comma)
{
	fprintf(file, "    ");
	cp_report_json_string(file, key);
	if (isfinite(value))
		fprintf(file, ": %.9g%s\n", value, comma ? "," : "");
	else
		fprintf(file, ": null%s\n", comma ? "," : "");
}

static void
cp_report_json_null_string(FILE *file)
{
	fprintf(file, "null");
}

static void
cp_report_json_string(FILE *file, const char *text)
{
	const unsigned char *cursor;

	if (text == NULL) {
		cp_report_json_null_string(file);
		return;
	}

	fputc('"', file);
	for (cursor = (const unsigned char *)text; *cursor != '\0'; cursor++) {
		switch (*cursor) {
		case '"':
			fputs("\\\"", file);
			break;
		case '\\':
			fputs("\\\\", file);
			break;
		case '\b':
			fputs("\\b", file);
			break;
		case '\f':
			fputs("\\f", file);
			break;
		case '\n':
			fputs("\\n", file);
			break;
		case '\r':
			fputs("\\r", file);
			break;
		case '\t':
			fputs("\\t", file);
			break;
		default:
			if (*cursor < 0x20)
				fprintf(file, "\\u%04x", (unsigned int)*cursor);
			else
				fputc((int)*cursor, file);
			break;
		}
	}
	fputc('"', file);
}

static void
cp_report_json_string_field(FILE *file, const char *key, const char *value,
	int comma)
{
	fprintf(file, "    ");
	cp_report_json_string(file, key);
	fprintf(file, ": ");
	cp_report_json_string(file, value);
	fprintf(file, "%s\n", comma ? "," : "");
}

static double
cp_report_rms(double square_sum, uint64_t samples)
{
	if (samples == 0 || square_sum <= 0.0)
		return 0.0;

	return sqrt(square_sum / (double)samples);
}

static int
cp_report_write_metrics(FILE *file,
	const struct cp_report_processed_file *report)
{
	const struct cp_auto_eq_metrics *auto_eq;
	const struct cp_report_metrics *metrics;
	const struct cp_restoration_metrics *restoration;
	double input_rms;
	double output_rms;

	if (file == NULL || report == NULL || report->metrics == NULL)
		return CP_REPORT_ERR_NULL;

	metrics = report->metrics;
	restoration = report->restoration_metrics;
	auto_eq = report->auto_eq_metrics;
	input_rms = cp_report_rms(metrics->input_square_sum,
	    metrics->samples);
	output_rms = cp_report_rms(metrics->output_square_sum,
	    metrics->samples);

	fprintf(file, "  \"metrics\": {\n");
	cp_report_json_double(file, "input_rms", input_rms, 1);
	cp_report_json_double(file, "output_rms", output_rms, 1);
	cp_report_json_double(file, "input_peak", metrics->input_peak, 1);
	cp_report_json_double(file, "output_peak", metrics->output_peak, 1);
	cp_report_json_double(file, "input_crest",
	    cp_report_crest(metrics->input_peak, input_rms), 1);
	cp_report_json_double(file, "output_crest",
	    cp_report_crest(metrics->output_peak, output_rms), 1);
	cp_report_json_double(file, "input_dc",
	    cp_report_dc(metrics->input_sum, metrics->samples), 1);
	cp_report_json_double(file, "output_dc",
	    cp_report_dc(metrics->output_sum, metrics->samples), 1);
	cp_report_json_double(file, "output_min", metrics->output_min, 1);
	cp_report_json_double(file, "output_max", metrics->output_max, 1);
	cp_report_json_bool(file, "finite", metrics->finite, 1);
	if (restoration != NULL) {
		cp_report_json_double(file, "analysis_clip_ratio",
		    restoration->clipped_sample_ratio, 1);
		cp_report_json_double(file, "analysis_hf_ratio",
		    restoration->high_frequency_ratio, 1);
		cp_report_json_double(file, "analysis_clip_confidence",
		    restoration->clipping_confidence, 1);
		cp_report_json_double(file,
		    "analysis_low_ceiling_confidence",
		    restoration->low_ceiling_clipping_confidence, 1);
		cp_report_json_double(file, "analysis_transient_confidence",
		    restoration->transient_confidence, 1);
		cp_report_json_double(file, "analysis_lossy_confidence",
		    restoration->lossy_confidence, 1);
		cp_report_json_double(file, "analysis_flat_ratio",
		    restoration->flat_run_ratio, 1);
		cp_report_json_double(file, "analysis_peak_repeat_ratio",
		    restoration->peak_repeat_ratio, 1);
		cp_report_json_double(file, "analysis_peak",
		    restoration->observed_peak, 1);
		cp_report_json_double(file, "analysis_crest",
		    restoration->crest_factor, 1);
		fprintf(file, "    \"analysis_profile\": ");
		cp_report_json_string(file,
		    cp_restoration_source_profile_string(
		    restoration->source_profile));
		fprintf(file, ",\n");
		fprintf(file, "    \"analysis_reason_flags\": %u,\n",
		    restoration->reason_flags);
	}
	if (auto_eq != NULL) {
		fprintf(file, "    \"auto_eq_source\": ");
		cp_report_json_string(file,
		    cp_auto_eq_source_hint_string(auto_eq->source_hint));
		fprintf(file, ",\n");
		cp_report_json_double(file, "auto_eq_rms",
		    auto_eq->total_rms, 1);
		cp_report_json_double(file, "auto_eq_tilt_db",
		    auto_eq->spectral_tilt_db, 1);
		cp_report_json_double(file, "auto_eq_low",
		    auto_eq->low_frequency_weight, 1);
		cp_report_json_double(file, "auto_eq_presence",
		    auto_eq->presence_weight, 1);
		cp_report_json_double(file, "auto_eq_high",
		    auto_eq->high_frequency_weight, 1);
	}
	cp_report_json_double(file, "sample_count",
	    (double)metrics->samples, 0);
	fprintf(file, "  }");

	return ferror(file) ? CP_REPORT_ERR_WRITE : CP_REPORT_OK;
}

static int
cp_report_write_stages(FILE *file, const struct cp_block_config *config)
{
	if (file == NULL)
		return CP_REPORT_ERR_NULL;

	fprintf(file, "  \"stages\": {\n");
	cp_report_json_bool(file, "dehummer",
	    config != NULL && config->dehummer_enabled, 1);
	cp_report_json_bool(file, "restoration_analysis",
	    config != NULL && config->restoration_config.enabled, 1);
	cp_report_json_bool(file, "declipper",
	    config != NULL && config->declipper_config.enabled, 1);
	cp_report_json_bool(file, "natural_dynamics",
	    config != NULL && config->natural_dynamics_config.enabled, 1);
	cp_report_json_bool(file, "low_level_boost",
	    config != NULL && config->low_level_boost_config.enabled, 1);
	cp_report_json_bool(file, "multiband",
	    config != NULL && config->multiband_enabled, 1);
	cp_report_json_bool(file, "multiband2",
	    config != NULL && config->multiband2_enabled, 1);
	cp_report_json_bool(file, "bass_eq",
	    config != NULL && config->bass_eq_config.enabled, 1);
	cp_report_json_bool(file, "am",
	    config != NULL && config->am_config.enabled, 1);
	cp_report_json_bool(file, "ssb",
	    config != NULL && config->ssb_config.enabled, 1);
	cp_report_json_bool(file, "limiter", 1, 0);
	fprintf(file, "  }");

	return ferror(file) ? CP_REPORT_ERR_WRITE : CP_REPORT_OK;
}

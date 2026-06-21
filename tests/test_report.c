/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_report.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "cp_block.h"
#include "cp_report.h"

#define TEST_REPORT_PATH	"build/tests/report_processed.json"
#define TEST_REPORT_BUFSIZ	8192

static int	read_file(const char *, char *, size_t);
static int	test_metrics_mono(void);
static int	test_metrics_nonfinite(void);
static int	test_metrics_stereo(void);
static int	test_write_json(void);

int
main(void)
{
	if (!test_metrics_mono())
		return 1;
	if (!test_metrics_stereo())
		return 1;
	if (!test_metrics_nonfinite())
		return 1;
	if (!test_write_json())
		return 1;

	return 0;
}

static int
read_file(const char *path, char *buffer, size_t buffer_size)
{
	FILE *file;
	size_t count;

	if (path == NULL || buffer == NULL || buffer_size == 0)
		return 0;

	file = fopen(path, "r");
	if (file == NULL)
		return 0;

	count = fread(buffer, 1, buffer_size - 1, file);
	buffer[count] = '\0';
	if (ferror(file)) {
		fclose(file);
		return 0;
	}
	fclose(file);

	return 1;
}

static int
test_metrics_mono(void)
{
	cp_sample_t input[] = { 0.50f, -0.50f, 0.25f, -0.25f };
	cp_sample_t output[] = { 0.25f, -0.25f, 0.125f, -0.125f };
	struct cp_report_metrics metrics;

	cp_report_metrics_init(&metrics);
	if (cp_report_metrics_update(&metrics, input, output, 4,
	    CP_CHANNELS_MONO) != CP_REPORT_OK) {
		printf("test_report: mono metrics update failed\n");
		return 0;
	}
	if (metrics.frames != 4 || metrics.samples != 4 ||
	    metrics.channels != CP_CHANNELS_MONO || !metrics.finite) {
		printf("test_report: mono counters failed\n");
		return 0;
	}
	if (fabsf(metrics.input_peak - 0.50f) > 0.0001f ||
	    fabsf(metrics.output_peak - 0.25f) > 0.0001f) {
		printf("test_report: mono peaks failed\n");
		return 0;
	}

	return 1;
}

static int
test_metrics_nonfinite(void)
{
	cp_sample_t input[] = { 0.0f, NAN };
	cp_sample_t output[] = { 0.0f, 0.0f };
	struct cp_report_metrics metrics;

	cp_report_metrics_init(&metrics);
	if (cp_report_metrics_update(&metrics, input, output, 2,
	    CP_CHANNELS_MONO) != CP_REPORT_OK) {
		printf("test_report: nonfinite metrics update failed\n");
		return 0;
	}
	if (metrics.finite) {
		printf("test_report: nonfinite not detected\n");
		return 0;
	}

	return 1;
}

static int
test_metrics_stereo(void)
{
	cp_sample_t input[] = { 0.20f, -0.20f, 0.40f, -0.40f };
	cp_sample_t output[] = { 0.10f, -0.10f, 0.20f, -0.20f };
	struct cp_report_metrics metrics;

	cp_report_metrics_init(&metrics);
	if (cp_report_metrics_update(&metrics, input, output, 2,
	    CP_CHANNELS_STEREO) != CP_REPORT_OK) {
		printf("test_report: stereo metrics update failed\n");
		return 0;
	}
	if (metrics.frames != 2 || metrics.samples != 4 ||
	    metrics.channels != CP_CHANNELS_STEREO || !metrics.finite) {
		printf("test_report: stereo counters failed\n");
		return 0;
	}
	if (fabsf(metrics.output_min + 0.20f) > 0.0001f ||
	    fabsf(metrics.output_max - 0.20f) > 0.0001f) {
		printf("test_report: stereo output bounds failed\n");
		return 0;
	}

	return 1;
}

static int
test_write_json(void)
{
	cp_sample_t input[] = { 0.20f, -0.20f, 0.40f, -0.40f };
	cp_sample_t output[] = { 0.10f, -0.10f, 0.20f, -0.20f };
	char buffer[TEST_REPORT_BUFSIZ];
	struct cp_block_config config;
	struct cp_report_metrics metrics;
	struct cp_report_processed_file report;

	cp_block_default_config(&config, CP_CHANNELS_STEREO);
	config.dehummer_enabled = 1;
	config.multiband_enabled = 1;

	cp_report_metrics_init(&metrics);
	if (cp_report_metrics_update(&metrics, input, output, 2,
	    CP_CHANNELS_STEREO) != CP_REPORT_OK) {
		printf("test_report: write metrics update failed\n");
		return 0;
	}

	(void)memset(&report, 0, sizeof(report));
	report.input_path = "input \"quoted\".wav";
	report.output_path = "output.wav";
	report.sample_rate_hz = 48000;
	report.channels = CP_CHANNELS_STEREO;
	report.frames = 2;
	report.metrics = &metrics;
	report.block_config = &config;

	if (cp_report_write_processed_file_json(TEST_REPORT_PATH,
	    &report) != CP_REPORT_OK) {
		printf("test_report: JSON write failed\n");
		return 0;
	}
	if (!read_file(TEST_REPORT_PATH, buffer, sizeof(buffer))) {
		printf("test_report: JSON read failed\n");
		return 0;
	}
	if (strstr(buffer, "\"carrierpress_report\": \"processed_file\"") ==
	    NULL || strstr(buffer, "\"status\": \"ok\"") == NULL ||
	    strstr(buffer, "\"schema_version\": 1") == NULL ||
	    strstr(buffer, "\"input_rms\"") == NULL ||
	    strstr(buffer, "\"output_rms\"") == NULL ||
	    strstr(buffer, "\"dehummer\": true") == NULL ||
	    strstr(buffer, "\"multiband\": true") == NULL ||
	    strstr(buffer, "input \\\"quoted\\\".wav") == NULL) {
		printf("test_report: JSON content failed\n");
		return 0;
	}
	if (strstr(buffer, "compliance") != NULL ||
	    strstr(buffer, "proof") != NULL) {
		printf("test_report: JSON wording failed\n");
		return 0;
	}

	return 1;
}

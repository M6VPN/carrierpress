/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_report_tool.c */

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>

#include "cp_report_tool.h"

#define TEST_REPORT_TOOL_DIR		"build/tests"
#define TEST_REPORT_TOOL_BUF_SIZE	16384

static int	read_file(const char *, char *, size_t);
static int	run_compare(const char *, const char *, double);
static int	test_compare_identical_processed(void);
static int	test_compare_large_delta(void);
static int	test_compare_mismatched_type(void);
static int	test_compare_quality(void);
static int	test_compare_small_delta(void);
static int	test_reject_missing_schema(void);
static int	test_reject_non_carrierpress(void);
static int	test_reject_unsupported_schema(void);
static int	test_summary_processed(void);
static int	test_summary_quality(void);
static int	test_summary_batch(void);
static int	write_file(const char *, const char *);

static const char processed_report_a[] =
	"{\n"
	"  \"carrierpress_report\": \"processed_file\",\n"
	"  \"schema_version\": 1,\n"
	"  \"version\": \"0.2.0\",\n"
	"  \"input\": \"input.wav\",\n"
	"  \"output\": \"output.wav\",\n"
	"  \"sample_rate_hz\": 48000,\n"
	"  \"channels\": 2,\n"
	"  \"frames\": 16,\n"
	"  \"status\": \"ok\",\n"
	"  \"metrics\": {\n"
	"    \"input_rms\": 0.100000,\n"
	"    \"output_rms\": 0.090000,\n"
	"    \"input_peak\": 0.500000,\n"
	"    \"output_peak\": 0.450000,\n"
	"    \"input_crest\": 5.000000,\n"
	"    \"output_crest\": 5.000000,\n"
	"    \"input_dc\": 0.001000,\n"
	"    \"output_dc\": 0.000500,\n"
	"    \"output_min\": -0.450000,\n"
	"    \"output_max\": 0.450000,\n"
	"    \"finite\": true,\n"
	"    \"sample_count\": 32\n"
	"  },\n"
	"  \"future_extra\": 123,\n"
	"  \"stages\": { \"limiter\": true }\n"
	"}\n";

static const char processed_report_b_small[] =
	"{\n"
	"  \"carrierpress_report\": \"processed_file\",\n"
	"  \"schema_version\": 1,\n"
	"  \"version\": \"0.2.0\",\n"
	"  \"input\": \"input.wav\",\n"
	"  \"output\": \"output.wav\",\n"
	"  \"sample_rate_hz\": 48000,\n"
	"  \"channels\": 2,\n"
	"  \"frames\": 16,\n"
	"  \"status\": \"ok\",\n"
	"  \"metrics\": {\n"
	"    \"input_rms\": 0.1000004,\n"
	"    \"output_rms\": 0.0900004,\n"
	"    \"input_peak\": 0.5000004,\n"
	"    \"output_peak\": 0.4500004,\n"
	"    \"input_crest\": 5.0000004,\n"
	"    \"output_crest\": 5.0000004,\n"
	"    \"input_dc\": 0.0010004,\n"
	"    \"output_dc\": 0.0005004,\n"
	"    \"output_min\": -0.4499996,\n"
	"    \"output_max\": 0.4500004,\n"
	"    \"finite\": true,\n"
	"    \"sample_count\": 32\n"
	"  },\n"
	"  \"stages\": { \"limiter\": true }\n"
	"}\n";

static const char processed_report_b_large[] =
	"{\n"
	"  \"carrierpress_report\": \"processed_file\",\n"
	"  \"schema_version\": 1,\n"
	"  \"version\": \"0.2.0\",\n"
	"  \"input\": \"input.wav\",\n"
	"  \"output\": \"output.wav\",\n"
	"  \"sample_rate_hz\": 48000,\n"
	"  \"channels\": 2,\n"
	"  \"frames\": 16,\n"
	"  \"status\": \"ok\",\n"
	"  \"metrics\": {\n"
	"    \"input_rms\": 0.200000,\n"
	"    \"output_rms\": 0.090000,\n"
	"    \"input_peak\": 0.500000,\n"
	"    \"output_peak\": 0.450000,\n"
	"    \"input_crest\": 5.000000,\n"
	"    \"output_crest\": 5.000000,\n"
	"    \"input_dc\": 0.001000,\n"
	"    \"output_dc\": 0.000500,\n"
	"    \"output_min\": -0.450000,\n"
	"    \"output_max\": 0.450000,\n"
	"    \"finite\": true,\n"
	"    \"sample_count\": 32\n"
	"  },\n"
	"  \"stages\": { \"limiter\": true }\n"
	"}\n";

static const char quality_report[] =
	"{\n"
	"  \"carrierpress_report\": \"quality\",\n"
	"  \"schema_version\": 1,\n"
	"  \"version\": \"0.2.0\",\n"
	"  \"sample_rate_hz\": 48000,\n"
	"  \"frames\": 8192,\n"
	"  \"channels\": 2,\n"
	"  \"status\": \"pass\",\n"
	"  \"cases\": [\n"
	"    {\n"
	"      \"profile\": \"default\",\n"
	"      \"fixture\": \"silence\",\n"
	"      \"check\": \"silence\",\n"
	"      \"status\": \"pass\",\n"
	"      \"reason\": \"ok\",\n"
	"      \"code\": 0,\n"
	"      \"metrics\": {\n"
	"        \"input_rms\": 0.0,\n"
	"        \"output_rms\": 0.0,\n"
	"        \"input_peak\": 0.0,\n"
	"        \"output_peak\": 0.0,\n"
	"        \"output_min\": 0.0,\n"
	"        \"output_max\": 0.0,\n"
	"        \"input_crest\": 0.0,\n"
	"        \"output_crest\": 0.0,\n"
	"        \"input_dc\": 0.0,\n"
	"        \"output_dc\": 0.0,\n"
	"        \"input_hum50\": 0.0,\n"
	"        \"output_hum50\": 0.0,\n"
	"        \"input_hum60\": 0.0,\n"
	"        \"output_hum60\": 0.0,\n"
	"        \"output_left_rms\": 0.0,\n"
	"        \"output_right_rms\": 0.0,\n"
	"        \"analysis_clip_ratio\": 0.0,\n"
	"        \"analysis_hf_ratio\": 0.0,\n"
	"        \"analysis_clip_confidence\": 0.0,\n"
	"        \"analysis_low_ceiling_confidence\": 0.0,\n"
	"        \"analysis_transient_confidence\": 0.0,\n"
	"        \"analysis_lossy_confidence\": 0.0,\n"
	"        \"analysis_flat_ratio\": 0.0,\n"
	"        \"analysis_peak_repeat_ratio\": 0.0,\n"
	"        \"analysis_peak\": 0.0,\n"
	"        \"analysis_crest\": 0.0,\n"
	"        \"analysis_reason_flags\": 0,\n"
	"        \"declipper_samples\": 0,\n"
	"        \"declipper_runs\": 0,\n"
	"        \"declipper_delta\": 0.0,\n"
	"        \"natural_gr_db\": 0.0,\n"
	"        \"low_boost_gain_db\": 0.0,\n"
	"        \"auto_eq_rms\": 0.0,\n"
	"        \"auto_eq_tilt_db\": 0.0,\n"
	"        \"auto_eq_low\": 0.0,\n"
	"        \"auto_eq_presence\": 0.0,\n"
	"        \"auto_eq_high\": 0.0,\n"
	"        \"bass_eq_recommend_low_db\": 0.0,\n"
	"        \"bass_eq_recommend_high_db\": 0.0,\n"
	"        \"bass_eq_recommend_output_db\": 0.0,\n"
	"        \"bass_eq_recommend_confidence\": 0.0\n"
	"      }\n"
	"    },\n"
	"    {\n"
	"      \"profile\": \"default\",\n"
	"      \"fixture\": \"speech\",\n"
	"      \"check\": \"speech\",\n"
	"      \"status\": \"fail\",\n"
	"      \"reason\": \"check\",\n"
	"      \"code\": 0,\n"
	"      \"metrics\": {\n"
	"        \"input_rms\": 0.1,\n"
	"        \"output_rms\": 0.1,\n"
	"        \"input_peak\": 0.2,\n"
	"        \"output_peak\": 0.2,\n"
	"        \"output_min\": -0.2,\n"
	"        \"output_max\": 0.2,\n"
	"        \"input_crest\": 2.0,\n"
	"        \"output_crest\": 2.0,\n"
	"        \"input_dc\": 0.0,\n"
	"        \"output_dc\": 0.0,\n"
	"        \"input_hum50\": 0.0,\n"
	"        \"output_hum50\": 0.0,\n"
	"        \"input_hum60\": 0.0,\n"
	"        \"output_hum60\": 0.0,\n"
	"        \"output_left_rms\": 0.1,\n"
	"        \"output_right_rms\": 0.1,\n"
	"        \"analysis_clip_ratio\": 0.0,\n"
	"        \"analysis_hf_ratio\": 0.0,\n"
	"        \"analysis_clip_confidence\": 0.0,\n"
	"        \"analysis_low_ceiling_confidence\": 0.0,\n"
	"        \"analysis_transient_confidence\": 0.0,\n"
	"        \"analysis_lossy_confidence\": 0.0,\n"
	"        \"analysis_flat_ratio\": 0.0,\n"
	"        \"analysis_peak_repeat_ratio\": 0.0,\n"
	"        \"analysis_peak\": 0.2,\n"
	"        \"analysis_crest\": 2.0,\n"
	"        \"analysis_reason_flags\": 0,\n"
	"        \"declipper_samples\": 0,\n"
	"        \"declipper_runs\": 0,\n"
	"        \"declipper_delta\": 0.0,\n"
	"        \"natural_gr_db\": 0.0,\n"
	"        \"low_boost_gain_db\": 0.0,\n"
	"        \"auto_eq_rms\": 0.0,\n"
	"        \"auto_eq_tilt_db\": 0.0,\n"
	"        \"auto_eq_low\": 0.0,\n"
	"        \"auto_eq_presence\": 0.0,\n"
	"        \"auto_eq_high\": 0.0,\n"
	"        \"bass_eq_recommend_low_db\": 0.0,\n"
	"        \"bass_eq_recommend_high_db\": 0.0,\n"
	"        \"bass_eq_recommend_output_db\": 0.0,\n"
	"        \"bass_eq_recommend_confidence\": 0.0\n"
	"      }\n"
	"    }\n"
	"  ]\n"
	"}\n";

static const char batch_summary_report[] =
	"{\n"
	"  \"carrierpress_report\": \"batch_summary\",\n"
	"  \"schema_version\": 1,\n"
	"  \"version\": \"0.3.0\",\n"
	"  \"status\": \"ok\",\n"
	"  \"batch_list\": \"batch.txt\",\n"
	"  \"output_dir\": \"processed\",\n"
	"  \"profile\": {\n"
	"    \"path\": \"profiles/file-cleanup.profile\",\n"
	"    \"name\": \"File Cleanup\"\n"
	"  },\n"
	"  \"planned\": 2,\n"
	"  \"processed\": 2,\n"
	"  \"failed\": 0,\n"
	"  \"skipped\": 1,\n"
	"  \"last_status\": 0,\n"
	"  \"items\": [\n"
	"    {\n"
	"      \"line\": 2,\n"
	"      \"input\": \"audio/a.wav\",\n"
	"      \"output\": \"processed/a.wav\",\n"
	"      \"report\": \"processed/a.report.json\",\n"
	"      \"status\": \"ok\",\n"
	"      \"reason\": null\n"
	"    }\n"
	"  ]\n"
	"}\n";

int
main(void)
{
	(void)mkdir(TEST_REPORT_TOOL_DIR, 0777);
	if (!test_summary_quality())
		return 1;
	if (!test_summary_processed())
		return 1;
	if (!test_summary_batch())
		return 1;
	if (!test_reject_missing_schema())
		return 1;
	if (!test_reject_unsupported_schema())
		return 1;
	if (!test_reject_non_carrierpress())
		return 1;
	if (!test_compare_mismatched_type())
		return 1;
	if (!test_compare_identical_processed())
		return 1;
	if (!test_compare_small_delta())
		return 1;
	if (!test_compare_large_delta())
		return 1;
	if (!test_compare_quality())
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
run_compare(const char *base_path, const char *new_path, double tolerance)
{
	FILE *out;
	int status;

	out = fopen(TEST_REPORT_TOOL_DIR "/compare-output.txt", "w");
	if (out == NULL)
		return CP_REPORT_TOOL_ERR_OPEN;
	status = cp_report_tool_compare_files(base_path, new_path, tolerance,
	    out);
	fclose(out);

	return status;
}

static int
test_compare_identical_processed(void)
{
	char buffer[TEST_REPORT_TOOL_BUF_SIZE];

	if (!write_file(TEST_REPORT_TOOL_DIR "/processed-a.json",
	    processed_report_a))
		return 0;
	if (run_compare(TEST_REPORT_TOOL_DIR "/processed-a.json",
	    TEST_REPORT_TOOL_DIR "/processed-a.json",
	    CP_REPORT_TOOL_DEFAULT_TOLERANCE) != CP_REPORT_TOOL_OK) {
		printf("test_report_tool: identical compare failed\n");
		return 0;
	}
	if (!read_file(TEST_REPORT_TOOL_DIR "/processed-a.json", buffer,
	    sizeof(buffer)))
		return 0;
	if (strstr(buffer, "compliance") != NULL ||
	    strstr(buffer, "proof") != NULL) {
		printf("test_report_tool: report wording failed\n");
		return 0;
	}

	return 1;
}

static int
test_compare_large_delta(void)
{
	if (!write_file(TEST_REPORT_TOOL_DIR "/processed-a.json",
	    processed_report_a) ||
	    !write_file(TEST_REPORT_TOOL_DIR "/processed-large.json",
	    processed_report_b_large))
		return 0;
	if (run_compare(TEST_REPORT_TOOL_DIR "/processed-a.json",
	    TEST_REPORT_TOOL_DIR "/processed-large.json",
	    CP_REPORT_TOOL_DEFAULT_TOLERANCE) !=
	    CP_REPORT_TOOL_ERR_COMPARE) {
		printf("test_report_tool: large delta compare failed\n");
		return 0;
	}

	return 1;
}

static int
test_compare_mismatched_type(void)
{
	if (!write_file(TEST_REPORT_TOOL_DIR "/processed-a.json",
	    processed_report_a) ||
	    !write_file(TEST_REPORT_TOOL_DIR "/quality.json", quality_report))
		return 0;
	if (run_compare(TEST_REPORT_TOOL_DIR "/processed-a.json",
	    TEST_REPORT_TOOL_DIR "/quality.json",
	    CP_REPORT_TOOL_DEFAULT_TOLERANCE) != CP_REPORT_TOOL_ERR_TYPE) {
		printf("test_report_tool: mismatched type compare failed\n");
		return 0;
	}

	return 1;
}

static int
test_compare_quality(void)
{
	if (!write_file(TEST_REPORT_TOOL_DIR "/quality.json", quality_report))
		return 0;
	if (run_compare(TEST_REPORT_TOOL_DIR "/quality.json",
	    TEST_REPORT_TOOL_DIR "/quality.json",
	    CP_REPORT_TOOL_DEFAULT_TOLERANCE) != CP_REPORT_TOOL_OK) {
		printf("test_report_tool: quality compare failed\n");
		return 0;
	}

	return 1;
}

static int
test_compare_small_delta(void)
{
	if (!write_file(TEST_REPORT_TOOL_DIR "/processed-a.json",
	    processed_report_a) ||
	    !write_file(TEST_REPORT_TOOL_DIR "/processed-small.json",
	    processed_report_b_small))
		return 0;
	if (run_compare(TEST_REPORT_TOOL_DIR "/processed-a.json",
	    TEST_REPORT_TOOL_DIR "/processed-small.json",
	    CP_REPORT_TOOL_DEFAULT_TOLERANCE) != CP_REPORT_TOOL_OK) {
		printf("test_report_tool: small delta compare failed\n");
		return 0;
	}

	return 1;
}

static int
test_reject_missing_schema(void)
{
	static const char report[] =
	    "{ \"carrierpress_report\": \"processed_file\", "
	    "\"version\": \"0.2.0\", \"status\": \"ok\" }\n";

	if (!write_file(TEST_REPORT_TOOL_DIR "/missing-schema.json", report))
		return 0;
	if (cp_report_tool_summary_file(TEST_REPORT_TOOL_DIR
	    "/missing-schema.json", stdout) != CP_REPORT_TOOL_ERR_SCHEMA) {
		printf("test_report_tool: missing schema not rejected\n");
		return 0;
	}

	return 1;
}

static int
test_reject_non_carrierpress(void)
{
	static const char report[] =
	    "{ \"schema_version\": 1, \"status\": \"ok\" }\n";

	if (!write_file(TEST_REPORT_TOOL_DIR "/not-carrierpress.json", report))
		return 0;
	if (cp_report_tool_summary_file(TEST_REPORT_TOOL_DIR
	    "/not-carrierpress.json", stdout) != CP_REPORT_TOOL_ERR_PARSE) {
		printf("test_report_tool: non-CarrierPress not rejected\n");
		return 0;
	}

	return 1;
}

static int
test_reject_unsupported_schema(void)
{
	static const char report[] =
	    "{ \"carrierpress_report\": \"processed_file\", "
	    "\"schema_version\": 99, \"version\": \"0.2.0\", "
	    "\"status\": \"ok\" }\n";

	if (!write_file(TEST_REPORT_TOOL_DIR "/bad-schema.json", report))
		return 0;
	if (cp_report_tool_summary_file(TEST_REPORT_TOOL_DIR
	    "/bad-schema.json", stdout) != CP_REPORT_TOOL_ERR_SCHEMA) {
		printf("test_report_tool: unsupported schema not rejected\n");
		return 0;
	}

	return 1;
}

static int
test_summary_processed(void)
{
	char buffer[TEST_REPORT_TOOL_BUF_SIZE];
	FILE *out;

	if (!write_file(TEST_REPORT_TOOL_DIR "/processed-a.json",
	    processed_report_a))
		return 0;
	out = fopen(TEST_REPORT_TOOL_DIR "/summary-processed.txt", "w");
	if (out == NULL)
		return 0;
	if (cp_report_tool_summary_file(TEST_REPORT_TOOL_DIR
	    "/processed-a.json", out) != CP_REPORT_TOOL_OK) {
		fclose(out);
		return 0;
	}
	fclose(out);
	if (!read_file(TEST_REPORT_TOOL_DIR "/summary-processed.txt", buffer,
	    sizeof(buffer)))
		return 0;
	if (strstr(buffer, "report=processed_file") == NULL ||
	    strstr(buffer, "schema_version=1") == NULL ||
	    strstr(buffer, "input_rms=0.1") == NULL ||
	    strstr(buffer, "output_peak=0.45") == NULL ||
	    strstr(buffer, "compliance") != NULL ||
	    strstr(buffer, "proof") != NULL) {
		printf("test_report_tool: processed summary failed\n");
		return 0;
	}

	return 1;
}

static int
test_summary_quality(void)
{
	char buffer[TEST_REPORT_TOOL_BUF_SIZE];
	FILE *out;

	if (!write_file(TEST_REPORT_TOOL_DIR "/quality.json", quality_report))
		return 0;
	out = fopen(TEST_REPORT_TOOL_DIR "/summary-quality.txt", "w");
	if (out == NULL)
		return 0;
	if (cp_report_tool_summary_file(TEST_REPORT_TOOL_DIR "/quality.json",
	    out) != CP_REPORT_TOOL_OK) {
		fclose(out);
		return 0;
	}
	fclose(out);
	if (!read_file(TEST_REPORT_TOOL_DIR "/summary-quality.txt", buffer,
	    sizeof(buffer)))
		return 0;
	if (strstr(buffer, "report=quality") == NULL ||
	    strstr(buffer, "schema_version=1") == NULL ||
	    strstr(buffer, "cases=2") == NULL ||
	    strstr(buffer, "failed_cases=1") == NULL ||
	    strstr(buffer, "compliance") != NULL ||
	    strstr(buffer, "proof") != NULL) {
		printf("test_report_tool: quality summary failed\n");
		return 0;
	}

	return 1;
}

static int
test_summary_batch(void)
{
	char buffer[TEST_REPORT_TOOL_BUF_SIZE];
	FILE *out;
	int status;

	if (!write_file(TEST_REPORT_TOOL_DIR "/batch-summary.json",
	    batch_summary_report))
		return 0;
	out = fopen(TEST_REPORT_TOOL_DIR "/summary-batch.txt", "w");
	if (out == NULL)
		return 0;
	status = cp_report_tool_summary_file(TEST_REPORT_TOOL_DIR
	    "/batch-summary.json", out);
	if (status != CP_REPORT_TOOL_OK) {
		fclose(out);
		printf("test_report_tool: batch summary status failed: %d\n",
		    status);
		return 0;
	}
	fclose(out);
	if (!read_file(TEST_REPORT_TOOL_DIR "/summary-batch.txt", buffer,
	    sizeof(buffer)))
		return 0;
	if (strstr(buffer, "report=batch_summary") == NULL ||
	    strstr(buffer, "schema_version=1") == NULL ||
	    strstr(buffer, "planned=2") == NULL ||
	    strstr(buffer, "processed=2") == NULL ||
	    strstr(buffer, "failed=0") == NULL ||
	    strstr(buffer, "compliance") != NULL ||
	    strstr(buffer, "proof") != NULL) {
		printf("test_report_tool: batch summary failed\n");
		return 0;
	}

	return 1;
}

static int
write_file(const char *path, const char *text)
{
	FILE *file;

	if (path == NULL || text == NULL)
		return 0;
	file = fopen(path, "w");
	if (file == NULL)
		return 0;
	fputs(text, file);
	if (ferror(file)) {
		fclose(file);
		return 0;
	}
	fclose(file);

	return 1;
}

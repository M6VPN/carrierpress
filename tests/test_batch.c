/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_batch.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_batch.h"

#define TEST_BATCH_DIR	"build/tests/base"

static int	write_long_batch(const char *);
static int	write_text_file(const char *, const char *);
static int	test_allow_overwrite(void);
static int	test_duplicate_outputs(void);
static int	test_existing_output(void);
static int	test_existing_report(void);
static int	test_missing_file(void);
static int	test_path_filter(void);
static int	test_too_long_line(void);
static int	test_unsupported_formats(void);
static int	test_valid_list(void);

int
main(void)
{
	if (!test_path_filter())
		return 1;
	if (!test_valid_list())
		return 1;
	if (!test_unsupported_formats())
		return 1;
	if (!test_too_long_line())
		return 1;
	if (!test_duplicate_outputs())
		return 1;
	if (!test_existing_output())
		return 1;
	if (!test_existing_report())
		return 1;
	if (!test_allow_overwrite())
		return 1;
	if (!test_missing_file())
		return 1;

	return 0;
}

static int
write_long_batch(const char *path)
{
	FILE *file;
	size_t index;

	file = fopen(path, "w");
	if (file == NULL)
		return 0;
	for (index = 0; index < CP_BATCH_MAX_LINE + 8; index++) {
		if (fputc('a', file) == EOF) {
			fclose(file);
			return 0;
		}
	}
	if (fputc('\n', file) == EOF) {
		fclose(file);
		return 0;
	}
	fclose(file);

	return 1;
}

static int
write_text_file(const char *path, const char *text)
{
	FILE *file;

	file = fopen(path, "w");
	if (file == NULL)
		return 0;
	if (fputs(text, file) == EOF) {
		fclose(file);
		return 0;
	}
	fclose(file);

	return 1;
}

static int
test_allow_overwrite(void)
{
	struct cp_batch_plan plan;
	const char *batch_path;
	const char *output_path;

	batch_path = TEST_BATCH_DIR "/batch_allow_overwrite.txt";
	output_path = TEST_BATCH_DIR "/allow.wav";
	if (!write_text_file(batch_path, "input/allow.wav\n") ||
	    !write_text_file(output_path, "placeholder\n")) {
		printf("test_batch: allow-overwrite fixture write failed\n");
		return 0;
	}
	if (cp_batch_plan_load(batch_path, TEST_BATCH_DIR, &plan, NULL) !=
	    CP_BATCH_OK ||
	    cp_batch_plan_check_overwrites(&plan, 1) != CP_BATCH_OK ||
	    plan.warnings != 1 || plan.errors != 0 ||
	    plan.planned_items != 1) {
		printf("test_batch: allow-overwrite did not warn cleanly\n");
		return 0;
	}

	return 1;
}

static int
test_duplicate_outputs(void)
{
	struct cp_batch_plan plan;
	const char *batch_path;

	batch_path = TEST_BATCH_DIR "/batch_duplicates.txt";
	if (!write_text_file(batch_path, "a/dup.wav\nb/dup.wav\n")) {
		printf("test_batch: duplicate fixture write failed\n");
		return 0;
	}
	if (cp_batch_plan_load(batch_path, TEST_BATCH_DIR, &plan, NULL) !=
	    CP_BATCH_ERR_BAD ||
	    plan.errors != 1 || plan.planned_items != 1 ||
	    strcmp(plan.items[1].reason, "duplicate output path") != 0) {
		printf("test_batch: duplicate output not detected\n");
		return 0;
	}

	return 1;
}

static int
test_existing_output(void)
{
	struct cp_batch_plan plan;
	const char *batch_path;
	const char *output_path;

	batch_path = TEST_BATCH_DIR "/batch_existing_output.txt";
	output_path = TEST_BATCH_DIR "/existing.wav";
	if (!write_text_file(batch_path, "input/existing.wav\n") ||
	    !write_text_file(output_path, "placeholder\n")) {
		printf("test_batch: existing-output fixture write failed\n");
		return 0;
	}
	if (cp_batch_plan_load(batch_path, TEST_BATCH_DIR, &plan, NULL) !=
	    CP_BATCH_OK ||
	    cp_batch_plan_check_overwrites(&plan, 0) != CP_BATCH_ERR_BAD ||
	    plan.errors != 1 ||
	    strcmp(plan.items[0].reason, "output already exists") != 0) {
		printf("test_batch: existing output not detected\n");
		return 0;
	}

	return 1;
}

static int
test_existing_report(void)
{
	struct cp_batch_plan plan;
	const char *batch_path;
	const char *report_path;

	batch_path = TEST_BATCH_DIR "/batch_existing_report.txt";
	report_path = TEST_BATCH_DIR "/report.report.json";
	if (!write_text_file(batch_path, "input/report.wav\n") ||
	    !write_text_file(report_path, "placeholder\n")) {
		printf("test_batch: existing-report fixture write failed\n");
		return 0;
	}
	if (cp_batch_plan_load(batch_path, TEST_BATCH_DIR, &plan, NULL) !=
	    CP_BATCH_OK ||
	    cp_batch_plan_check_overwrites(&plan, 0) != CP_BATCH_ERR_BAD ||
	    plan.errors != 1 ||
	    strcmp(plan.items[0].reason, "report already exists") != 0) {
		printf("test_batch: existing report not detected\n");
		return 0;
	}

	return 1;
}

static int
test_missing_file(void)
{
	struct cp_batch_plan plan;
	struct cp_batch_error error;

	if (cp_batch_plan_load(TEST_BATCH_DIR "/missing-batch.txt",
	    TEST_BATCH_DIR, &plan, &error) != CP_BATCH_ERR_OPEN ||
	    plan.errors != 1 ||
	    strcmp(error.reason, "could not open batch list") != 0) {
		printf("test_batch: missing batch file accepted\n");
		return 0;
	}

	return 1;
}

static int
test_path_filter(void)
{
	if (!cp_batch_path_is_supported_input("track.wav"))
		return 0;
	if (!cp_batch_path_is_supported_input("TRACK.WAV"))
		return 0;
	if (cp_batch_path_is_supported_input("track.mp3"))
		return 0;
	if (cp_batch_path_is_supported_input("wav"))
		return 0;
	if (strcmp(cp_batch_status_string(CP_BATCH_ERR_BAD),
	    "batch has errors") != 0)
		return 0;

	return 1;
}

static int
test_too_long_line(void)
{
	struct cp_batch_plan plan;
	const char *batch_path;

	batch_path = TEST_BATCH_DIR "/batch_too_long.txt";
	if (!write_long_batch(batch_path)) {
		printf("test_batch: long fixture write failed\n");
		return 0;
	}
	if (cp_batch_plan_load(batch_path, TEST_BATCH_DIR, &plan, NULL) !=
	    CP_BATCH_ERR_BAD ||
	    plan.total_lines != 1 || plan.errors != 1 ||
	    strcmp(plan.items[0].reason, "batch line is too long") != 0) {
		printf("test_batch: long line not reported\n");
		return 0;
	}

	return 1;
}

static int
test_unsupported_formats(void)
{
	struct cp_batch_plan plan;
	const char *batch_path;

	batch_path = TEST_BATCH_DIR "/batch_unsupported.txt";
	if (!write_text_file(batch_path, "a.mp3\nb.flac\nc.ogg\n"
	    "d.opus\ne.m4a\n")) {
		printf("test_batch: unsupported fixture write failed\n");
		return 0;
	}
	if (cp_batch_plan_load(batch_path, TEST_BATCH_DIR, &plan, NULL) !=
	    CP_BATCH_ERR_BAD ||
	    plan.errors != 5 || plan.planned_items != 0 ||
	    strcmp(plan.items[0].reason,
	    "unsupported format: convert to WAV first") != 0) {
		printf("test_batch: unsupported formats not reported\n");
		return 0;
	}

	return 1;
}

static int
test_valid_list(void)
{
	struct cp_batch_plan plan;
	const char *batch_path;

	batch_path = TEST_BATCH_DIR "/batch_valid.txt";
	if (!write_text_file(batch_path, "\n# comment\naudio/intro.wav\n"
	    "audio/TWO.WAV\n")) {
		printf("test_batch: valid fixture write failed\n");
		return 0;
	}
	if (cp_batch_plan_load(batch_path, TEST_BATCH_DIR, &plan, NULL) !=
	    CP_BATCH_OK ||
	    plan.total_lines != 4 || plan.skipped_items != 2 ||
	    plan.planned_items != 2 || plan.errors != 0) {
		printf("test_batch: valid list summary mismatch\n");
		return 0;
	}
	if (strcmp(plan.items[2].output_path,
	    TEST_BATCH_DIR "/intro.wav") != 0 ||
	    strcmp(plan.items[2].report_path,
	    TEST_BATCH_DIR "/intro.report.json") != 0 ||
	    strcmp(plan.items[3].output_path,
	    TEST_BATCH_DIR "/TWO.WAV") != 0 ||
	    strcmp(plan.items[3].report_path,
	    TEST_BATCH_DIR "/TWO.report.json") != 0) {
		printf("test_batch: planned paths mismatch\n");
		return 0;
	}

	return 1;
}

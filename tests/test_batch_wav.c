/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_batch_wav.c */

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>

#include <sndfile.h>

#include "cp_batch.h"
#include "cp_batch_wav.h"
#include "cp_block.h"

#define TEST_BATCH_WAV_BLOCK	8
#define TEST_BATCH_WAV_BUFSIZ	8192
#define TEST_BATCH_WAV_CHANNELS	2
#define TEST_BATCH_WAV_FRAMES	32
#define TEST_BATCH_WAV_RATE	48000
#define TEST_BATCH_WAV_DIR	"build/tests/base-sndfile"
#define TEST_BATCH_WAV_OUTDIR	TEST_BATCH_WAV_DIR "/batch-out"
#define TEST_BATCH_WAV_CLIOUT	TEST_BATCH_WAV_DIR "/batch-cli-out"

static int	check_file_exists(const char *);
static int	check_report(const char *);
static int	ensure_dir(const char *);
static int	read_file(const char *, char *, size_t);
static int	test_batch_processing(void);
static int	test_batch_rejects_bad_plan(void);
static int	test_cli_fixture(void);
static int	write_fixture(const char *);
static int	write_text(const char *, const char *);

int
main(void)
{
	if (!ensure_dir(TEST_BATCH_WAV_OUTDIR) ||
	    !ensure_dir(TEST_BATCH_WAV_CLIOUT)) {
		printf("test_batch_wav: could not create scratch dirs\n");
		return 1;
	}
	if (!test_batch_processing())
		return 1;
	if (!test_batch_rejects_bad_plan())
		return 1;
	if (!test_cli_fixture())
		return 1;

	return 0;
}

static int
check_file_exists(const char *path)
{
	FILE *file;

	file = fopen(path, "rb");
	if (file == NULL)
		return 0;
	fclose(file);

	return 1;
}

static int
check_report(const char *path)
{
	char buffer[TEST_BATCH_WAV_BUFSIZ];

	if (!read_file(path, buffer, sizeof(buffer)))
		return 0;
	if (strstr(buffer,
	    "\"carrierpress_report\": \"processed_file\"") == NULL)
		return 0;
	if (strstr(buffer, "\"status\": \"ok\"") == NULL)
		return 0;
	if (strstr(buffer, "\"input_rms\"") == NULL ||
	    strstr(buffer, "\"output_rms\"") == NULL)
		return 0;
	if (strstr(buffer, "compliance") != NULL)
		return 0;

	return 1;
}

static int
ensure_dir(const char *path)
{
	if (mkdir(path, 0777) == -1) {
		struct stat st;

		if (stat(path, &st) == -1 || !S_ISDIR(st.st_mode))
			return 0;
	}

	return 1;
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
test_batch_processing(void)
{
	struct cp_batch_plan plan;
	struct cp_batch_error error;
	struct cp_batch_wav_result result;
	struct cp_block_config config;
	const char *input_a;
	const char *input_b;
	const char *list_path;
	int status;

	input_a = TEST_BATCH_WAV_DIR "/batch-a.wav";
	input_b = TEST_BATCH_WAV_DIR "/batch-b.wav";
	list_path = TEST_BATCH_WAV_DIR "/batch-good.txt";

	(void)remove(TEST_BATCH_WAV_OUTDIR "/batch-a.wav");
	(void)remove(TEST_BATCH_WAV_OUTDIR "/batch-b.wav");
	(void)remove(TEST_BATCH_WAV_OUTDIR "/batch-a.report.json");
	(void)remove(TEST_BATCH_WAV_OUTDIR "/batch-b.report.json");

	if (!write_fixture(input_a) || !write_fixture(input_b) ||
	    !write_text(list_path,
	    "# batch fixture\n"
	    "build/tests/base-sndfile/batch-a.wav\n"
	    "build/tests/base-sndfile/batch-b.wav\n")) {
		printf("test_batch_wav: fixture setup failed\n");
		return 0;
	}

	status = cp_batch_plan_load(list_path, TEST_BATCH_WAV_OUTDIR, &plan,
	    &error);
	if (status != CP_BATCH_OK ||
	    cp_batch_plan_check_overwrites(&plan, 0) != CP_BATCH_OK) {
		printf("test_batch_wav: plan failed\n");
		return 0;
	}
	cp_block_default_config(&config, TEST_BATCH_WAV_CHANNELS);
	status = cp_batch_wav_process_plan(&plan, &config,
	    TEST_BATCH_WAV_BLOCK, &result, NULL);
	if (status != CP_BATCH_OK || result.processed != 2 ||
	    result.failed != 0) {
		printf("test_batch_wav: process failed\n");
		return 0;
	}
	if (!check_file_exists(TEST_BATCH_WAV_OUTDIR "/batch-a.wav") ||
	    !check_file_exists(TEST_BATCH_WAV_OUTDIR "/batch-b.wav") ||
	    !check_report(TEST_BATCH_WAV_OUTDIR "/batch-a.report.json") ||
	    !check_report(TEST_BATCH_WAV_OUTDIR "/batch-b.report.json")) {
		printf("test_batch_wav: output/report missing\n");
		return 0;
	}

	status = cp_batch_plan_load(list_path, TEST_BATCH_WAV_OUTDIR, &plan,
	    &error);
	if (status != CP_BATCH_OK ||
	    cp_batch_plan_check_overwrites(&plan, 0) == CP_BATCH_OK) {
		printf("test_batch_wav: overwrite was not rejected\n");
		return 0;
	}
	status = cp_batch_plan_load(list_path, TEST_BATCH_WAV_OUTDIR, &plan,
	    &error);
	if (status != CP_BATCH_OK ||
	    cp_batch_plan_check_overwrites(&plan, 1) != CP_BATCH_OK ||
	    cp_batch_wav_process_plan(&plan, &config, TEST_BATCH_WAV_BLOCK,
	    &result, NULL) != CP_BATCH_OK) {
		printf("test_batch_wav: overwrite process failed\n");
		return 0;
	}

	return 1;
}

static int
test_batch_rejects_bad_plan(void)
{
	struct cp_batch_plan plan;
	struct cp_batch_error error;
	const char *bad_list;
	const char *dup_list;
	int status;

	bad_list = TEST_BATCH_WAV_DIR "/batch-bad.txt";
	dup_list = TEST_BATCH_WAV_DIR "/batch-dup.txt";
	if (!write_text(bad_list,
	    "build/tests/base-sndfile/batch-a.wav\n"
	    "music.mp3\n") ||
	    !write_text(dup_list,
	    "dir1/dup.wav\n"
	    "dir2/dup.wav\n")) {
		printf("test_batch_wav: bad fixture write failed\n");
		return 0;
	}

	status = cp_batch_plan_load(bad_list, TEST_BATCH_WAV_OUTDIR, &plan,
	    &error);
	if (status == CP_BATCH_OK || plan.errors == 0) {
		printf("test_batch_wav: unsupported format accepted\n");
		return 0;
	}
	status = cp_batch_plan_load(dup_list, TEST_BATCH_WAV_OUTDIR, &plan,
	    &error);
	if (status == CP_BATCH_OK || plan.errors == 0) {
		printf("test_batch_wav: duplicate output accepted\n");
		return 0;
	}

	return 1;
}

static int
test_cli_fixture(void)
{
	const char *input_a;
	const char *input_b;
	const char *list_path;

	input_a = TEST_BATCH_WAV_DIR "/cli-a.wav";
	input_b = TEST_BATCH_WAV_DIR "/cli-b.wav";
	list_path = TEST_BATCH_WAV_DIR "/batch-cli.txt";

	(void)remove(TEST_BATCH_WAV_CLIOUT "/cli-a.wav");
	(void)remove(TEST_BATCH_WAV_CLIOUT "/cli-b.wav");
	(void)remove(TEST_BATCH_WAV_CLIOUT "/cli-a.report.json");
	(void)remove(TEST_BATCH_WAV_CLIOUT "/cli-b.report.json");

	if (!write_fixture(input_a) || !write_fixture(input_b))
		return 0;
	if (!write_text(list_path,
	    "build/tests/base-sndfile/cli-a.wav\n"
	    "build/tests/base-sndfile/cli-b.wav\n"))
		return 0;

	return 1;
}

static int
write_fixture(const char *path)
{
	SF_INFO info;
	SNDFILE *file;
	float samples[TEST_BATCH_WAV_FRAMES * TEST_BATCH_WAV_CHANNELS];
	size_t index;
	sf_count_t written;

	memset(&info, 0, sizeof(info));
	info.channels = TEST_BATCH_WAV_CHANNELS;
	info.frames = TEST_BATCH_WAV_FRAMES;
	info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
	info.samplerate = TEST_BATCH_WAV_RATE;

	for (index = 0;
	    index < TEST_BATCH_WAV_FRAMES * TEST_BATCH_WAV_CHANNELS;
	    index++) {
		samples[index] = (index % 2 == 0) ? 0.12f : -0.08f;
	}

	file = sf_open(path, SFM_WRITE, &info);
	if (file == NULL)
		return 0;
	written = sf_writef_float(file, samples, TEST_BATCH_WAV_FRAMES);
	sf_close(file);

	return written == TEST_BATCH_WAV_FRAMES;
}

static int
write_text(const char *path, const char *text)
{
	FILE *file;

	if (path == NULL || text == NULL)
		return 0;
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

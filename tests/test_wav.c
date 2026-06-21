/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_wav.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include <sndfile.h>

#include "cp_wav.h"

#define TEST_WAV_BLOCK		8
#define TEST_WAV_BUFSIZ		8192
#define TEST_WAV_CHANNELS	2
#define TEST_WAV_FRAMES		32
#define TEST_WAV_RATE		48000

static int	read_report(const char *, char *, size_t);
static int	write_fixture(const char *);

int
main(void)
{
	SF_INFO info;
	SNDFILE *file;
	const char *input_path;
	const char *output_path;
	const char *report_path;
	char report_buffer[TEST_WAV_BUFSIZ];
	int status;

	input_path  = "build/tests/wav_input.wav";
	output_path = "build/tests/wav_output.wav";
	report_path = "build/tests/wav_output.report.json";

	if (!write_fixture(input_path)) {
		printf("test_wav: fixture write failed\n");
		return 1;
	}

	status = cp_wav_process_file(input_path, output_path, TEST_WAV_BLOCK);
	if (status != CP_WAV_OK) {
		printf("test_wav: process failed: %s\n",
		    cp_wav_status_string(status));
		return 1;
	}

	memset(&info, 0, sizeof(info));
	file = sf_open(output_path, SFM_READ, &info);
	if (file == NULL) {
		printf("test_wav: output open failed\n");
		return 1;
	}
	sf_close(file);

	if (info.frames != TEST_WAV_FRAMES) {
		printf("test_wav: frame count mismatch\n");
		return 1;
	}
	if (info.samplerate != TEST_WAV_RATE) {
		printf("test_wav: sample rate mismatch\n");
		return 1;
	}
	if (info.channels != TEST_WAV_CHANNELS) {
		printf("test_wav: channel count mismatch\n");
		return 1;
	}

	status = cp_wav_process_file_config_full_sidecar_report(input_path,
	    output_path, TEST_WAV_BLOCK, NULL, NULL, report_path);
	if (status != CP_WAV_OK) {
		printf("test_wav: report process failed: %s\n",
		    cp_wav_status_string(status));
		return 1;
	}
	if (!read_report(report_path, report_buffer, sizeof(report_buffer))) {
		printf("test_wav: report read failed\n");
		return 1;
	}
	if (strstr(report_buffer,
	    "\"carrierpress_report\": \"processed_file\"") == NULL ||
	    strstr(report_buffer, "\"status\": \"ok\"") == NULL ||
	    strstr(report_buffer, "\"input_rms\"") == NULL ||
	    strstr(report_buffer, "\"output_rms\"") == NULL ||
	    strstr(report_buffer, "compliance") != NULL) {
		printf("test_wav: report content mismatch\n");
		return 1;
	}

	return 0;
}

static int
read_report(const char *path, char *buffer, size_t buffer_size)
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
write_fixture(const char *path)
{
	SF_INFO info;
	SNDFILE *file;
	float samples[TEST_WAV_FRAMES * TEST_WAV_CHANNELS];
	size_t index;
	sf_count_t written;

	memset(&info, 0, sizeof(info));
	info.channels   = TEST_WAV_CHANNELS;
	info.frames     = TEST_WAV_FRAMES;
	info.format     = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
	info.samplerate = TEST_WAV_RATE;

	for (index = 0; index < TEST_WAV_FRAMES * TEST_WAV_CHANNELS; index++)
		samples[index] = (index % 2 == 0) ? 0.10f : -0.10f;

	file = sf_open(path, SFM_WRITE, &info);
	if (file == NULL)
		return 0;

	written = sf_writef_float(file, samples, TEST_WAV_FRAMES);
	sf_close(file);

	return written == TEST_WAV_FRAMES;
}

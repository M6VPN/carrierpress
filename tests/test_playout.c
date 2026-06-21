/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_playout.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_gui_workflow.h"
#include "cp_playout.h"

static int	write_text_file(const char *, const char *);
static int	test_config_validation(void);
static int	test_cue_formatting(void);
static int	test_meter_snapshot(void);
static int	test_path_filter(void);
static int	test_playlist_load(void);
static int	test_playlist_report(void);
static int	test_playlist_rejects_mp3(void);
static int	test_restart_decision(void);

int
main(void)
{
	if (!test_config_validation())
		return 1;
	if (!test_cue_formatting())
		return 1;
	if (!test_meter_snapshot())
		return 1;
	if (!test_path_filter())
		return 1;
	if (!test_playlist_load())
		return 1;
	if (!test_playlist_report())
		return 1;
	if (!test_playlist_rejects_mp3())
		return 1;
	if (!test_restart_decision())
		return 1;

	return 0;
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
test_cue_formatting(void)
{
	char buffer[128];
	char small[8];
	int status;

	status = cp_playout_format_file_start(buffer, sizeof(buffer),
	    "first.wav");
	if (status != CP_PLAYOUT_OK ||
	    strcmp(buffer, "playout: start file=first.wav") != 0) {
		printf("test_playout: file start format mismatch\n");
		return 0;
	}
	status = cp_playout_format_file_done(buffer, sizeof(buffer),
	    "first.wav");
	if (status != CP_PLAYOUT_OK ||
	    strcmp(buffer, "playout: done file=first.wav") != 0) {
		printf("test_playout: file done format mismatch\n");
		return 0;
	}
	status = cp_playout_format_playlist_start(buffer, sizeof(buffer), 3);
	if (status != CP_PLAYOUT_OK ||
	    strcmp(buffer, "playlist: start count=3") != 0) {
		printf("test_playout: playlist start format mismatch\n");
		return 0;
	}
	status = cp_playout_format_playlist_cue(buffer, sizeof(buffer), 1, 3,
	    "second.wav");
	if (status != CP_PLAYOUT_OK ||
	    strcmp(buffer, "cue: 2/3 file=second.wav") != 0) {
		printf("test_playout: playlist cue format mismatch\n");
		return 0;
	}
	status = cp_playout_format_playlist_done(buffer, sizeof(buffer), 3);
	if (status != CP_PLAYOUT_OK ||
	    strcmp(buffer, "playlist: done count=3") != 0) {
		printf("test_playout: playlist done format mismatch\n");
		return 0;
	}
	status = cp_playout_format_stop(buffer, sizeof(buffer));
	if (status != CP_PLAYOUT_OK ||
	    strcmp(buffer, "playout: stopped") != 0) {
		printf("test_playout: stop format mismatch\n");
		return 0;
	}
	if (cp_playout_format_playlist_cue(buffer, sizeof(buffer), 3, 3,
	    "bad.wav") != CP_PLAYOUT_ERR_PLAYLIST) {
		printf("test_playout: invalid cue index accepted\n");
		return 0;
	}
	if (cp_playout_format_file_start(small, sizeof(small),
	    "very-long-file-name.wav") != CP_PLAYOUT_ERR_FORMAT) {
		printf("test_playout: long cue line accepted\n");
		return 0;
	}

	return 1;
}

static int
test_config_validation(void)
{
	struct cp_playout_config config;

	cp_playout_default_config(&config);
	if (config.playlist_index != 0 || config.playlist_count != 0) {
		printf("test_playout: playlist metadata default mismatch\n");
		return 0;
	}
	if (cp_playout_validate_config(&config) != CP_PLAYOUT_OK) {
		printf("test_playout: default config rejected\n");
		return 0;
	}
	if (strcmp(cp_playout_status_string(CP_PLAYOUT_NEXT),
	    "next playlist item") != 0) {
		printf("test_playout: next status string mismatch\n");
		return 0;
	}

	config.meter_interval_ms = CP_AUDIO_MIN_METER_MS - 1;
	if (cp_playout_validate_config(&config) != CP_PLAYOUT_ERR_METER) {
		printf("test_playout: bad meter interval accepted\n");
		return 0;
	}

	cp_playout_default_config(&config);
	config.block_frames = CP_AUDIO_MAX_BLOCK_SIZE + 1;
	if (cp_playout_validate_config(&config) != CP_PLAYOUT_ERR_FORMAT) {
		printf("test_playout: bad block size accepted\n");
		return 0;
	}
	cp_playout_default_config(&config);
	config.audio_config.sample_rate = CP_AUDIO_MIN_SAMPLE_RATE - 1.0;
	if (cp_playout_validate_config(&config) != CP_PLAYOUT_ERR_CONFIG) {
		printf("test_playout: bad audio config accepted\n");
		return 0;
	}
	cp_playout_default_config(&config);
	config.audio_config.am_config.enabled = 1;
	config.audio_config.ssb_config.enabled = 1;
	if (cp_playout_validate_config(&config) != CP_PLAYOUT_ERR_CONFIG) {
		printf("test_playout: AM and SSB config accepted\n");
		return 0;
	}
	cp_playout_default_config(&config);
	config.audio_config.auto_eq_config.enabled = 1;
	if (cp_audio_config_set_format(&config.audio_config,
	    CP_CHANNELS_STEREO, 24000.0) != CP_AUDIO_OK ||
	    cp_playout_validate_config(&config) != CP_PLAYOUT_OK) {
		printf("test_playout: 24000 Hz synced config rejected\n");
		return 0;
	}
	config.audio_config.auto_eq_config.sample_rate = 48000.0f;
	if (cp_playout_validate_config(&config) != CP_PLAYOUT_ERR_CONFIG) {
		printf("test_playout: unsynced auto EQ config accepted\n");
		return 0;
	}

	return 1;
}

static int
test_meter_snapshot(void)
{
	cp_sample_t input[CP_PLAYOUT_DEFAULT_BLOCK_FRAMES];
	cp_sample_t output[CP_PLAYOUT_DEFAULT_BLOCK_FRAMES];
	cp_sample_t scratch[CP_PLAYOUT_DEFAULT_BLOCK_FRAMES];
	struct cp_block_config config;
	struct cp_block_processor processor;
	struct cp_monitor_snapshot snapshot;
	size_t frame;

	cp_block_default_config(&config, CP_CHANNELS_MONO);
	config.sample_rate = 48000.0f;
	if (cp_block_init(&processor, &config) != CP_OK) {
		printf("test_playout: block init failed\n");
		return 0;
	}
	for (frame = 0; frame < CP_PLAYOUT_DEFAULT_BLOCK_FRAMES; frame++)
		input[frame] = 0.10f;
	if (cp_block_process(&processor, input, output, scratch,
	    CP_PLAYOUT_DEFAULT_BLOCK_FRAMES,
	    CP_PLAYOUT_DEFAULT_BLOCK_FRAMES) != CP_OK) {
		printf("test_playout: block process failed\n");
		return 0;
	}
	if (cp_playout_build_snapshot(&processor, &snapshot) !=
	    CP_PLAYOUT_OK) {
		printf("test_playout: snapshot failed\n");
		return 0;
	}
	if (snapshot.input_peak == 0 || snapshot.input_rms == 0 ||
	    snapshot.output_peak == 0 || snapshot.output_rms == 0 ||
	    snapshot.agc_gain == 0) {
		printf("test_playout: snapshot levels missing\n");
		return 0;
	}

	return 1;
}

static int
test_path_filter(void)
{
	if (!cp_playout_path_is_wav("track.wav"))
		return 0;
	if (!cp_playout_path_is_wav("TRACK.WAV"))
		return 0;
	if (cp_playout_path_is_wav("track.mp3"))
		return 0;
	if (cp_playout_path_is_wav("wav"))
		return 0;

	return 1;
}

static int
test_playlist_load(void)
{
	struct cp_playlist playlist;
	const char *path;
	int status;

	path = "build/tests/playout_good.txt";
	if (!write_text_file(path, "\n# comment\n one.wav \nTWO.WAV\n")) {
		printf("test_playout: fixture write failed\n");
		return 0;
	}

	status = cp_playlist_load(path, &playlist);
	if (status != CP_PLAYOUT_OK) {
		printf("test_playout: load failed: %s\n",
		    cp_playout_status_string(status));
		return 0;
	}
	if (cp_playlist_count(&playlist) != 2) {
		cp_playlist_free(&playlist);
		printf("test_playout: count mismatch\n");
		return 0;
	}
	if (strcmp(cp_playlist_get(&playlist, 0), "one.wav") != 0 ||
	    strcmp(cp_playlist_get(&playlist, 1), "TWO.WAV") != 0) {
		cp_playlist_free(&playlist);
		printf("test_playout: path mismatch\n");
		return 0;
	}

	cp_playlist_free(&playlist);
	return 1;
}

static int
test_playlist_report(void)
{
	struct cp_playlist_error error;
	struct cp_playlist playlist;
	const char *path;
	int status;

	path = "build/tests/playout_report.txt";
	if (!write_text_file(path, "# comment\none.wav\nbad.mp3\n")) {
		printf("test_playout: fixture write failed\n");
		return 0;
	}

	status = cp_playlist_load_report(path, &playlist, &error);
	if (status != CP_PLAYOUT_ERR_UNSUPPORTED) {
		printf("test_playout: report unsupported status mismatch\n");
		return 0;
	}
	if (error.line != 3) {
		printf("test_playout: report line mismatch\n");
		return 0;
	}
	if (strcmp(error.path, "bad.mp3") != 0) {
		printf("test_playout: report path mismatch\n");
		return 0;
	}
	if (strstr(error.reason, "unsupported") == NULL) {
		printf("test_playout: report reason mismatch\n");
		return 0;
	}

	return 1;
}

static int
test_playlist_rejects_mp3(void)
{
	struct cp_playlist playlist;
	const char *path;
	int status;

	path = "build/tests/playout_bad.txt";
	if (!write_text_file(path, "good.wav\nbad.mp3\n")) {
		printf("test_playout: fixture write failed\n");
		return 0;
	}

	status = cp_playlist_load(path, &playlist);
	if (status != CP_PLAYOUT_ERR_UNSUPPORTED) {
		printf("test_playout: unsupported file accepted\n");
		return 0;
	}

	return 1;
}

static int
test_restart_decision(void)
{
	struct cp_gui_workflow_request request;
	int requested_device;
	int restart_needed;

	if (!cp_playout_status_allows_output_fallback(CP_PLAYOUT_ERR_AUDIO) ||
	    !cp_playout_status_allows_output_fallback(
	    CP_PLAYOUT_ERR_STREAM) ||
	    !cp_playout_status_allows_output_fallback(
	    CP_PLAYOUT_ERR_CONFIG)) {
		printf("test_playout: fallback status rejected\n");
		return 0;
	}
	if (cp_playout_status_allows_output_fallback(
	    CP_PLAYOUT_ERR_PROCESS) ||
	    cp_playout_status_allows_output_fallback(CP_PLAYOUT_ERR_READ) ||
	    cp_playout_status_allows_output_fallback(CP_PLAYOUT_OK)) {
		printf("test_playout: non-output status allows fallback\n");
		return 0;
	}

	cp_gui_workflow_request_clear(&request);
	if (cp_gui_workflow_request_set_device(&request, 3) != CP_OK ||
	    cp_gui_workflow_output_device_restart_needed(2, &request,
	    &restart_needed, &requested_device) != CP_OK ||
	    !restart_needed || requested_device != 3) {
		printf("test_playout: device restart request rejected\n");
		return 0;
	}
	if (cp_gui_workflow_output_device_restart_needed(3, &request,
	    &restart_needed, &requested_device) != CP_OK ||
	    restart_needed) {
		printf("test_playout: same-device restart requested\n");
		return 0;
	}
	cp_gui_workflow_request_clear(&request);
	if (cp_gui_workflow_request_set_device(&request, -2) == CP_OK ||
	    cp_gui_workflow_output_device_restart_needed(2, &request,
	    &restart_needed, &requested_device) == CP_OK) {
		printf("test_playout: invalid device restart accepted\n");
		return 0;
	}

	return 1;
}

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_playout.c */

#include <sys/types.h>

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <portaudio.h>
#include <sndfile.h>

#include "cp_cat.h"
#include "cp_control.h"
#include "cp_declipper.h"
#ifdef CP_WITH_GUI
#include "cp_gui.h"
#endif
#include "cp_playout.h"
#include "cp_restoration.h"
#include "cp_resampler.h"
#ifdef CP_WITH_FFTW
#include "cp_spectrum.h"
#endif
#ifdef CP_WITH_TUI
#include "cp_tui.h"
#endif

static size_t	cp_playout_interval_frames(double, unsigned int);
static int	cp_playout_append_path(struct cp_playlist *, const char *);
static char	*cp_playout_dup_range(const char *, size_t);
static void	cp_playout_error_clear(struct cp_playlist_error *);
static void	cp_playout_error_set(struct cp_playlist_error *, size_t,
		    const char *, const char *);
static int	cp_playout_name_contains(const char *, const char *);
static int	cp_playout_open_output_stream(PaStream **,
		    const PaStreamParameters *, double, size_t);
#ifdef CP_WITH_GUI
static int	cp_playout_open_started_output_stream(PaStream **,
		    PaStreamParameters *, const struct cp_audio_config *,
		    size_t, double, size_t, PaDeviceIndex *);
#endif
static int	cp_playout_output_device_contains(PaDeviceIndex, const char *);
static int	cp_playout_output_matches(const struct cp_audio_config *,
		    PaDeviceIndex, size_t);
static PaTime	cp_playout_output_latency(const PaDeviceInfo *);
static int	cp_playout_rate_valid(double);
static int	cp_playout_rates_equal(double, double);
static int	cp_playout_read_line(FILE *, char *, size_t, int *);
#ifdef CP_WITH_GUI
static int	cp_playout_restart_output_stream(PaStream **,
		    struct cp_audio_config *, PaStreamParameters *, size_t,
		    double, size_t, PaDeviceIndex *, int, int, int *);
#endif
static int	cp_playout_return_result(struct cp_playout_run_result *, int);
static void	cp_playout_run_result_clear(struct cp_playout_run_result *);
static int	cp_playout_select_output_device(
		    const struct cp_audio_config *, size_t, PaDeviceIndex *);
static int	cp_playout_should_stop(const struct cp_playout_config *);
static int	cp_playout_status_enabled(const struct cp_audio_config *);
static void	cp_playout_print_line(const char *);
static void	cp_playout_print_meters(const struct cp_monitor_snapshot *);
static char	*cp_playout_trim_line(char *);
#ifdef CP_WITH_GUI
static void	cp_playout_workflow_reason(
		    struct cp_gui_workflow_request *, int, const char *);
#endif

int
cp_playout_format_file_done(char *buffer, size_t buffer_size,
	const char *path)
{
	int written;

	if (buffer == NULL || buffer_size == 0 || path == NULL)
		return CP_PLAYOUT_ERR_NULL;

	written = snprintf(buffer, buffer_size, "playout: done file=%s",
	    path);
	if (written < 0 || (size_t)written >= buffer_size)
		return CP_PLAYOUT_ERR_FORMAT;

	return CP_PLAYOUT_OK;
}

int
cp_playout_format_file_start(char *buffer, size_t buffer_size,
	const char *path)
{
	int written;

	if (buffer == NULL || buffer_size == 0 || path == NULL)
		return CP_PLAYOUT_ERR_NULL;

	written = snprintf(buffer, buffer_size, "playout: start file=%s",
	    path);
	if (written < 0 || (size_t)written >= buffer_size)
		return CP_PLAYOUT_ERR_FORMAT;

	return CP_PLAYOUT_OK;
}

int
cp_playout_format_playlist_cue(char *buffer, size_t buffer_size,
	size_t index, size_t count, const char *path)
{
	int written;

	if (buffer == NULL || buffer_size == 0 || path == NULL)
		return CP_PLAYOUT_ERR_NULL;
	if (count == 0 || index >= count)
		return CP_PLAYOUT_ERR_PLAYLIST;

	written = snprintf(buffer, buffer_size, "cue: %zu/%zu file=%s",
	    index + 1, count, path);
	if (written < 0 || (size_t)written >= buffer_size)
		return CP_PLAYOUT_ERR_FORMAT;

	return CP_PLAYOUT_OK;
}

int
cp_playout_format_playlist_done(char *buffer, size_t buffer_size,
	size_t count)
{
	int written;

	if (buffer == NULL || buffer_size == 0)
		return CP_PLAYOUT_ERR_NULL;

	written = snprintf(buffer, buffer_size, "playlist: done count=%zu",
	    count);
	if (written < 0 || (size_t)written >= buffer_size)
		return CP_PLAYOUT_ERR_FORMAT;

	return CP_PLAYOUT_OK;
}

int
cp_playout_format_playlist_start(char *buffer, size_t buffer_size,
	size_t count)
{
	int written;

	if (buffer == NULL || buffer_size == 0)
		return CP_PLAYOUT_ERR_NULL;

	written = snprintf(buffer, buffer_size, "playlist: start count=%zu",
	    count);
	if (written < 0 || (size_t)written >= buffer_size)
		return CP_PLAYOUT_ERR_FORMAT;

	return CP_PLAYOUT_OK;
}

int
cp_playout_format_stop(char *buffer, size_t buffer_size)
{
	int written;

	if (buffer == NULL || buffer_size == 0)
		return CP_PLAYOUT_ERR_NULL;

	written = snprintf(buffer, buffer_size, "%s", "playout: stopped");
	if (written < 0 || (size_t)written >= buffer_size)
		return CP_PLAYOUT_ERR_FORMAT;

	return CP_PLAYOUT_OK;
}

static size_t
cp_playout_interval_frames(double sample_rate, unsigned int meter_interval_ms)
{
	double frames;

	if (!isfinite(sample_rate) || sample_rate < CP_AUDIO_MIN_SAMPLE_RATE)
		return CP_AUDIO_DEFAULT_BLOCK_SIZE;

	frames = sample_rate * (double)meter_interval_ms / 1000.0;
	if (frames < 1.0)
		return 1;
	if (frames > (double)SIZE_MAX)
		return SIZE_MAX;

	return (size_t)frames;
}

void
cp_playout_default_config(struct cp_playout_config *config)
{
	if (config == NULL)
		return;

	cp_audio_default_config(&config->audio_config);
	cp_block_default_config(&config->block_config, CP_CHANNELS_STEREO);
	cp_cat_default_config(&config->cat_config);
	config->block_frames = CP_PLAYOUT_DEFAULT_BLOCK_FRAMES;
	config->meter_interval_ms = CP_AUDIO_DEFAULT_METER_MS;
	config->stop_requested = NULL;
	config->operator_state = NULL;
	config->playlist_index = 0;
	config->playlist_count = 0;
}

int
cp_playout_build_snapshot(const struct cp_block_processor *processor,
	struct cp_monitor_snapshot *snapshot)
{
	int status;

	if (processor == NULL || snapshot == NULL)
		return CP_PLAYOUT_ERR_NULL;

	status = cp_monitor_snapshot_from_processor(processor, snapshot);
	if (status != CP_OK)
		return CP_PLAYOUT_ERR_NULL;

	return CP_PLAYOUT_OK;
}

void
cp_playlist_free(struct cp_playlist *playlist)
{
	size_t index;

	if (playlist == NULL)
		return;

	for (index = 0; index < playlist->count; index++)
		free(playlist->paths[index]);
	free(playlist->paths);
	playlist->paths = NULL;
	playlist->count = 0;
	playlist->capacity = 0;
}

const char *
cp_playlist_get(const struct cp_playlist *playlist, size_t index)
{
	if (playlist == NULL || index >= playlist->count)
		return NULL;

	return playlist->paths[index];
}

int
cp_playlist_load(const char *path, struct cp_playlist *playlist)
{
	return cp_playlist_load_report(path, playlist, NULL);
}

int
cp_playlist_load_report(const char *path, struct cp_playlist *playlist,
	struct cp_playlist_error *error)
{
	FILE *file;
	char line[CP_PLAYOUT_MAX_LINE];
	char *entry;
	size_t line_number;
	int truncated;
	int status;

	if (path == NULL || playlist == NULL)
		return CP_PLAYOUT_ERR_NULL;

	cp_playout_error_clear(error);
	memset(playlist, 0, sizeof(*playlist));
	file = fopen(path, "r");
	if (file == NULL) {
		cp_playout_error_set(error, 0, path, "could not open playlist");
		return CP_PLAYOUT_ERR_PLAYLIST;
	}

	status = CP_PLAYOUT_OK;
	line_number = 0;
	while (cp_playout_read_line(file, line, sizeof(line), &truncated)) {
		line_number++;
		if (truncated) {
			cp_playout_error_set(error, line_number, "",
			    "playlist line is too long");
			status = CP_PLAYOUT_ERR_PLAYLIST;
			break;
		}
		entry = cp_playout_trim_line(line);
		if (entry[0] == '\0' || entry[0] == '#')
			continue;
		if (!cp_playout_path_is_wav(entry)) {
			cp_playout_error_set(error, line_number, entry,
			    "unsupported format: convert to WAV first");
			status = CP_PLAYOUT_ERR_UNSUPPORTED;
			break;
		}
		status = cp_playout_append_path(playlist, entry);
		if (status != CP_PLAYOUT_OK) {
			cp_playout_error_set(error, line_number, entry,
			    cp_playout_status_string(status));
			break;
		}
	}

	fclose(file);
	if (status != CP_PLAYOUT_OK)
		cp_playlist_free(playlist);

	return status;
}

size_t
cp_playlist_count(const struct cp_playlist *playlist)
{
	if (playlist == NULL)
		return 0;

	return playlist->count;
}

int
cp_playout_path_is_wav(const char *path)
{
	size_t length;

	if (path == NULL)
		return 0;

	length = strlen(path);
	if (length < 4)
		return 0;

	path += length - 4;
	if (tolower((unsigned char)path[0]) != '.')
		return 0;
	if (tolower((unsigned char)path[1]) != 'w')
		return 0;
	if (tolower((unsigned char)path[2]) != 'a')
		return 0;
	if (tolower((unsigned char)path[3]) != 'v')
		return 0;

	return 1;
}

int
cp_playout_run_file(const char *path, const struct cp_playout_config *config)
{
	return cp_playout_run_file_with_result(path, config, NULL);
}

int
cp_playout_run_file_with_result(const char *path,
	const struct cp_playout_config *config,
	struct cp_playout_run_result *run_result)
{
	struct cp_monitor_snapshot snapshot;
	SF_INFO input_info;
	SNDFILE *input_file;
	PaStreamParameters output_params;
	PaStream *stream;
	PaDeviceIndex output_device;
	PaError error;
	const PaDeviceInfo *output_info;
	cp_sample_t *input;
	cp_sample_t *output;
	cp_sample_t *process_input;
	cp_sample_t *resampled;
	cp_sample_t *scratch;
	struct cp_audio_config audio_config;
	struct cp_block_config block_config;
	struct cp_block_processor processor;
	struct cp_operator_state operator_state;
	struct cp_resampler resampler;
	struct cp_resampler_config resampler_config;
#if defined(CP_WITH_TUI) || defined(CP_WITH_GUI)
	struct cp_cat_snapshot cat_snapshot;
	struct cp_control_command command;
#endif
#ifdef CP_WITH_TUI
	struct cp_tui tui;
	struct cp_tui_view tui_view;
#endif
#ifdef CP_WITH_GUI
	struct cp_gui gui;
	struct cp_gui_view gui_view;
	struct cp_gui_workflow_request workflow_request;
	struct cp_gui_workflow_request pending_workflow;
	struct cp_waveform_snapshot waveform;
#endif
#ifdef CP_WITH_FFTW
	struct cp_spectrum_analyzer spectrum_analyzer;
	struct cp_spectrum_input spectrum_input;
	struct cp_spectrum_snapshot spectrum;
	int spectrum_ready;
#endif
	sf_count_t frames_read;
	size_t block_frames;
	size_t block_samples;
	size_t channels;
	size_t frames_until_meter;
	size_t meter_frames;
	size_t output_block_frames;
	size_t output_block_samples;
	size_t process_frames;
	size_t resampled_frames;
	double input_rate;
	double output_rate;
	char status_line[CP_PLAYOUT_MAX_LINE + CP_PLAYOUT_ERROR_TEXT];
	int result;
	int resampling;
	int status;
	unsigned int block_stream_flags;
#ifdef CP_WITH_GUI
	int requested_output_device;
	int fallback_used;
	int restart_needed;
#endif

#ifdef CP_WITH_FFTW
	spectrum_ready = 0;
#endif
	cp_playout_run_result_clear(run_result);
	if (path == NULL || config == NULL)
		return cp_playout_return_result(run_result, CP_PLAYOUT_ERR_NULL);
	status = cp_playout_validate_config(config);
	if (status != CP_PLAYOUT_OK)
		return cp_playout_return_result(run_result, status);
	if (!cp_playout_path_is_wav(path))
		return cp_playout_return_result(run_result,
		    CP_PLAYOUT_ERR_UNSUPPORTED);
	if (config->audio_config.tui_enabled &&
	    config->audio_config.gui_enabled)
		return cp_playout_return_result(run_result, CP_PLAYOUT_ERR_AUDIO);
#ifndef CP_WITH_TUI
	if (config->audio_config.tui_enabled)
		return cp_playout_return_result(run_result, CP_PLAYOUT_ERR_AUDIO);
#endif
#ifndef CP_WITH_GUI
	if (config->audio_config.gui_enabled)
		return cp_playout_return_result(run_result, CP_PLAYOUT_ERR_AUDIO);
#endif

	memset(&input_info, 0, sizeof(input_info));
	input_file = sf_open(path, SFM_READ, &input_info);
	if (input_file == NULL)
		return cp_playout_return_result(run_result,
		    CP_PLAYOUT_ERR_OPEN_IN);
	if (input_info.channels != CP_CHANNELS_MONO &&
	    input_info.channels != CP_CHANNELS_STEREO) {
		sf_close(input_file);
		return cp_playout_return_result(run_result,
		    CP_PLAYOUT_ERR_CHANNELS);
	}
	if (input_info.samplerate < CP_AUDIO_MIN_SAMPLE_RATE ||
	    input_info.samplerate > CP_AUDIO_MAX_SAMPLE_RATE) {
		sf_close(input_file);
		return cp_playout_return_result(run_result,
		    CP_PLAYOUT_ERR_FORMAT);
	}

	channels     = (size_t)input_info.channels;
	block_frames = config->block_frames;
	if (block_frames > (SIZE_MAX / channels)) {
		sf_close(input_file);
		return cp_playout_return_result(run_result,
		    CP_PLAYOUT_ERR_FORMAT);
	}
	block_samples = block_frames * channels;

	input = NULL;
	output = NULL;
	process_input = NULL;
	resampled = NULL;
	scratch = NULL;
	stream = NULL;
	input_rate = (double)input_info.samplerate;
	output_rate = config->audio_config.sample_rate_explicit ?
	    config->audio_config.sample_rate : input_rate;

	audio_config = config->audio_config;
	audio_config.sample_rate_explicit = 1;
	status = cp_audio_config_set_format(&audio_config, channels,
	    output_rate);
	if (status != CP_AUDIO_OK) {
		sf_close(input_file);
		return cp_playout_return_result(run_result,
		    CP_PLAYOUT_ERR_CONFIG);
	}
	status = cp_audio_validate_config(&audio_config);
	if (status != CP_AUDIO_OK) {
		sf_close(input_file);
		return cp_playout_return_result(run_result,
		    CP_PLAYOUT_ERR_CONFIG);
	}
	if (config->operator_state != NULL)
		operator_state = *config->operator_state;
	else
		cp_operator_state_clear(&operator_state);
	operator_state.cue_path = path;
	operator_state.playlist_index = config->playlist_index;
	operator_state.playlist_count = config->playlist_count;

	error = Pa_Initialize();
	if (error != paNoError) {
		sf_close(input_file);
		return cp_playout_return_result(run_result,
		    CP_PLAYOUT_ERR_AUDIO);
	}
	status = cp_playout_select_output_device(&audio_config, channels,
	    &output_device);
	if (status != CP_PLAYOUT_OK) {
		Pa_Terminate();
		sf_close(input_file);
		return cp_playout_return_result(run_result, status);
	}

	output_info = Pa_GetDeviceInfo(output_device);
	if (output_info == NULL) {
		Pa_Terminate();
		sf_close(input_file);
		return cp_playout_return_result(run_result,
		    CP_PLAYOUT_ERR_AUDIO);
	}

	output_params.device = output_device;
	output_params.channelCount = (int)channels;
	output_params.sampleFormat = paFloat32;
	output_params.suggestedLatency = cp_playout_output_latency(output_info);
	output_params.hostApiSpecificStreamInfo = NULL;

	status = cp_playout_open_output_stream(&stream, &output_params,
	    output_rate, block_frames);
	if (status != CP_PLAYOUT_OK &&
	    !config->audio_config.sample_rate_explicit &&
	    cp_playout_rate_valid(output_info->defaultSampleRate) &&
	    !cp_playout_rates_equal(output_rate,
	    output_info->defaultSampleRate)) {
		output_rate = output_info->defaultSampleRate;
		if (cp_audio_config_set_format(&audio_config, channels,
		    output_rate) != CP_AUDIO_OK ||
		    cp_audio_validate_config(&audio_config) != CP_AUDIO_OK) {
			Pa_Terminate();
			sf_close(input_file);
			return CP_PLAYOUT_ERR_CONFIG;
		}
		status = cp_playout_open_output_stream(&stream,
		    &output_params, output_rate, block_frames);
	}
	if (status != CP_PLAYOUT_OK) {
		Pa_Terminate();
		sf_close(input_file);
		return status;
	}

	resampling = !cp_playout_rates_equal(input_rate, output_rate);
	output_block_frames = resampling ?
	    cp_resampler_output_capacity(block_frames, input_rate,
	    output_rate) : block_frames;
	if (output_block_frames == 0 ||
	    output_block_frames > (SIZE_MAX / channels)) {
		Pa_CloseStream(stream);
		Pa_Terminate();
		sf_close(input_file);
		return CP_PLAYOUT_ERR_FORMAT;
	}
	output_block_samples = output_block_frames * channels;

	status = cp_block_config_from_audio(&block_config, &audio_config,
	    channels, (cp_sample_t)output_rate);
	if (status != CP_OK) {
		Pa_CloseStream(stream);
		Pa_Terminate();
		sf_close(input_file);
		return CP_PLAYOUT_ERR_PROCESS;
	}
	status = cp_block_init(&processor, &block_config);
	if (status != CP_OK) {
		Pa_CloseStream(stream);
		Pa_Terminate();
		sf_close(input_file);
		return CP_PLAYOUT_ERR_PROCESS;
	}

	cp_resampler_default_config(&resampler_config);
	resampler_config.input_rate = input_rate;
	resampler_config.output_rate = output_rate;
	resampler_config.channel_count = channels;
	resampler_config.enabled = resampling;
	status = cp_resampler_init(&resampler, &resampler_config);
	if (status != CP_OK) {
		Pa_CloseStream(stream);
		Pa_Terminate();
		sf_close(input_file);
		return CP_PLAYOUT_ERR_PROCESS;
	}

	input = calloc(block_samples, sizeof(*input));
	if (resampling)
		resampled = calloc(output_block_samples, sizeof(*resampled));
	output = calloc(output_block_samples, sizeof(*output));
	scratch = calloc(output_block_samples, sizeof(*scratch));
	if (input == NULL || output == NULL || scratch == NULL ||
	    (resampling && resampled == NULL)) {
		Pa_CloseStream(stream);
		Pa_Terminate();
		free(input);
		free(output);
		free(resampled);
		free(scratch);
		sf_close(input_file);
		return CP_PLAYOUT_ERR_ALLOC;
	}
#ifdef CP_WITH_TUI
	tui.active = 0;
	if (audio_config.tui_enabled && cp_tui_init(&tui) != 0) {
		Pa_CloseStream(stream);
		Pa_Terminate();
		free(input);
		free(output);
		free(resampled);
		free(scratch);
		sf_close(input_file);
		return CP_PLAYOUT_ERR_AUDIO;
	}
#endif
#ifdef CP_WITH_GUI
	gui.active = 0;
	cp_gui_workflow_request_clear(&workflow_request);
	cp_waveform_clear(&waveform);
#ifdef CP_WITH_FFTW
	cp_spectrum_input_clear(&spectrum_input);
	cp_spectrum_clear(&spectrum);
#endif
	if (audio_config.gui_enabled && cp_gui_init(&gui) != CP_OK) {
#ifdef CP_WITH_TUI
		cp_tui_close(&tui);
#endif
		Pa_CloseStream(stream);
		Pa_Terminate();
		free(input);
		free(output);
		free(resampled);
		free(scratch);
		sf_close(input_file);
		return CP_PLAYOUT_ERR_AUDIO;
	}
#endif
#ifdef CP_WITH_FFTW
	if (audio_config.gui_enabled &&
	    cp_spectrum_analyzer_init(&spectrum_analyzer) != CP_OK) {
#ifdef CP_WITH_GUI
		cp_gui_close(&gui);
#endif
#ifdef CP_WITH_TUI
		cp_tui_close(&tui);
#endif
		Pa_CloseStream(stream);
		Pa_Terminate();
		free(input);
		free(output);
		free(resampled);
		free(scratch);
		sf_close(input_file);
		return CP_PLAYOUT_ERR_AUDIO;
	}
	if (audio_config.gui_enabled)
		spectrum_ready = 1;
#endif
	error = Pa_StartStream(stream);
	if (error != paNoError) {
#ifdef CP_WITH_FFTW
		if (spectrum_ready)
			cp_spectrum_analyzer_close(&spectrum_analyzer);
#endif
#ifdef CP_WITH_GUI
		cp_gui_close(&gui);
#endif
#ifdef CP_WITH_TUI
		cp_tui_close(&tui);
#endif
		Pa_CloseStream(stream);
		Pa_Terminate();
		free(input);
		free(output);
		free(resampled);
		free(scratch);
		sf_close(input_file);
		return CP_PLAYOUT_ERR_STREAM;
	}

	if (!audio_config.tui_enabled && !audio_config.gui_enabled) {
		if (cp_playout_format_file_start(status_line,
		    sizeof(status_line), path) == CP_PLAYOUT_OK)
			cp_playout_print_line(status_line);
		if (resampling) {
			printf("playing: %s input_rate=%0.0f output_rate=%0.0f "
			    "channels=%zu output_device=%d resampler=linear\n",
			    path, input_rate, output_rate, channels,
			    output_device);
		} else {
			printf("playing: %s rate=%0.0f channels=%zu "
			    "output_device=%d\n", path, output_rate, channels,
			    output_device);
		}
	}

	result = CP_PLAYOUT_OK;
	meter_frames = cp_playout_interval_frames(output_rate,
	    config->meter_interval_ms);
	frames_until_meter = meter_frames;
	while (!cp_playout_should_stop(config) &&
	    (frames_read = sf_readf_float(input_file, input,
	    (sf_count_t)block_frames)) > 0) {
		if (resampling) {
			status = cp_resampler_process(&resampler, input,
			    (size_t)frames_read, resampled,
			    output_block_frames, &resampled_frames);
			if (status != CP_OK) {
				result = CP_PLAYOUT_ERR_PROCESS;
				break;
			}
			process_input = resampled;
			process_frames = resampled_frames;
		} else {
			process_input = input;
			process_frames = (size_t)frames_read;
		}
		if (process_frames == 0)
			continue;
		block_stream_flags = 0;
		status = cp_block_process(&processor, process_input, output,
		    scratch, output_block_samples, process_frames);
		if (status != CP_OK) {
			result = CP_PLAYOUT_ERR_PROCESS;
			break;
		}
#ifdef CP_WITH_GUI
		if (audio_config.gui_enabled)
			(void)cp_waveform_capture(&waveform, output,
			    process_frames, channels);
#endif
#ifdef CP_WITH_FFTW
		if (spectrum_ready &&
		    cp_spectrum_capture_input(&spectrum_input, output,
		    process_frames, channels, output_rate) == CP_OK) {
			if (cp_spectrum_analyze(&spectrum_analyzer,
			    &spectrum_input, &spectrum) != CP_OK)
				cp_spectrum_clear(&spectrum);
		}
#endif
		error = Pa_WriteStream(stream, output,
		    (unsigned long)process_frames);
		if (error != paNoError) {
			if (error == paOutputUnderflowed) {
				block_stream_flags |=
				    CP_MONITOR_OUTPUT_UNDERFLOW;
			} else {
				result = CP_PLAYOUT_ERR_WRITE;
				break;
			}
		}
		status = cp_playout_build_snapshot(&processor, &snapshot);
		if (status != CP_PLAYOUT_OK) {
			result = status;
			break;
		}
		snapshot.stream_flags |= block_stream_flags;
#ifdef CP_WITH_TUI
		if (audio_config.tui_enabled) {
			(void)cp_cat_snapshot_update(&config->cat_config,
			    &cat_snapshot);
			memset(&tui_view, 0, sizeof(tui_view));
			tui_view.mode           = CP_TUI_MODE_PLAYOUT;
			tui_view.config         = &audio_config;
			tui_view.snapshot       = &snapshot;
			tui_view.cat_snapshot   = &cat_snapshot;
			tui_view.operator_state = &operator_state;
			tui_view.path           = path;
			tui_view.playlist_index = config->playlist_index;
			tui_view.playlist_count = config->playlist_count;
			tui_view.next_enabled   = config->playlist_count >
			    config->playlist_index + 1;
			tui_view.output_device  = (int)output_device;
			if (cp_tui_update_view(&tui, &tui_view, &command)) {
				if (config->stop_requested != NULL)
					*config->stop_requested = 1;
				break;
			}
			if (command.type == CP_CONTROL_COMMAND_PLAYOUT_NEXT) {
				result = CP_PLAYOUT_NEXT;
				break;
			}
			if (command.type != CP_CONTROL_COMMAND_NONE) {
				status = cp_control_apply(&processor, &command);
				if (status != CP_OK) {
					result = CP_PLAYOUT_ERR_PROCESS;
					break;
				}
			}
			continue;
		}
#endif
#ifdef CP_WITH_GUI
		if (audio_config.gui_enabled) {
			(void)cp_cat_snapshot_update(&config->cat_config,
			    &cat_snapshot);
			memset(&gui_view, 0, sizeof(gui_view));
			gui_view.mode           = CP_GUI_MODE_PLAYOUT;
			gui_view.config         = &audio_config;
			gui_view.snapshot       = &snapshot;
			gui_view.cat_snapshot   = &cat_snapshot;
			gui_view.operator_state = &operator_state;
			gui_view.workflow_request = &workflow_request;
			gui_view.waveform       = &waveform;
#ifdef CP_WITH_FFTW
			gui_view.spectrum       = &spectrum;
#endif
			gui_view.cue_wav_path =
			    audio_config.gui_cue_wav_path;
			gui_view.cue_playlist_path =
			    audio_config.gui_cue_playlist_path;
			gui_view.path           = path;
			gui_view.playlist_index = config->playlist_index;
			gui_view.playlist_count = config->playlist_count;
			gui_view.output_device  = (int)output_device;
			if (cp_gui_update(&gui, &gui_view) != CP_OK) {
				result = CP_PLAYOUT_ERR_AUDIO;
				break;
			}
			if (cp_gui_should_stop(&gui)) {
				if (config->stop_requested != NULL)
					*config->stop_requested = 1;
				break;
			}
			if (cp_gui_take_workflow_request(&gui,
			    &pending_workflow) == CP_OK &&
			    pending_workflow.type !=
			    CP_GUI_WORKFLOW_REQUEST_NONE) {
				(void)cp_gui_workflow_request_validate(
				    &pending_workflow);
				workflow_request = pending_workflow;
				if (pending_workflow.type ==
				    CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE &&
				    pending_workflow.validation_status == CP_OK &&
				    cp_gui_workflow_output_device_restart_needed(
				    (int)output_device, &pending_workflow,
				    &restart_needed,
				    &requested_output_device) == CP_OK &&
				    restart_needed) {
					fallback_used = 0;
					status = cp_playout_restart_output_stream(
					    &stream, &audio_config,
					    &output_params, channels,
					    output_rate, block_frames,
					    &output_device,
					    requested_output_device,
					    (int)output_device,
					    &fallback_used);
					if (run_result != NULL) {
						run_result->restart_requested = 1;
						run_result->requested_output_device =
						    requested_output_device;
					}
					if (status == CP_PLAYOUT_OK) {
						cp_playout_workflow_reason(
						    &workflow_request, CP_OK,
						    fallback_used ?
						    "fallback" :
						    "accepted");
					} else {
						cp_playout_workflow_reason(
						    &workflow_request, status,
						    cp_playout_status_string(
						    status));
						result = status;
						break;
					}
				}
			}
			if (cp_gui_take_control_command(&gui, &command) ==
			    CP_OK) {
				if (command.type ==
				    CP_CONTROL_COMMAND_PLAYOUT_NEXT) {
					result = CP_PLAYOUT_NEXT;
					break;
				}
				if (command.type != CP_CONTROL_COMMAND_NONE &&
				    command.type !=
				    CP_CONTROL_COMMAND_STOP) {
					status = cp_control_apply(&processor,
					    &command);
					if (status != CP_OK) {
						result =
						    CP_PLAYOUT_ERR_PROCESS;
						break;
					}
				}
			}
			continue;
		}
#endif
		if (process_frames >= frames_until_meter) {
			cp_playout_print_meters(&snapshot);
			frames_until_meter = meter_frames;
		} else {
			frames_until_meter -= process_frames;
		}
	}
	if (!cp_playout_should_stop(config) && frames_read < 0 &&
	    result == CP_PLAYOUT_OK)
		result = CP_PLAYOUT_ERR_READ;
	if (result == CP_PLAYOUT_OK && !audio_config.tui_enabled &&
	    !audio_config.gui_enabled) {
		status = cp_playout_build_snapshot(&processor, &snapshot);
		if (status == CP_PLAYOUT_OK)
			cp_playout_print_meters(&snapshot);
	}

	if (stream != NULL)
		Pa_StopStream(stream);
#ifdef CP_WITH_TUI
	cp_tui_close(&tui);
#endif
#ifdef CP_WITH_GUI
	cp_gui_close(&gui);
#endif
#ifdef CP_WITH_FFTW
	if (spectrum_ready)
		cp_spectrum_analyzer_close(&spectrum_analyzer);
#endif
	if (stream != NULL)
		Pa_CloseStream(stream);
	Pa_Terminate();
	free(input);
	free(output);
	free(resampled);
	free(scratch);
	sf_close(input_file);

	if (cp_playout_status_enabled(&audio_config) &&
	    result == CP_PLAYOUT_OK) {
		if (cp_playout_should_stop(config)) {
			if (config->playlist_count == 0 &&
			    cp_playout_format_stop(status_line,
			    sizeof(status_line)) == CP_PLAYOUT_OK)
				cp_playout_print_line(status_line);
		} else if (cp_playout_format_file_done(status_line,
		    sizeof(status_line), path) == CP_PLAYOUT_OK) {
			cp_playout_print_line(status_line);
		}
	}

	if (run_result != NULL)
		run_result->status = result;

	return result;
}

int
cp_playout_run_playlist(const char *path,
	const struct cp_playout_config *config)
{
	return cp_playout_run_playlist_with_result(path, config, NULL);
}

int
cp_playout_run_playlist_with_result(const char *path,
	const struct cp_playout_config *config,
	struct cp_playout_run_result *run_result)
{
	struct cp_playout_config file_config;
	struct cp_playlist_error error;
	struct cp_playlist playlist;
	struct cp_playout_run_result file_result;
	const char *entry;
	char status_line[CP_PLAYOUT_MAX_LINE + CP_PLAYOUT_ERROR_TEXT];
	size_t index;
	int status_output;
	int status;

	cp_playout_run_result_clear(run_result);
	if (path == NULL || config == NULL)
		return cp_playout_return_result(run_result, CP_PLAYOUT_ERR_NULL);

	status = cp_playlist_load_report(path, &playlist, &error);
	if (status != CP_PLAYOUT_OK) {
		if (error.line > 0) {
			fprintf(stderr, "playlist error: line=%zu path=%s "
			    "reason=%s\n", error.line, error.path,
			    error.reason);
		} else if (error.reason[0] != '\0') {
			fprintf(stderr, "playlist error: path=%s reason=%s\n",
			    error.path, error.reason);
		}
		return cp_playout_return_result(run_result, status);
	}

	if (playlist.count == 0) {
		cp_playlist_free(&playlist);
		return CP_PLAYOUT_ERR_PLAYLIST;
	}

	status_output = cp_playout_status_enabled(&config->audio_config);
	if (status_output &&
	    cp_playout_format_playlist_start(status_line,
	    sizeof(status_line), playlist.count) == CP_PLAYOUT_OK)
		cp_playout_print_line(status_line);

	for (index = 0; index < playlist.count; index++) {
		if (cp_playout_should_stop(config))
			break;
		entry = cp_playlist_get(&playlist, index);
		if (status_output &&
		    cp_playout_format_playlist_cue(status_line,
		    sizeof(status_line), index, playlist.count, entry) ==
		    CP_PLAYOUT_OK)
			cp_playout_print_line(status_line);
		file_config = *config;
		file_config.playlist_index = index;
		file_config.playlist_count = playlist.count;
		status = cp_playout_run_file_with_result(entry, &file_config,
		    &file_result);
		if (run_result != NULL && file_result.restart_requested) {
			run_result->restart_requested = 1;
			run_result->requested_output_device =
			    file_result.requested_output_device;
		}
		if (status == CP_PLAYOUT_NEXT) {
			if (status_output)
				cp_playout_print_line("cue: next requested");
			status = CP_PLAYOUT_OK;
			continue;
		}
		if (status != CP_PLAYOUT_OK)
			break;
	}

	if (status_output && status == CP_PLAYOUT_OK) {
		if (cp_playout_should_stop(config)) {
			if (cp_playout_format_stop(status_line,
			    sizeof(status_line)) == CP_PLAYOUT_OK)
				cp_playout_print_line(status_line);
		} else if (cp_playout_format_playlist_done(status_line,
		    sizeof(status_line), playlist.count) == CP_PLAYOUT_OK) {
			cp_playout_print_line(status_line);
		}
	}

	cp_playlist_free(&playlist);
	if (run_result != NULL)
		run_result->status = status;
	return status;
}

int
cp_playout_status_allows_output_fallback(int status)
{
	return status == CP_PLAYOUT_ERR_AUDIO ||
	    status == CP_PLAYOUT_ERR_STREAM ||
	    status == CP_PLAYOUT_ERR_CONFIG;
}

const char *
cp_playout_status_string(int status)
{
	switch (status) {
	case CP_PLAYOUT_NEXT:
		return "next playlist item";
	case CP_PLAYOUT_OK:
		return "ok";
	case CP_PLAYOUT_ERR_NULL:
		return "missing playout path or config";
	case CP_PLAYOUT_ERR_ALLOC:
		return "could not allocate playout buffers";
	case CP_PLAYOUT_ERR_PLAYLIST:
		return "invalid playlist";
	case CP_PLAYOUT_ERR_OPEN_IN:
		return "could not open playout WAV";
	case CP_PLAYOUT_ERR_FORMAT:
		return "invalid playout format";
	case CP_PLAYOUT_ERR_CHANNELS:
		return "playout WAV must be mono or stereo";
	case CP_PLAYOUT_ERR_AUDIO:
		return "invalid or unavailable output audio device";
	case CP_PLAYOUT_ERR_STREAM:
		return "could not open playout audio stream";
	case CP_PLAYOUT_ERR_PROCESS:
		return "DSP processing failed";
	case CP_PLAYOUT_ERR_READ:
		return "could not read playout WAV";
	case CP_PLAYOUT_ERR_WRITE:
		return "could not write playout audio stream";
	case CP_PLAYOUT_ERR_UNSUPPORTED:
		return "unsupported playout format: use WAV in this milestone";
	case CP_PLAYOUT_ERR_METER:
		return "invalid playout meter interval";
	case CP_PLAYOUT_ERR_CONFIG:
		return "invalid playout audio config";
	default:
		return "unknown playout error";
	}
}

int
cp_playout_validate_config(const struct cp_playout_config *config)
{
	int status;

	if (config == NULL)
		return CP_PLAYOUT_ERR_NULL;
	if (config->block_frames < CP_AUDIO_MIN_BLOCK_SIZE ||
	    config->block_frames > CP_AUDIO_MAX_BLOCK_SIZE)
		return CP_PLAYOUT_ERR_FORMAT;
	if (config->block_frames > (SIZE_MAX / CP_CHANNELS_STEREO))
		return CP_PLAYOUT_ERR_FORMAT;
	if (config->meter_interval_ms < CP_AUDIO_MIN_METER_MS ||
	    config->meter_interval_ms > CP_AUDIO_MAX_METER_MS)
		return CP_PLAYOUT_ERR_METER;
	status = cp_audio_validate_config(&config->audio_config);
	if (status != CP_AUDIO_OK)
		return CP_PLAYOUT_ERR_CONFIG;

	return CP_PLAYOUT_OK;
}

static int
cp_playout_append_path(struct cp_playlist *playlist, const char *path)
{
	char **paths;
	char *copy;
	size_t capacity;
	size_t length;

	if (playlist == NULL || path == NULL)
		return CP_PLAYOUT_ERR_NULL;

	if (playlist->count == playlist->capacity) {
		capacity = playlist->capacity == 0 ? 8 : playlist->capacity * 2;
		if (capacity < playlist->capacity ||
		    capacity > (SIZE_MAX / sizeof(*playlist->paths)))
			return CP_PLAYOUT_ERR_ALLOC;
		paths = realloc(playlist->paths,
		    capacity * sizeof(*playlist->paths));
		if (paths == NULL)
			return CP_PLAYOUT_ERR_ALLOC;
		playlist->paths = paths;
		playlist->capacity = capacity;
	}

	length = strlen(path);
	copy = cp_playout_dup_range(path, length);
	if (copy == NULL)
		return CP_PLAYOUT_ERR_ALLOC;

	playlist->paths[playlist->count++] = copy;
	return CP_PLAYOUT_OK;
}

static char *
cp_playout_dup_range(const char *text, size_t length)
{
	char *copy;

	if (text == NULL)
		return NULL;
	if (length == SIZE_MAX)
		return NULL;

	copy = malloc(length + 1);
	if (copy == NULL)
		return NULL;
	memcpy(copy, text, length);
	copy[length] = '\0';

	return copy;
}

static void
cp_playout_error_clear(struct cp_playlist_error *error)
{
	if (error == NULL)
		return;

	memset(error, 0, sizeof(*error));
}

static void
cp_playout_error_set(struct cp_playlist_error *error, size_t line,
	const char *path, const char *reason)
{
	int written;

	if (error == NULL)
		return;

	error->line = line;
	if (path != NULL) {
		written = snprintf(error->path, sizeof(error->path), "%s",
		    path);
		if (written < 0 || (size_t)written >= sizeof(error->path))
			error->path[sizeof(error->path) - 1] = '\0';
	}
	if (reason != NULL) {
		written = snprintf(error->reason, sizeof(error->reason), "%s",
		    reason);
		if (written < 0 || (size_t)written >= sizeof(error->reason))
			error->reason[sizeof(error->reason) - 1] = '\0';
	}
}

static int
cp_playout_name_contains(const char *text, const char *needle)
{
	size_t index;
	size_t offset;

	if (text == NULL || needle == NULL || needle[0] == '\0')
		return 0;

	for (offset = 0; text[offset] != '\0'; offset++) {
		for (index = 0; needle[index] != '\0'; index++) {
			if (text[offset + index] == '\0')
				return 0;
			if (tolower((unsigned char)text[offset + index]) !=
			    tolower((unsigned char)needle[index]))
				break;
		}
		if (needle[index] == '\0')
			return 1;
	}

	return 0;
}

static int
cp_playout_open_output_stream(PaStream **stream,
	const PaStreamParameters *output_params, double sample_rate,
	size_t block_frames)
{
	PaError error;

	if (stream == NULL || output_params == NULL)
		return CP_PLAYOUT_ERR_NULL;
	if (!cp_playout_rate_valid(sample_rate))
		return CP_PLAYOUT_ERR_FORMAT;
	if (block_frames == 0 || block_frames > (size_t)ULONG_MAX)
		return CP_PLAYOUT_ERR_FORMAT;

	*stream = NULL;
	error = Pa_OpenStream(stream, NULL, output_params, sample_rate,
	    (unsigned long)block_frames, paNoFlag, NULL, NULL);
	if (error != paNoError)
		return CP_PLAYOUT_ERR_STREAM;

	return CP_PLAYOUT_OK;
}

#ifdef CP_WITH_GUI
static int
cp_playout_open_started_output_stream(PaStream **stream,
	PaStreamParameters *output_params, const struct cp_audio_config *config,
	size_t channels, double sample_rate, size_t block_frames,
	PaDeviceIndex *output_device)
{
	const PaDeviceInfo *output_info;
	PaError error;
	int status;

	if (stream == NULL || output_params == NULL || config == NULL ||
	    output_device == NULL)
		return CP_PLAYOUT_ERR_NULL;

	*stream = NULL;
	status = cp_playout_select_output_device(config, channels,
	    output_device);
	if (status != CP_PLAYOUT_OK)
		return status;

	output_info = Pa_GetDeviceInfo(*output_device);
	if (output_info == NULL)
		return CP_PLAYOUT_ERR_AUDIO;

	output_params->device = *output_device;
	output_params->channelCount = (int)channels;
	output_params->sampleFormat = paFloat32;
	output_params->suggestedLatency =
	    cp_playout_output_latency(output_info);
	output_params->hostApiSpecificStreamInfo = NULL;

	status = cp_playout_open_output_stream(stream, output_params,
	    sample_rate, block_frames);
	if (status != CP_PLAYOUT_OK)
		return status;

	error = Pa_StartStream(*stream);
	if (error != paNoError) {
		Pa_CloseStream(*stream);
		*stream = NULL;
		return CP_PLAYOUT_ERR_STREAM;
	}

	return CP_PLAYOUT_OK;
}
#endif

static int
cp_playout_output_matches(const struct cp_audio_config *config,
	PaDeviceIndex device, size_t channels)
{
	const PaDeviceInfo *device_info;
	const PaHostApiInfo *host_info;
	const char *host_name;

	device_info = Pa_GetDeviceInfo(device);
	if (device_info == NULL)
		return 0;
	if (device_info->maxOutputChannels < (int)channels)
		return 0;

	host_info = Pa_GetHostApiInfo(device_info->hostApi);
	host_name = host_info == NULL ? "" : host_info->name;
	if (config->device_name != NULL)
		return cp_playout_name_contains(device_info->name,
		    config->device_name);

	switch (config->backend) {
	case CP_AUDIO_BACKEND_JACK:
	case CP_AUDIO_BACKEND_ALSA:
	case CP_AUDIO_BACKEND_PULSE:
		return cp_playout_name_contains(host_name,
		    cp_audio_backend_string(config->backend)) ||
		    cp_playout_name_contains(device_info->name,
		    cp_audio_backend_string(config->backend));
	case CP_AUDIO_BACKEND_AUTO:
	case CP_AUDIO_BACKEND_DEFAULT:
		return 1;
	default:
		return 0;
	}
}

static int
cp_playout_output_device_contains(PaDeviceIndex device, const char *needle)
{
	const PaDeviceInfo *device_info;
	const PaHostApiInfo *host_info;

	device_info = Pa_GetDeviceInfo(device);
	if (device_info == NULL)
		return 0;

	host_info = Pa_GetHostApiInfo(device_info->hostApi);
	if (cp_playout_name_contains(device_info->name, needle))
		return 1;
	if (host_info != NULL &&
	    cp_playout_name_contains(host_info->name, needle))
		return 1;

	return 0;
}

static PaTime
cp_playout_output_latency(const PaDeviceInfo *device_info)
{
	if (device_info == NULL)
		return 0.0;
	if (device_info->defaultHighOutputLatency > 0.0)
		return device_info->defaultHighOutputLatency;

	return device_info->defaultLowOutputLatency;
}

static int
cp_playout_rate_valid(double sample_rate)
{
	return isfinite(sample_rate) &&
	    sample_rate >= CP_AUDIO_MIN_SAMPLE_RATE &&
	    sample_rate <= CP_AUDIO_MAX_SAMPLE_RATE;
}

static int
cp_playout_rates_equal(double left, double right)
{
	double difference;

	difference = left - right;
	if (difference < 0.0)
		difference = -difference;

	return difference <= 0.5;
}

static int
cp_playout_read_line(FILE *file, char *buffer, size_t buffer_size,
	int *truncated)
{
	size_t length;
	int character;

	if (file == NULL || buffer == NULL || buffer_size < 2 ||
	    truncated == NULL)
		return 0;

	*truncated = 0;
	if (fgets(buffer, (int)buffer_size, file) == NULL)
		return 0;

	length = strlen(buffer);
	if (length > 0 && buffer[length - 1] == '\n')
		return 1;
	if (feof(file))
		return 1;

	*truncated = 1;
	do {
		character = fgetc(file);
	} while (character != '\n' && character != EOF);

	return 1;
}

#ifdef CP_WITH_GUI
static int
cp_playout_restart_output_stream(PaStream **stream,
	struct cp_audio_config *config, PaStreamParameters *output_params,
	size_t channels, double sample_rate, size_t block_frames,
	PaDeviceIndex *output_device, int requested_output_device,
	int fallback_output_device, int *fallback_used)
{
	PaStream *replacement;
	int original_output_device;
	int status;

	if (stream == NULL || config == NULL || output_params == NULL ||
	    output_device == NULL)
		return CP_PLAYOUT_ERR_NULL;
	if (requested_output_device < CP_AUDIO_DEFAULT_DEVICE ||
	    fallback_output_device < CP_AUDIO_DEFAULT_DEVICE)
		return CP_PLAYOUT_ERR_AUDIO;
	if (fallback_used != NULL)
		*fallback_used = 0;

	if (*stream != NULL) {
		(void)Pa_StopStream(*stream);
		Pa_CloseStream(*stream);
		*stream = NULL;
	}

	original_output_device = config->output_device;
	config->output_device = requested_output_device;
	replacement = NULL;
	status = cp_playout_open_started_output_stream(&replacement,
	    output_params, config, channels, sample_rate, block_frames,
	    output_device);
	if (status == CP_PLAYOUT_OK) {
		*stream = replacement;
		return CP_PLAYOUT_OK;
	}
	if (!cp_playout_status_allows_output_fallback(status)) {
		config->output_device = original_output_device;
		return status;
	}

	printf("carrierpress: requested playout output device %d failed: %s\n",
	    requested_output_device, cp_playout_status_string(status));
	printf("carrierpress: falling back to previous playout output "
	    "device %d\n", fallback_output_device);

	config->output_device = fallback_output_device;
	replacement = NULL;
	status = cp_playout_open_started_output_stream(&replacement,
	    output_params, config, channels, sample_rate, block_frames,
	    output_device);
	if (status == CP_PLAYOUT_OK) {
		*stream = replacement;
		if (fallback_used != NULL)
			*fallback_used = 1;
		return CP_PLAYOUT_OK;
	}

	config->output_device = original_output_device;
	printf("carrierpress: previous playout output device %d failed: %s\n",
	    fallback_output_device, cp_playout_status_string(status));

	return status;
}
#endif

static int
cp_playout_return_result(struct cp_playout_run_result *result, int status)
{
	if (result != NULL)
		result->status = status;

	return status;
}

static void
cp_playout_run_result_clear(struct cp_playout_run_result *result)
{
	if (result == NULL)
		return;

	result->status = CP_PLAYOUT_OK;
	result->restart_requested = 0;
	result->requested_output_device = CP_AUDIO_DEFAULT_DEVICE;
}

static void
cp_playout_print_meters(const struct cp_monitor_snapshot *snapshot)
{
	size_t band;

	if (snapshot == NULL)
		return;

	printf("input_peak=%0.6f input_rms=%0.6f output_peak=%0.6f "
	    "output_rms=%0.6f gain=%0.6f gain_db=%0.2f agc_state=%s\n",
	    cp_monitor_level_to_sample(snapshot->input_peak),
	    cp_monitor_level_to_sample(snapshot->input_rms),
	    cp_monitor_level_to_sample(snapshot->output_peak),
	    cp_monitor_level_to_sample(snapshot->output_rms),
	    cp_monitor_level_to_sample(snapshot->agc_gain),
	    cp_monitor_centibel_to_db(snapshot->agc_gain_db_centibel),
	    cp_agc_state_string((enum cp_agc_gate_state)snapshot->agc_state));
	if (snapshot->band_count > 0) {
		for (band = 0; band < snapshot->band_count; band++) {
			printf("band%zu_rms=%0.6f band%zu_gr_db=%0.2f\n",
			    band + 1,
			    cp_monitor_level_to_sample(snapshot->band_rms[band]),
			    band + 1,
			    cp_monitor_centibel_to_db(
			    snapshot->band_gr_db_centibel[band]));
		}
	}
	if (snapshot->band2_count > 0) {
		for (band = 0; band < snapshot->band2_count; band++) {
			printf("band2_%zu_rms=%0.6f band2_%zu_gr_db=%0.2f\n",
			    band + 1,
			    cp_monitor_level_to_sample(
			    snapshot->band2_rms[band]),
			    band + 1,
			    cp_monitor_centibel_to_db(
			    snapshot->band2_gr_db_centibel[band]));
		}
	}
	if (snapshot->auto_eq_enabled) {
		printf("auto_eq=on source=%s total_rms=%0.6f "
		    "tilt_db=%0.2f low=%0.6f presence=%0.6f high=%0.6f "
		    "finite=%s\n",
		    cp_auto_eq_source_hint_string(
		    (enum cp_auto_eq_source_hint)snapshot->auto_eq_source_hint),
		    cp_monitor_level_to_sample(snapshot->auto_eq_total_rms),
		    cp_monitor_centibel_to_db(
		    snapshot->auto_eq_spectral_tilt_db_centibel),
		    cp_monitor_level_to_sample(snapshot->auto_eq_low_weight),
		    cp_monitor_level_to_sample(
		    snapshot->auto_eq_presence_weight),
		    cp_monitor_level_to_sample(snapshot->auto_eq_high_weight),
		    snapshot->auto_eq_finite ? "yes" : "no");
		for (band = 0; band < CP_MONITOR_AUTO_EQ_BANDS; band++) {
			printf("auto_eq_band%zu_rms=%0.6f "
			    "auto_eq_band%zu_relative_db=%0.2f enabled=%s\n",
			    band + 1,
			    cp_monitor_level_to_sample(
			    snapshot->auto_eq_band_rms[band]),
			    band + 1,
			    cp_monitor_centibel_to_db(
			    snapshot->auto_eq_band_relative_db_centibel[band]),
			    snapshot->auto_eq_band_enabled[band] ? "yes" :
			    "no");
		}
		printf("bass_eq_recommend=%s preset=%s bass_gain_db=%0.2f "
		    "presence_gain_db=%0.2f output_gain_db=%0.2f "
		    "confidence=%0.6f source=%s\n",
		    snapshot->bass_eq_recommend_valid ? "valid" : "invalid",
		    cp_bass_eq_preset_string(
		    (enum cp_bass_eq_preset)
		    snapshot->bass_eq_recommend_preset),
		    cp_monitor_centibel_to_db(
		    snapshot->bass_eq_recommend_low_gain_db_centibel),
		    cp_monitor_centibel_to_db(
		    snapshot->bass_eq_recommend_high_gain_db_centibel),
		    cp_monitor_centibel_to_db(
		    snapshot->bass_eq_recommend_output_gain_db_centibel),
		    cp_monitor_level_to_sample(
		    snapshot->bass_eq_recommend_confidence),
		    cp_auto_eq_source_hint_string(
		    (enum cp_auto_eq_source_hint)
		    snapshot->bass_eq_recommend_source_hint));
	}
	if (snapshot->am_enabled) {
		printf("am=on preset=%s highpass=%u lowpass=%u "
		    "positive_peak=%0.2f negative_peak=%0.2f "
		    "asymmetry=%s asymmetry_ratio=%0.2f\n",
		    cp_am_preset_string((enum cp_am_preset)snapshot->am_preset),
		    snapshot->am_highpass_hz, snapshot->am_lowpass_hz,
		    cp_monitor_level_to_sample(snapshot->am_positive_peak),
		    cp_monitor_level_to_sample(snapshot->am_negative_peak),
		    snapshot->am_asymmetry_enabled ? "on" : "off",
		    cp_monitor_level_to_sample(snapshot->am_asymmetry_ratio));
	}
	if (snapshot->restoration_enabled) {
		printf("analysis_profile=%s analysis_reason_flags=0x%08x "
		    "analysis_clip_ratio=%0.6f analysis_hf_ratio=%0.6f "
		    "analysis_clip_confidence=%0.6f "
		    "analysis_low_ceiling_confidence=%0.6f "
		    "analysis_transient_confidence=%0.6f "
		    "analysis_lossy_confidence=%0.6f flat_ratio=%0.6f "
		    "peak_repeat_ratio=%0.6f analysis_peak=%0.6f "
		    "analysis_crest=%0.6f flat_runs=%u peak_repeats=%u\n",
		    cp_restoration_source_profile_string(
		    (enum cp_restoration_source_profile)
		    snapshot->restoration_source_profile),
		    snapshot->restoration_reason_flags,
		    cp_monitor_level_to_sample(
		    snapshot->restoration_clipped_ratio),
		    cp_monitor_level_to_sample(snapshot->restoration_hf_ratio),
		    cp_monitor_level_to_sample(
		    snapshot->restoration_clipping_confidence),
		    cp_monitor_level_to_sample(
		    snapshot->restoration_low_ceiling_confidence),
		    cp_monitor_level_to_sample(
		    snapshot->restoration_transient_confidence),
		    cp_monitor_level_to_sample(
		    snapshot->restoration_lossy_confidence),
		    cp_monitor_level_to_sample(
		    snapshot->restoration_flat_run_ratio),
		    cp_monitor_level_to_sample(
		    snapshot->restoration_peak_repeat_ratio),
		    cp_monitor_level_to_sample(
		    snapshot->restoration_observed_peak),
		    cp_monitor_level_to_sample(
		    snapshot->restoration_crest_factor),
		    snapshot->restoration_flat_runs,
		    snapshot->restoration_peak_repeats);
	}
	if (snapshot->declipper_enabled) {
		printf("declipper=on repaired_samples=%u repaired_runs=%u "
		    "max_delta=%0.6f bypass=%s finite=%s\n",
		    snapshot->declipper_repaired_samples,
		    snapshot->declipper_repaired_runs,
		    cp_monitor_level_to_sample(snapshot->declipper_max_delta),
		    cp_declipper_bypass_reason_string(
		    (enum cp_declipper_bypass_reason)
		    snapshot->declipper_bypass_reason),
		    snapshot->declipper_finite ? "yes" : "no");
	}
	if (snapshot->natural_dynamics_enabled) {
		printf("natural_dynamics=on rms=%0.6f gain=%0.6f "
		    "gain_reduction_db=%0.2f\n",
		    cp_monitor_level_to_sample(
		    snapshot->natural_dynamics_rms),
		    cp_monitor_level_to_sample(
		    snapshot->natural_dynamics_gain),
		    cp_monitor_centibel_to_db(
		    snapshot->natural_dynamics_gr_db_centibel));
	}
	if (snapshot->low_level_boost_enabled) {
		printf("low_level_boost=on rms=%0.6f gain=%0.6f "
		    "gain_db=%0.2f state=%s\n",
		    cp_monitor_level_to_sample(snapshot->low_level_boost_rms),
		    cp_monitor_level_to_sample(snapshot->low_level_boost_gain),
		    cp_monitor_centibel_to_db(
		    snapshot->low_level_boost_gain_db_centibel),
		    cp_agc_state_string((enum cp_agc_gate_state)
		    snapshot->low_level_boost_state));
	}
	if (snapshot->ssb_enabled) {
		printf("ssb=on preset=%s highpass=%u lowpass=%u "
		    "peak=%0.2f phase_rotator=%s\n",
		    cp_ssb_preset_string(
		    (enum cp_ssb_preset)snapshot->ssb_preset),
		    snapshot->ssb_highpass_hz, snapshot->ssb_lowpass_hz,
		    cp_monitor_level_to_sample(snapshot->ssb_peak_limit),
		    snapshot->ssb_phase_rotator_enabled ? "on" : "off");
	}
}

static int
cp_playout_select_output_device(const struct cp_audio_config *config,
	size_t channels, PaDeviceIndex *output_device)
{
	PaDeviceIndex count;
	PaDeviceIndex device;
	PaDeviceIndex selected;

	if (config == NULL || output_device == NULL)
		return CP_PLAYOUT_ERR_NULL;

	if (config->output_device != CP_AUDIO_DEFAULT_DEVICE) {
		if (!cp_playout_output_matches(config,
		    (PaDeviceIndex)config->output_device, channels))
			return CP_PLAYOUT_ERR_AUDIO;
		*output_device = (PaDeviceIndex)config->output_device;
		return CP_PLAYOUT_OK;
	}

	selected = Pa_GetDefaultOutputDevice();
	if (config->backend == CP_AUDIO_BACKEND_DEFAULT &&
	    selected != paNoDevice &&
	    cp_playout_output_matches(config, selected, channels)) {
		*output_device = selected;
		return CP_PLAYOUT_OK;
	}

	count = Pa_GetDeviceCount();
	if (count < 0)
		return CP_PLAYOUT_ERR_AUDIO;

	if (config->device_name != NULL ||
	    config->backend != CP_AUDIO_BACKEND_DEFAULT) {
		if (config->backend == CP_AUDIO_BACKEND_AUTO) {
			for (device = 0; device < count; device++) {
				if (cp_playout_output_matches(config, device,
				    channels) &&
				    cp_playout_output_device_contains(device,
				    "jack")) {
					*output_device = device;
					return CP_PLAYOUT_OK;
				}
			}
			for (device = 0; device < count; device++) {
				if (cp_playout_output_matches(config, device,
				    channels) &&
				    (cp_playout_output_device_contains(device,
				    "pipewire") ||
				    cp_playout_output_device_contains(device,
				    "pulse"))) {
					*output_device = device;
					return CP_PLAYOUT_OK;
				}
			}
		}
		for (device = 0; device < count; device++) {
			if (cp_playout_output_matches(config, device, channels)) {
				*output_device = device;
				return CP_PLAYOUT_OK;
			}
		}
	}

	if (selected != paNoDevice &&
	    cp_playout_output_matches(config, selected, channels)) {
		*output_device = selected;
		return CP_PLAYOUT_OK;
	}

	return CP_PLAYOUT_ERR_AUDIO;
}

static int
cp_playout_should_stop(const struct cp_playout_config *config)
{
	if (config == NULL || config->stop_requested == NULL)
		return 0;

	return *config->stop_requested != 0;
}

static int
cp_playout_status_enabled(const struct cp_audio_config *config)
{
	if (config == NULL)
		return 0;

	return !config->tui_enabled && !config->gui_enabled;
}

static void
cp_playout_print_line(const char *line)
{
	if (line == NULL)
		return;

	printf("%s\n", line);
}

static char *
cp_playout_trim_line(char *line)
{
	char *start;
	char *end;

	start = line;
	while (*start != '\0' && isspace((unsigned char)*start))
		start++;

	end = start + strlen(start);
	while (end > start && isspace((unsigned char)end[-1]))
		*--end = '\0';

	return start;
}

#ifdef CP_WITH_GUI
static void
cp_playout_workflow_reason(struct cp_gui_workflow_request *request,
	int validation_status, const char *reason)
{
	int written;

	if (request == NULL)
		return;

	request->validated = 1;
	request->validation_status = validation_status;
	if (reason == NULL)
		return;

	written = snprintf(request->reason, sizeof(request->reason), "%s",
	    reason);
	if (written < 0 || (size_t)written >= sizeof(request->reason))
		request->reason[sizeof(request->reason) - 1] = '\0';
}
#endif

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

#include "cp_control.h"
#include "cp_playout.h"
#include "cp_resampler.h"
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
static int	cp_playout_output_device_contains(PaDeviceIndex, const char *);
static int	cp_playout_output_matches(const struct cp_audio_config *,
		    PaDeviceIndex, size_t);
static int	cp_playout_rate_valid(double);
static int	cp_playout_rates_equal(double, double);
static int	cp_playout_read_line(FILE *, char *, size_t, int *);
static int	cp_playout_select_output_device(
		    const struct cp_audio_config *, size_t, PaDeviceIndex *);
static int	cp_playout_should_stop(const struct cp_playout_config *);
static void	cp_playout_print_meters(const struct cp_monitor_snapshot *);
static char	*cp_playout_trim_line(char *);

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
	config->block_frames = CP_PLAYOUT_DEFAULT_BLOCK_FRAMES;
	config->meter_interval_ms = CP_AUDIO_DEFAULT_METER_MS;
	config->stop_requested = NULL;
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
	struct cp_resampler resampler;
	struct cp_resampler_config resampler_config;
#ifdef CP_WITH_TUI
	struct cp_control_command command;
	struct cp_tui tui;
	struct cp_tui_view tui_view;
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
	int result;
	int resampling;
	int status;

	if (path == NULL || config == NULL)
		return CP_PLAYOUT_ERR_NULL;
	status = cp_playout_validate_config(config);
	if (status != CP_PLAYOUT_OK)
		return status;
	if (!cp_playout_path_is_wav(path))
		return CP_PLAYOUT_ERR_UNSUPPORTED;
#ifndef CP_WITH_TUI
	if (config->audio_config.tui_enabled)
		return CP_PLAYOUT_ERR_AUDIO;
#endif

	memset(&input_info, 0, sizeof(input_info));
	input_file = sf_open(path, SFM_READ, &input_info);
	if (input_file == NULL)
		return CP_PLAYOUT_ERR_OPEN_IN;
	if (input_info.channels != CP_CHANNELS_MONO &&
	    input_info.channels != CP_CHANNELS_STEREO) {
		sf_close(input_file);
		return CP_PLAYOUT_ERR_CHANNELS;
	}
	if (input_info.samplerate < CP_AUDIO_MIN_SAMPLE_RATE ||
	    input_info.samplerate > CP_AUDIO_MAX_SAMPLE_RATE) {
		sf_close(input_file);
		return CP_PLAYOUT_ERR_FORMAT;
	}

	channels     = (size_t)input_info.channels;
	block_frames = config->block_frames;
	if (block_frames > (SIZE_MAX / channels)) {
		sf_close(input_file);
		return CP_PLAYOUT_ERR_FORMAT;
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
	audio_config.sample_rate = output_rate;
	audio_config.sample_rate_explicit = 1;
	audio_config.channels = channels;
	audio_config.bass_eq_config.sample_rate = (cp_sample_t)output_rate;
	audio_config.bass_eq_config.channel_count = channels;
	audio_config.am_config.sample_rate = (cp_sample_t)output_rate;
	audio_config.am_config.channel_count = channels;
	audio_config.ssb_config.sample_rate = (cp_sample_t)output_rate;
	audio_config.ssb_config.channel_count = channels;
	status = cp_audio_validate_config(&audio_config);
	if (status != CP_AUDIO_OK) {
		sf_close(input_file);
		return CP_PLAYOUT_ERR_AUDIO;
	}

	error = Pa_Initialize();
	if (error != paNoError) {
		sf_close(input_file);
		return CP_PLAYOUT_ERR_AUDIO;
	}
	status = cp_playout_select_output_device(&audio_config, channels,
	    &output_device);
	if (status != CP_PLAYOUT_OK) {
		Pa_Terminate();
		sf_close(input_file);
		return status;
	}

	output_info = Pa_GetDeviceInfo(output_device);
	if (output_info == NULL) {
		Pa_Terminate();
		sf_close(input_file);
		return CP_PLAYOUT_ERR_AUDIO;
	}

	output_params.device = output_device;
	output_params.channelCount = (int)channels;
	output_params.sampleFormat = paFloat32;
	output_params.suggestedLatency = output_info->defaultLowOutputLatency;
	output_params.hostApiSpecificStreamInfo = NULL;

	status = cp_playout_open_output_stream(&stream, &output_params,
	    output_rate, block_frames);
	if (status != CP_PLAYOUT_OK &&
	    !config->audio_config.sample_rate_explicit &&
	    cp_playout_rate_valid(output_info->defaultSampleRate) &&
	    !cp_playout_rates_equal(output_rate,
	    output_info->defaultSampleRate)) {
		output_rate = output_info->defaultSampleRate;
		audio_config.sample_rate = output_rate;
		audio_config.bass_eq_config.sample_rate =
		    (cp_sample_t)output_rate;
		audio_config.am_config.sample_rate = (cp_sample_t)output_rate;
		audio_config.ssb_config.sample_rate = (cp_sample_t)output_rate;
		if (cp_audio_validate_config(&audio_config) != CP_AUDIO_OK) {
			Pa_Terminate();
			sf_close(input_file);
			return CP_PLAYOUT_ERR_AUDIO;
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
	error = Pa_StartStream(stream);
	if (error != paNoError) {
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

	if (!audio_config.tui_enabled) {
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
		status = cp_block_process(&processor, process_input, output,
		    scratch, output_block_samples, process_frames);
		if (status != CP_OK) {
			result = CP_PLAYOUT_ERR_PROCESS;
			break;
		}
		error = Pa_WriteStream(stream, output,
		    (unsigned long)process_frames);
		if (error != paNoError) {
			result = CP_PLAYOUT_ERR_WRITE;
			break;
		}
		status = cp_playout_build_snapshot(&processor, &snapshot);
		if (status != CP_PLAYOUT_OK) {
			result = status;
			break;
		}
#ifdef CP_WITH_TUI
		if (audio_config.tui_enabled) {
			memset(&tui_view, 0, sizeof(tui_view));
			tui_view.mode           = CP_TUI_MODE_PLAYOUT;
			tui_view.config         = &audio_config;
			tui_view.snapshot       = &snapshot;
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
	if (result == CP_PLAYOUT_OK && !audio_config.tui_enabled) {
		status = cp_playout_build_snapshot(&processor, &snapshot);
		if (status == CP_PLAYOUT_OK)
			cp_playout_print_meters(&snapshot);
	}

	Pa_StopStream(stream);
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

	return result;
}

int
cp_playout_run_playlist(const char *path,
	const struct cp_playout_config *config)
{
	struct cp_playout_config file_config;
	struct cp_playlist_error error;
	struct cp_playlist playlist;
	const char *entry;
	size_t index;
	int status;

	if (path == NULL || config == NULL)
		return CP_PLAYOUT_ERR_NULL;

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
		return status;
	}

	if (playlist.count == 0) {
		cp_playlist_free(&playlist);
		return CP_PLAYOUT_ERR_PLAYLIST;
	}

	for (index = 0; index < playlist.count; index++) {
		if (cp_playout_should_stop(config))
			break;
		entry = cp_playlist_get(&playlist, index);
		file_config = *config;
		file_config.playlist_index = index;
		file_config.playlist_count = playlist.count;
		status = cp_playout_run_file(entry, &file_config);
		if (status == CP_PLAYOUT_NEXT) {
			status = CP_PLAYOUT_OK;
			continue;
		}
		if (status != CP_PLAYOUT_OK)
			break;
	}

	cp_playlist_free(&playlist);
	return status;
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
		return CP_PLAYOUT_ERR_AUDIO;

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
		printf("analysis_clip_ratio=%0.6f analysis_hf_ratio=%0.6f "
		    "analysis_clip_confidence=%0.6f "
		    "analysis_lossy_confidence=%0.6f flat_runs=%u "
		    "peak_repeats=%u\n",
		    cp_monitor_level_to_sample(
		    snapshot->restoration_clipped_ratio),
		    cp_monitor_level_to_sample(snapshot->restoration_hf_ratio),
		    cp_monitor_level_to_sample(
		    snapshot->restoration_clipping_confidence),
		    cp_monitor_level_to_sample(
		    snapshot->restoration_lossy_confidence),
		    snapshot->restoration_flat_runs,
		    snapshot->restoration_peak_repeats);
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

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_playout.c */

#include <sys/types.h>

#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <portaudio.h>
#include <sndfile.h>

#include "cp_playout.h"

static int	cp_playout_append_path(struct cp_playlist *, const char *);
static char	*cp_playout_dup_range(const char *, size_t);
static int	cp_playout_name_contains(const char *, const char *);
static int	cp_playout_output_device_contains(PaDeviceIndex, const char *);
static int	cp_playout_output_matches(const struct cp_audio_config *,
		    PaDeviceIndex, size_t);
static int	cp_playout_read_line(FILE *, char *, size_t, int *);
static int	cp_playout_select_output_device(
		    const struct cp_audio_config *, size_t, PaDeviceIndex *);
static char	*cp_playout_trim_line(char *);

void
cp_playout_default_config(struct cp_playout_config *config)
{
	if (config == NULL)
		return;

	cp_audio_default_config(&config->audio_config);
	cp_block_default_config(&config->block_config, CP_CHANNELS_STEREO);
	config->block_frames = CP_PLAYOUT_DEFAULT_BLOCK_FRAMES;
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
	FILE *file;
	char line[CP_PLAYOUT_MAX_LINE];
	char *entry;
	int truncated;
	int status;

	if (path == NULL || playlist == NULL)
		return CP_PLAYOUT_ERR_NULL;

	memset(playlist, 0, sizeof(*playlist));
	file = fopen(path, "r");
	if (file == NULL)
		return CP_PLAYOUT_ERR_PLAYLIST;

	status = CP_PLAYOUT_OK;
	while (cp_playout_read_line(file, line, sizeof(line), &truncated)) {
		if (truncated) {
			status = CP_PLAYOUT_ERR_PLAYLIST;
			break;
		}
		entry = cp_playout_trim_line(line);
		if (entry[0] == '\0' || entry[0] == '#')
			continue;
		if (!cp_playout_path_is_wav(entry)) {
			status = CP_PLAYOUT_ERR_UNSUPPORTED;
			break;
		}
		status = cp_playout_append_path(playlist, entry);
		if (status != CP_PLAYOUT_OK)
			break;
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
	SF_INFO input_info;
	SNDFILE *input_file;
	PaStreamParameters output_params;
	PaStream *stream;
	PaDeviceIndex output_device;
	PaError error;
	const PaDeviceInfo *output_info;
	cp_sample_t *input;
	cp_sample_t *output;
	cp_sample_t *scratch;
	struct cp_audio_config audio_config;
	struct cp_block_config block_config;
	struct cp_block_processor processor;
	sf_count_t frames_read;
	size_t block_frames;
	size_t block_samples;
	size_t channels;
	int result;
	int status;

	if (path == NULL || config == NULL)
		return CP_PLAYOUT_ERR_NULL;
	if (!cp_playout_path_is_wav(path))
		return CP_PLAYOUT_ERR_UNSUPPORTED;
	if (config->block_frames == 0 ||
	    config->block_frames > (SIZE_MAX / CP_CHANNELS_STEREO))
		return CP_PLAYOUT_ERR_FORMAT;

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

	if (config->audio_config.sample_rate_explicit &&
	    fabs(config->audio_config.sample_rate -
	    (double)input_info.samplerate) > 0.5) {
		sf_close(input_file);
		return CP_PLAYOUT_ERR_FORMAT;
	}

	audio_config = config->audio_config;
	audio_config.sample_rate = (double)input_info.samplerate;
	audio_config.sample_rate_explicit = 1;
	audio_config.channels = channels;
	audio_config.am_config.sample_rate =
	    (cp_sample_t)input_info.samplerate;
	audio_config.am_config.channel_count = channels;
	status = cp_audio_validate_config(&audio_config);
	if (status != CP_AUDIO_OK) {
		sf_close(input_file);
		return CP_PLAYOUT_ERR_AUDIO;
	}

	block_config = config->block_config;
	block_config.channels = channels;
	block_config.sample_rate = (cp_sample_t)input_info.samplerate;
	block_config.dehummer_enabled = audio_config.dehummer_enabled;
	block_config.hum_base_frequency = audio_config.hum_base_frequency;
	block_config.hum_harmonic_count = audio_config.hum_harmonic_count;
	block_config.hum_q_factor = audio_config.hum_q_factor;
	block_config.multiband_enabled = audio_config.multiband_enabled;
	block_config.multiband_band_count = audio_config.multiband_band_count;
	block_config.multiband_preset = audio_config.multiband_preset;
	block_config.am_config = audio_config.am_config;
	status = cp_block_init(&processor, &block_config);
	if (status != CP_OK) {
		sf_close(input_file);
		return CP_PLAYOUT_ERR_PROCESS;
	}

	input = calloc(block_samples, sizeof(*input));
	output = calloc(block_samples, sizeof(*output));
	scratch = calloc(block_samples, sizeof(*scratch));
	if (input == NULL || output == NULL || scratch == NULL) {
		free(input);
		free(output);
		free(scratch);
		sf_close(input_file);
		return CP_PLAYOUT_ERR_ALLOC;
	}

	error = Pa_Initialize();
	if (error != paNoError) {
		free(input);
		free(output);
		free(scratch);
		sf_close(input_file);
		return CP_PLAYOUT_ERR_AUDIO;
	}
	status = cp_playout_select_output_device(&audio_config, channels,
	    &output_device);
	if (status != CP_PLAYOUT_OK) {
		Pa_Terminate();
		free(input);
		free(output);
		free(scratch);
		sf_close(input_file);
		return status;
	}

	output_info = Pa_GetDeviceInfo(output_device);
	if (output_info == NULL) {
		Pa_Terminate();
		free(input);
		free(output);
		free(scratch);
		sf_close(input_file);
		return CP_PLAYOUT_ERR_AUDIO;
	}

	output_params.device = output_device;
	output_params.channelCount = (int)channels;
	output_params.sampleFormat = paFloat32;
	output_params.suggestedLatency = output_info->defaultLowOutputLatency;
	output_params.hostApiSpecificStreamInfo = NULL;

	error = Pa_OpenStream(&stream, NULL, &output_params,
	    (double)input_info.samplerate, (unsigned long)block_frames,
	    paNoFlag, NULL, NULL);
	if (error != paNoError) {
		Pa_Terminate();
		free(input);
		free(output);
		free(scratch);
		sf_close(input_file);
		return CP_PLAYOUT_ERR_STREAM;
	}
	error = Pa_StartStream(stream);
	if (error != paNoError) {
		Pa_CloseStream(stream);
		Pa_Terminate();
		free(input);
		free(output);
		free(scratch);
		sf_close(input_file);
		return CP_PLAYOUT_ERR_STREAM;
	}

	printf("playing: %s rate=%d channels=%zu output_device=%d\n",
	    path, input_info.samplerate, channels, output_device);

	result = CP_PLAYOUT_OK;
	while ((frames_read = sf_readf_float(input_file, input,
	    (sf_count_t)block_frames)) > 0) {
		status = cp_block_process(&processor, input, output, scratch,
		    block_samples, (size_t)frames_read);
		if (status != CP_OK) {
			result = CP_PLAYOUT_ERR_PROCESS;
			break;
		}
		error = Pa_WriteStream(stream, output,
		    (unsigned long)frames_read);
		if (error != paNoError) {
			result = CP_PLAYOUT_ERR_WRITE;
			break;
		}
	}
	if (frames_read < 0 && result == CP_PLAYOUT_OK)
		result = CP_PLAYOUT_ERR_READ;

	Pa_StopStream(stream);
	Pa_CloseStream(stream);
	Pa_Terminate();
	free(input);
	free(output);
	free(scratch);
	sf_close(input_file);

	return result;
}

int
cp_playout_run_playlist(const char *path,
	const struct cp_playout_config *config)
{
	struct cp_playlist playlist;
	const char *entry;
	size_t index;
	int status;

	if (path == NULL || config == NULL)
		return CP_PLAYOUT_ERR_NULL;

	status = cp_playlist_load(path, &playlist);
	if (status != CP_PLAYOUT_OK)
		return status;

	if (playlist.count == 0) {
		cp_playlist_free(&playlist);
		return CP_PLAYOUT_ERR_PLAYLIST;
	}

	for (index = 0; index < playlist.count; index++) {
		entry = cp_playlist_get(&playlist, index);
		status = cp_playout_run_file(entry, config);
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
	default:
		return "unknown playout error";
	}
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

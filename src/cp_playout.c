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

#include "cp_control.h"
#include "cp_playout.h"
#ifdef CP_WITH_TUI
#include "cp_tui.h"
#endif

static size_t	cp_playout_interval_frames(double, unsigned int);
static int	cp_playout_append_path(struct cp_playlist *, const char *);
static int	cp_playout_am_preset_id(const struct cp_am_config *);
static char	*cp_playout_dup_range(const char *, size_t);
static int	cp_playout_name_contains(const char *, const char *);
static int	cp_playout_output_device_contains(PaDeviceIndex, const char *);
static int	cp_playout_output_matches(const struct cp_audio_config *,
		    PaDeviceIndex, size_t);
static int	cp_playout_read_line(FILE *, char *, size_t, int *);
static int	cp_playout_select_output_device(
		    const struct cp_audio_config *, size_t, PaDeviceIndex *);
static int	cp_playout_should_stop(const struct cp_playout_config *);
static int	cp_playout_ssb_preset_id(const struct cp_ssb_config *);
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
	size_t band;

	if (processor == NULL || snapshot == NULL)
		return CP_PLAYOUT_ERR_NULL;

	cp_monitor_snapshot_clear(snapshot);
	snapshot->input_peak =
	    cp_monitor_sample_to_level(processor->input_meter.peak[0]);
	snapshot->input_rms =
	    cp_monitor_sample_to_level(processor->input_meter.rms[0]);
	snapshot->output_peak =
	    cp_monitor_sample_to_level(processor->output_meter.peak[0]);
	snapshot->output_rms =
	    cp_monitor_sample_to_level(processor->output_meter.rms[0]);
	snapshot->agc_gain = cp_monitor_sample_to_level(processor->agc.gain);
	snapshot->agc_gain_db_centibel =
	    cp_monitor_db_to_centibel(processor->agc.gain_db);
	snapshot->agc_state = (int)processor->agc.gate_state;
	snapshot->dsp_status = CP_OK;
	snapshot->am_enabled = processor->am.config.enabled ? 1u : 0u;
	snapshot->am_highpass_hz =
	    (unsigned int)lrintf(processor->am.config.highpass_hz);
	snapshot->am_lowpass_hz =
	    (unsigned int)lrintf(processor->am.config.lowpass_hz);
	snapshot->am_positive_peak =
	    cp_monitor_sample_to_level(processor->am.config.positive_peak_limit);
	snapshot->am_negative_peak =
	    cp_monitor_sample_to_level(processor->am.config.negative_peak_limit);
	snapshot->am_asymmetry_enabled =
	    processor->am.config.asymmetry_enabled ? 1u : 0u;
	snapshot->am_asymmetry_ratio =
	    cp_monitor_sample_to_level(processor->am.config.asymmetry_ratio);
	snapshot->am_preset = cp_playout_am_preset_id(&processor->am.config);
	snapshot->ssb_enabled = processor->ssb.config.enabled ? 1u : 0u;
	snapshot->ssb_highpass_hz =
	    (unsigned int)lrintf(processor->ssb.config.highpass_hz);
	snapshot->ssb_lowpass_hz =
	    (unsigned int)lrintf(processor->ssb.config.lowpass_hz);
	snapshot->ssb_peak_limit =
	    cp_monitor_sample_to_level(processor->ssb.config.peak_limit);
	snapshot->ssb_phase_rotator_enabled =
	    processor->ssb.config.phase_rotator_enabled ? 1u : 0u;
	snapshot->ssb_preset = cp_playout_ssb_preset_id(&processor->ssb.config);
	snapshot->band_count = processor->multiband.band_count;
	if (snapshot->band_count > CP_MONITOR_MAX_BANDS)
		snapshot->band_count = CP_MONITOR_MAX_BANDS;
	for (band = 0; band < snapshot->band_count; band++) {
		snapshot->band_rms[band] = cp_monitor_sample_to_level(
		    processor->multiband.band_rms[band]);
		snapshot->band_gr_db_centibel[band] = cp_monitor_db_to_centibel(
		    processor->multiband.band_gain_reduction_db[band]);
	}

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
	cp_sample_t *scratch;
	struct cp_audio_config audio_config;
	struct cp_block_config block_config;
	struct cp_block_processor processor;
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
	int result;
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
	audio_config.ssb_config.sample_rate =
	    (cp_sample_t)input_info.samplerate;
	audio_config.ssb_config.channel_count = channels;
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
	block_config.ssb_config = audio_config.ssb_config;
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
#ifdef CP_WITH_TUI
	tui.active = 0;
	if (audio_config.tui_enabled && cp_tui_init(&tui) != 0) {
		Pa_CloseStream(stream);
		Pa_Terminate();
		free(input);
		free(output);
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
		free(scratch);
		sf_close(input_file);
		return CP_PLAYOUT_ERR_STREAM;
	}

	if (!audio_config.tui_enabled) {
		printf("playing: %s rate=%d channels=%zu output_device=%d\n",
		    path, input_info.samplerate, channels, output_device);
	}

	result = CP_PLAYOUT_OK;
	meter_frames = cp_playout_interval_frames((double)input_info.samplerate,
	    config->meter_interval_ms);
	frames_until_meter = meter_frames;
	while (!cp_playout_should_stop(config) &&
	    (frames_read = sf_readf_float(input_file, input,
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
		if ((size_t)frames_read >= frames_until_meter) {
			cp_playout_print_meters(&snapshot);
			frames_until_meter = meter_frames;
		} else {
			frames_until_meter -= (size_t)frames_read;
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
	free(scratch);
	sf_close(input_file);

	return result;
}

int
cp_playout_run_playlist(const char *path,
	const struct cp_playout_config *config)
{
	struct cp_playout_config file_config;
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

static int
cp_playout_am_preset_id(const struct cp_am_config *config)
{
	enum cp_am_preset preset;

	if (config == NULL)
		return (int)CP_AM_PRESET_SAFE;
	if (cp_am_preset_from_string(config->preset_name, &preset) == CP_OK)
		return (int)preset;

	return (int)CP_AM_PRESET_SAFE;
}

static int
cp_playout_ssb_preset_id(const struct cp_ssb_config *config)
{
	enum cp_ssb_preset preset;

	if (config == NULL)
		return (int)CP_SSB_PRESET_SPEECH;
	if (cp_ssb_preset_from_string(config->preset_name, &preset) == CP_OK)
		return (int)preset;

	return (int)CP_SSB_PRESET_SPEECH;
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

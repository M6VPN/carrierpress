/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_portaudio.c */

#include <sys/types.h>

#include <math.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <portaudio.h>

#include "cp_block.h"
#include "cp_control.h"
#include "cp_monitor.h"
#include "cp_portaudio.h"
#ifdef CP_WITH_TUI
#include "cp_tui.h"
#endif

#define CP_PA_SLEEP_MS		50

struct cp_portaudio_runtime {
	struct cp_block_processor processor;
	cp_sample_t *scratch;
	cp_sample_t *zero_input;
	size_t channels;
	size_t block_size;
	atomic_uint input_peak;
	atomic_uint input_rms;
	atomic_uint output_peak;
	atomic_uint output_rms;
	atomic_uint agc_gain;
	atomic_int agc_gain_db_centibel;
	atomic_int agc_state;
	atomic_uint dehummer_enabled;
	atomic_uint dehummer_base_hz;
	atomic_uint dehummer_harmonic_count;
	atomic_uint multiband_enabled;
	atomic_int multiband_preset;
	atomic_uint band_count;
	atomic_uint band_rms[CP_MONITOR_MAX_BANDS];
	atomic_int band_gr_db_centibel[CP_MONITOR_MAX_BANDS];
	atomic_uint am_enabled;
	atomic_uint am_highpass_hz;
	atomic_uint am_lowpass_hz;
	atomic_uint am_positive_peak;
	atomic_uint am_negative_peak;
	atomic_uint am_asymmetry_enabled;
	atomic_uint am_asymmetry_ratio;
	atomic_int am_preset;
	atomic_uint ssb_enabled;
	atomic_uint ssb_highpass_hz;
	atomic_uint ssb_lowpass_hz;
	atomic_uint ssb_peak_limit;
	atomic_uint ssb_phase_rotator_enabled;
	atomic_int ssb_preset;
	atomic_int pending_control_type;
	atomic_int pending_am_preset;
	atomic_int pending_ssb_preset;
	atomic_int control_command;
	atomic_int control_status;
	atomic_uint status_flags;
	atomic_int dsp_status;
};

static void		cp_pa_apply_pending_control(
			    struct cp_portaudio_runtime *);
static int		cp_pa_build_candidates(
			    struct cp_audio_device_candidate **,
			    PaDeviceIndex *);
static unsigned int	cp_pa_flags_from_status(PaStreamCallbackFlags);
static const char	*cp_pa_host_api_name(PaDeviceIndex);
static unsigned int	cp_pa_hz_to_uint(cp_sample_t);
static int		cp_pa_init_processor(struct cp_portaudio_runtime *,
			    const struct cp_audio_config *, double);
static void		cp_pa_load_snapshot(struct cp_portaudio_runtime *,
			    struct cp_monitor_snapshot *);
static int		cp_pa_rate_supported(const PaStreamParameters *,
			    const PaStreamParameters *, double);
static int		cp_pa_select_sample_rate(
			    const struct cp_audio_config *,
			    const PaStreamParameters *,
			    const PaStreamParameters *,
			    const PaDeviceInfo *, const PaDeviceInfo *,
			    double *);
static void		cp_pa_print_flags(unsigned int);
static void		cp_pa_print_meters(const struct cp_monitor_snapshot *);
static void		cp_pa_print_recommendation(
			    const struct cp_audio_device_candidate *, size_t);
static int		cp_pa_select_devices(const struct cp_audio_config *,
			    PaDeviceIndex *, PaDeviceIndex *);
#ifdef CP_WITH_TUI
static void		cp_pa_store_control_request(
			    struct cp_portaudio_runtime *,
			    const struct cp_control_command *);
#endif
static void		cp_pa_store_meters(struct cp_portaudio_runtime *);
static int		cp_pa_stream_callback(const void *, void *,
			    unsigned long, const PaStreamCallbackTimeInfo *,
			    PaStreamCallbackFlags, void *);

int
cp_portaudio_list_devices(void)
{
	struct cp_audio_device_candidate *candidates;
	const PaDeviceInfo *info;
	PaDeviceIndex count;
	PaDeviceIndex device;
	PaError error;

	candidates = NULL;
	error = Pa_Initialize();
	if (error != paNoError)
		return CP_PORTAUDIO_ERR_INIT;

	if (cp_pa_build_candidates(&candidates, &count) != CP_PORTAUDIO_OK) {
		Pa_Terminate();
		return CP_PORTAUDIO_ERR_DEVICE;
	}

	for (device = 0; device < count; device++) {
		info = Pa_GetDeviceInfo(device);
		if (info == NULL)
			continue;

		printf("%d: [%s] %s input=%d output=%d rate=%0.0f",
		    device, cp_pa_host_api_name(device), info->name,
		    info->maxInputChannels, info->maxOutputChannels,
		    info->defaultSampleRate);
		if (device == Pa_GetDefaultInputDevice())
			printf(" default_input");
		if (device == Pa_GetDefaultOutputDevice())
			printf(" default_output");
		if (candidates[device].max_input_channels >=
		    (int)CP_AUDIO_DEFAULT_CHANNELS &&
		    candidates[device].max_output_channels >=
		    (int)CP_AUDIO_DEFAULT_CHANNELS)
			printf(" live_candidate");
		printf("\n");
	}

	cp_pa_print_recommendation(candidates, (size_t)count);
	free(candidates);
	Pa_Terminate();
	return CP_PORTAUDIO_OK;
}

int
cp_portaudio_run(const struct cp_audio_config *config,
	volatile sig_atomic_t *stop_requested)
{
	PaStreamParameters input_params;
	PaStreamParameters output_params;
	PaStream *stream;
	PaDeviceIndex input_device;
	PaDeviceIndex output_device;
	PaError error;
	const PaDeviceInfo *input_info;
	const PaDeviceInfo *output_info;
	struct cp_portaudio_runtime runtime;
	struct cp_monitor_snapshot snapshot;
	double stream_rate;
#ifdef CP_WITH_TUI
	struct cp_control_command command;
	struct cp_tui tui;
#endif
	int status;

	status = cp_audio_validate_config(config);
	if (status != CP_AUDIO_OK)
		return CP_PORTAUDIO_ERR_CONFIG;
	if (stop_requested == NULL)
		return CP_PORTAUDIO_ERR_CONFIG;
#ifndef CP_WITH_TUI
	if (config->tui_enabled)
		return CP_PORTAUDIO_ERR_CONFIG;
#endif

	error = Pa_Initialize();
	if (error != paNoError)
		return CP_PORTAUDIO_ERR_INIT;

	status = cp_pa_select_devices(config, &input_device, &output_device);
	if (status != CP_PORTAUDIO_OK) {
		Pa_Terminate();
		return status;
	}

	input_info = Pa_GetDeviceInfo(input_device);
	output_info = Pa_GetDeviceInfo(output_device);
	if (input_info == NULL || output_info == NULL) {
		Pa_Terminate();
		return CP_PORTAUDIO_ERR_DEVICE;
	}

	input_params.device                    = input_device;
	input_params.channelCount              = (int)config->channels;
	input_params.sampleFormat              = paFloat32;
	input_params.suggestedLatency          = input_info->defaultLowInputLatency;
	input_params.hostApiSpecificStreamInfo = NULL;

	output_params.device                    = output_device;
	output_params.channelCount              = (int)config->channels;
	output_params.sampleFormat              = paFloat32;
	output_params.suggestedLatency          = output_info->defaultLowOutputLatency;
	output_params.hostApiSpecificStreamInfo = NULL;

	status = cp_pa_select_sample_rate(config, &input_params, &output_params,
	    input_info, output_info, &stream_rate);
	if (status != CP_PORTAUDIO_OK) {
		Pa_Terminate();
		return status;
	}

	status = cp_pa_init_processor(&runtime, config, stream_rate);
	if (status != CP_PORTAUDIO_OK) {
		Pa_Terminate();
		return status;
	}

	stream = NULL;
	error = Pa_OpenStream(&stream, &input_params, &output_params,
	    stream_rate, (unsigned long)config->block_size, paNoFlag,
	    cp_pa_stream_callback, &runtime);
	if (error != paNoError) {
		free(runtime.scratch);
		free(runtime.zero_input);
		Pa_Terminate();
		return CP_PORTAUDIO_ERR_STREAM;
	}

#ifdef CP_WITH_TUI
	tui.active = 0;
	if (config->tui_enabled && cp_tui_init(&tui) != 0) {
		Pa_CloseStream(stream);
		free(runtime.scratch);
		free(runtime.zero_input);
		Pa_Terminate();
		return CP_PORTAUDIO_ERR_CONFIG;
	}
#endif

	error = Pa_StartStream(stream);
	if (error != paNoError) {
#ifdef CP_WITH_TUI
		cp_tui_close(&tui);
#endif
		Pa_CloseStream(stream);
		free(runtime.scratch);
		free(runtime.zero_input);
		Pa_Terminate();
		return CP_PORTAUDIO_ERR_START;
	}

	if (!config->tui_enabled)
		printf("live audio started. input_device=%d output_device=%d "
		    "rate=%0.0f backend=%s. press Ctrl-C to stop.\n",
		    input_device, output_device, stream_rate,
		    cp_audio_backend_string(config->backend));
	while (!*stop_requested && Pa_IsStreamActive(stream) == 1) {
		Pa_Sleep(config->tui_enabled ? CP_PA_SLEEP_MS :
		    (long)config->meter_interval_ms);
		cp_pa_load_snapshot(&runtime, &snapshot);
#ifdef CP_WITH_TUI
		if (config->tui_enabled) {
			if (cp_tui_update(&tui, config, &snapshot, &command))
				*stop_requested = 1;
			if (command.type != CP_CONTROL_COMMAND_NONE &&
			    command.type != CP_CONTROL_COMMAND_STOP)
				cp_pa_store_control_request(&runtime,
				    &command);
		} else
#endif
		{
			cp_pa_print_meters(&snapshot);
			cp_pa_print_flags(snapshot.stream_flags);
		}

		if (snapshot.dsp_status != CP_OK)
			break;
	}

	error = Pa_StopStream(stream);
	if (error != paNoError)
		status = CP_PORTAUDIO_ERR_STOP;
	else
		status = CP_PORTAUDIO_OK;

#ifdef CP_WITH_TUI
	cp_tui_close(&tui);
#endif
	Pa_CloseStream(stream);
	free(runtime.scratch);
	free(runtime.zero_input);
	Pa_Terminate();

	if (atomic_load(&runtime.dsp_status) != CP_OK)
		return CP_PORTAUDIO_ERR_DSP;

	return status;
}

const char *
cp_portaudio_status_string(int status)
{
	switch (status) {
	case CP_PORTAUDIO_OK:
		return "ok";
	case CP_PORTAUDIO_ERR_CONFIG:
		return "invalid live audio config";
	case CP_PORTAUDIO_ERR_INIT:
		return "could not initialize PortAudio";
	case CP_PORTAUDIO_ERR_DEVICE:
		return "invalid or unavailable PortAudio device";
	case CP_PORTAUDIO_ERR_ALLOC:
		return "could not allocate live audio buffers";
	case CP_PORTAUDIO_ERR_DSP:
		return "DSP processing failed";
	case CP_PORTAUDIO_ERR_STREAM:
		return "could not open PortAudio stream";
	case CP_PORTAUDIO_ERR_START:
		return "could not start PortAudio stream";
	case CP_PORTAUDIO_ERR_STOP:
		return "could not stop PortAudio stream";
	default:
		return "unknown PortAudio error";
	}
}

static int
cp_pa_build_candidates(struct cp_audio_device_candidate **candidates,
	PaDeviceIndex *count)
{
	struct cp_audio_device_candidate *list;
	const PaDeviceInfo *info;
	PaDeviceIndex device;
	PaDeviceIndex devices;

	if (candidates == NULL || count == NULL)
		return CP_PORTAUDIO_ERR_DEVICE;

	devices = Pa_GetDeviceCount();
	if (devices < 0)
		return CP_PORTAUDIO_ERR_DEVICE;

	list = calloc((size_t)devices, sizeof(*list));
	if (list == NULL)
		return CP_PORTAUDIO_ERR_ALLOC;

	for (device = 0; device < devices; device++) {
		info = Pa_GetDeviceInfo(device);
		list[device].index = device;
		list[device].name = "";
		list[device].host_api = "";
		if (info == NULL)
			continue;
		list[device].name = info->name;
		list[device].host_api = cp_pa_host_api_name(device);
		list[device].max_input_channels = info->maxInputChannels;
		list[device].max_output_channels = info->maxOutputChannels;
		list[device].default_sample_rate = info->defaultSampleRate;
		list[device].default_input =
		    (device == Pa_GetDefaultInputDevice());
		list[device].default_output =
		    (device == Pa_GetDefaultOutputDevice());
	}

	*candidates = list;
	*count = devices;
	return CP_PORTAUDIO_OK;
}

static void
cp_pa_apply_pending_control(struct cp_portaudio_runtime *runtime)
{
	struct cp_control_command command;
	enum cp_control_command_type type;
	int status;

	type = (enum cp_control_command_type)atomic_exchange(
	    &runtime->pending_control_type, CP_CONTROL_COMMAND_NONE);
	if (type == CP_CONTROL_COMMAND_NONE)
		return;

	command.type = type;
	command.bank = type == CP_CONTROL_COMMAND_SSB_PRESET ||
	    type == CP_CONTROL_COMMAND_SSB_OFF ? CP_CONTROL_BANK_SSB :
	    CP_CONTROL_BANK_AM;
	command.am_preset = (enum cp_am_preset)atomic_load(
	    &runtime->pending_am_preset);
	command.ssb_preset = (enum cp_ssb_preset)atomic_load(
	    &runtime->pending_ssb_preset);
	status = cp_control_apply(&runtime->processor, &command);
	atomic_store(&runtime->control_command, (int)command.type);
	atomic_store(&runtime->control_status, status);
}

static unsigned int
cp_pa_flags_from_status(PaStreamCallbackFlags status_flags)
{
	unsigned int flags;

	flags = 0u;
	if ((status_flags & paInputUnderflow) != 0)
		flags |= CP_MONITOR_INPUT_UNDERFLOW;
	if ((status_flags & paInputOverflow) != 0)
		flags |= CP_MONITOR_INPUT_OVERFLOW;
	if ((status_flags & paOutputUnderflow) != 0)
		flags |= CP_MONITOR_OUTPUT_UNDERFLOW;
	if ((status_flags & paOutputOverflow) != 0)
		flags |= CP_MONITOR_OUTPUT_OVERFLOW;
	if ((status_flags & paPrimingOutput) != 0)
		flags |= CP_MONITOR_PRIMING_OUTPUT;

	return flags;
}

static const char *
cp_pa_host_api_name(PaDeviceIndex device)
{
	const PaDeviceInfo *device_info;
	const PaHostApiInfo *host_info;

	device_info = Pa_GetDeviceInfo(device);
	if (device_info == NULL)
		return "unknown";

	host_info = Pa_GetHostApiInfo(device_info->hostApi);
	if (host_info == NULL || host_info->name == NULL)
		return "unknown";

	return host_info->name;
}

static unsigned int
cp_pa_hz_to_uint(cp_sample_t frequency)
{
	if (!isfinite(frequency) || frequency <= 0.0f)
		return 0u;
	if (frequency >= (cp_sample_t)UINT32_MAX)
		return UINT32_MAX;

	return (unsigned int)lrintf(frequency);
}

static int
cp_pa_rate_supported(const PaStreamParameters *input_params,
	const PaStreamParameters *output_params, double sample_rate)
{
	PaError error;

	error = Pa_IsFormatSupported(input_params, output_params, sample_rate);
	return error == paFormatIsSupported;
}

static int
cp_pa_init_processor(struct cp_portaudio_runtime *runtime,
	const struct cp_audio_config *config, double sample_rate)
{
	struct cp_block_config block_config;
	size_t samples;
	size_t band;
	int status;

	if (config->block_size > (SIZE_MAX / config->channels))
		return CP_PORTAUDIO_ERR_CONFIG;

	samples = config->block_size * config->channels;

	runtime->scratch = calloc(samples, sizeof(*runtime->scratch));
	runtime->zero_input = calloc(samples, sizeof(*runtime->zero_input));
	if (runtime->scratch == NULL || runtime->zero_input == NULL) {
		free(runtime->scratch);
		free(runtime->zero_input);
		return CP_PORTAUDIO_ERR_ALLOC;
	}

	runtime->channels   = config->channels;
	runtime->block_size = config->block_size;
	atomic_init(&runtime->input_peak, 0u);
	atomic_init(&runtime->input_rms, 0u);
	atomic_init(&runtime->output_peak, 0u);
	atomic_init(&runtime->output_rms, 0u);
	atomic_init(&runtime->agc_gain, 0u);
	atomic_init(&runtime->agc_gain_db_centibel, 0);
	atomic_init(&runtime->agc_state, 0);
	atomic_init(&runtime->dehummer_enabled, 0u);
	atomic_init(&runtime->dehummer_base_hz, 0u);
	atomic_init(&runtime->dehummer_harmonic_count, 0u);
	atomic_init(&runtime->multiband_enabled, 0u);
	atomic_init(&runtime->multiband_preset,
	    (int)CP_MULTIBAND_PRESET_SPEECH);
	atomic_init(&runtime->band_count, 0u);
	for (band = 0; band < CP_MONITOR_MAX_BANDS; band++) {
		atomic_init(&runtime->band_rms[band], 0u);
		atomic_init(&runtime->band_gr_db_centibel[band], 0);
	}
	atomic_init(&runtime->am_enabled, 0u);
	atomic_init(&runtime->am_highpass_hz, 0u);
	atomic_init(&runtime->am_lowpass_hz, 0u);
	atomic_init(&runtime->am_positive_peak, 0u);
	atomic_init(&runtime->am_negative_peak, 0u);
	atomic_init(&runtime->am_asymmetry_enabled, 0u);
	atomic_init(&runtime->am_asymmetry_ratio, 0u);
	atomic_init(&runtime->am_preset, (int)CP_AM_PRESET_SAFE);
	atomic_init(&runtime->ssb_enabled, 0u);
	atomic_init(&runtime->ssb_highpass_hz, 0u);
	atomic_init(&runtime->ssb_lowpass_hz, 0u);
	atomic_init(&runtime->ssb_peak_limit, 0u);
	atomic_init(&runtime->ssb_phase_rotator_enabled, 0u);
	atomic_init(&runtime->ssb_preset, (int)CP_SSB_PRESET_SPEECH);
	atomic_init(&runtime->pending_control_type,
	    (int)CP_CONTROL_COMMAND_NONE);
	atomic_init(&runtime->pending_am_preset, (int)CP_AM_PRESET_SAFE);
	atomic_init(&runtime->pending_ssb_preset,
	    (int)CP_SSB_PRESET_SPEECH);
	atomic_init(&runtime->control_command,
	    (int)CP_CONTROL_COMMAND_NONE);
	atomic_init(&runtime->control_status, CP_OK);
	atomic_init(&runtime->status_flags, 0u);
	atomic_init(&runtime->dsp_status, CP_OK);

	cp_block_default_config(&block_config, config->channels);
	block_config.sample_rate = (cp_sample_t)sample_rate;
	block_config.dehummer_enabled = config->dehummer_enabled;
	block_config.hum_base_frequency = config->hum_base_frequency;
	block_config.hum_harmonic_count = config->hum_harmonic_count;
	block_config.hum_q_factor = config->hum_q_factor;
	block_config.multiband_enabled = config->multiband_enabled;
	block_config.multiband_band_count = config->multiband_band_count;
	block_config.multiband_preset = config->multiband_preset;
	block_config.am_config = config->am_config;
	block_config.am_config.sample_rate = (cp_sample_t)sample_rate;
	block_config.am_config.channel_count = config->channels;
	block_config.ssb_config = config->ssb_config;
	block_config.ssb_config.sample_rate = (cp_sample_t)sample_rate;
	block_config.ssb_config.channel_count = config->channels;
	status = cp_block_init(&runtime->processor, &block_config);
	if (status != CP_OK) {
		free(runtime->scratch);
		free(runtime->zero_input);
		return CP_PORTAUDIO_ERR_DSP;
	}

	cp_pa_store_meters(runtime);

	return CP_PORTAUDIO_OK;
}

static void
cp_pa_load_snapshot(struct cp_portaudio_runtime *runtime,
	struct cp_monitor_snapshot *snapshot)
{
	size_t band;
	unsigned int band_count;

	cp_monitor_snapshot_clear(snapshot);
	snapshot->input_peak = atomic_load(&runtime->input_peak);
	snapshot->input_rms = atomic_load(&runtime->input_rms);
	snapshot->output_peak = atomic_load(&runtime->output_peak);
	snapshot->output_rms = atomic_load(&runtime->output_rms);
	snapshot->agc_gain = atomic_load(&runtime->agc_gain);
	snapshot->agc_gain_db_centibel =
	    atomic_load(&runtime->agc_gain_db_centibel);
	snapshot->agc_state = atomic_load(&runtime->agc_state);
	snapshot->stream_flags = atomic_exchange(&runtime->status_flags, 0u);
	snapshot->dsp_status = atomic_load(&runtime->dsp_status);
	snapshot->dehummer_enabled =
	    atomic_load(&runtime->dehummer_enabled);
	snapshot->dehummer_base_hz =
	    atomic_load(&runtime->dehummer_base_hz);
	snapshot->dehummer_harmonic_count =
	    atomic_load(&runtime->dehummer_harmonic_count);
	snapshot->multiband_enabled =
	    atomic_load(&runtime->multiband_enabled);
	snapshot->multiband_preset =
	    atomic_load(&runtime->multiband_preset);
	snapshot->am_enabled = atomic_load(&runtime->am_enabled);
	snapshot->am_highpass_hz = atomic_load(&runtime->am_highpass_hz);
	snapshot->am_lowpass_hz = atomic_load(&runtime->am_lowpass_hz);
	snapshot->am_positive_peak = atomic_load(&runtime->am_positive_peak);
	snapshot->am_negative_peak = atomic_load(&runtime->am_negative_peak);
	snapshot->am_asymmetry_enabled =
	    atomic_load(&runtime->am_asymmetry_enabled);
	snapshot->am_asymmetry_ratio =
	    atomic_load(&runtime->am_asymmetry_ratio);
	snapshot->am_preset = atomic_load(&runtime->am_preset);
	snapshot->ssb_enabled = atomic_load(&runtime->ssb_enabled);
	snapshot->ssb_highpass_hz = atomic_load(&runtime->ssb_highpass_hz);
	snapshot->ssb_lowpass_hz = atomic_load(&runtime->ssb_lowpass_hz);
	snapshot->ssb_peak_limit = atomic_load(&runtime->ssb_peak_limit);
	snapshot->ssb_phase_rotator_enabled =
	    atomic_load(&runtime->ssb_phase_rotator_enabled);
	snapshot->ssb_preset = atomic_load(&runtime->ssb_preset);
	snapshot->control_command = atomic_load(&runtime->control_command);
	snapshot->control_status = atomic_load(&runtime->control_status);

	band_count = atomic_load(&runtime->band_count);
	if (band_count > CP_MONITOR_MAX_BANDS)
		band_count = CP_MONITOR_MAX_BANDS;
	snapshot->band_count = band_count;
	for (band = 0; band < snapshot->band_count; band++) {
		snapshot->band_rms[band] = atomic_load(&runtime->band_rms[band]);
		snapshot->band_gr_db_centibel[band] =
		    atomic_load(&runtime->band_gr_db_centibel[band]);
	}
}

static void
cp_pa_print_flags(unsigned int flags)
{
	if (flags == 0u)
		return;

	printf("stream_status:");
	if ((flags & CP_MONITOR_INPUT_UNDERFLOW) != 0)
		printf(" input_underflow");
	if ((flags & CP_MONITOR_INPUT_OVERFLOW) != 0)
		printf(" input_overflow");
	if ((flags & CP_MONITOR_OUTPUT_UNDERFLOW) != 0)
		printf(" output_underflow");
	if ((flags & CP_MONITOR_OUTPUT_OVERFLOW) != 0)
		printf(" output_overflow");
	if ((flags & CP_MONITOR_PRIMING_OUTPUT) != 0)
		printf(" priming_output");
	printf("\n");
}

static void
cp_pa_print_meters(const struct cp_monitor_snapshot *snapshot)
{
	printf("input_peak=%0.6f input_rms=%0.6f output_peak=%0.6f "
	    "output_rms=%0.6f\n",
	    cp_monitor_level_to_sample(snapshot->input_peak),
	    cp_monitor_level_to_sample(snapshot->input_rms),
	    cp_monitor_level_to_sample(snapshot->output_peak),
	    cp_monitor_level_to_sample(snapshot->output_rms));
}

static void
cp_pa_print_recommendation(const struct cp_audio_device_candidate *candidates,
	size_t count)
{
	struct cp_audio_config config;
	int device;

	cp_audio_default_config(&config);
	if (cp_audio_select_device_candidate(&config, candidates, count,
	    &device) != CP_AUDIO_OK || device == CP_AUDIO_DEFAULT_DEVICE)
		return;

	printf("recommended: ./carrierpress --live --device \"%s\" "
	    "--channels %zu\n", candidates[device].name, config.channels);
}

static int
cp_pa_select_sample_rate(const struct cp_audio_config *config,
	const PaStreamParameters *input_params,
	const PaStreamParameters *output_params,
	const PaDeviceInfo *input_info, const PaDeviceInfo *output_info,
	double *sample_rate)
{
	int input_rate_supported;
	int output_rate_supported;
	int requested_supported;
	int status;

	if (config == NULL || input_params == NULL || output_params == NULL ||
	    input_info == NULL || output_info == NULL || sample_rate == NULL)
		return CP_PORTAUDIO_ERR_CONFIG;

	requested_supported = cp_pa_rate_supported(input_params, output_params,
	    config->sample_rate);
	input_rate_supported = cp_pa_rate_supported(input_params, output_params,
	    input_info->defaultSampleRate);
	output_rate_supported = cp_pa_rate_supported(input_params, output_params,
	    output_info->defaultSampleRate);

	status = cp_audio_choose_sample_rate(config,
	    input_info->defaultSampleRate, output_info->defaultSampleRate,
	    requested_supported, input_rate_supported, output_rate_supported,
	    sample_rate);
	if (status != CP_AUDIO_OK)
		return CP_PORTAUDIO_ERR_STREAM;

	return CP_PORTAUDIO_OK;
}

static int
cp_pa_select_devices(const struct cp_audio_config *config,
	PaDeviceIndex *input_device, PaDeviceIndex *output_device)
{
	struct cp_audio_device_candidate *candidates;
	const PaDeviceInfo *input_info;
	const PaDeviceInfo *output_info;
	PaDeviceIndex count;
	int selected_device;
	int status;

	candidates = NULL;
	count = Pa_GetDeviceCount();
	if (count < 0)
		return CP_PORTAUDIO_ERR_DEVICE;

	status = cp_pa_build_candidates(&candidates, &count);
	if (status != CP_PORTAUDIO_OK)
		return status;

	status = cp_audio_select_device_candidate(config, candidates,
	    (size_t)count, &selected_device);
	if (status != CP_AUDIO_OK) {
		free(candidates);
		return CP_PORTAUDIO_ERR_DEVICE;
	}

	if (selected_device != CP_AUDIO_DEFAULT_DEVICE) {
		*input_device = (PaDeviceIndex)selected_device;
		*output_device = (PaDeviceIndex)selected_device;
	} else {
		*input_device = (config->input_device == CP_AUDIO_DEFAULT_DEVICE) ?
		    Pa_GetDefaultInputDevice() :
		    (PaDeviceIndex)config->input_device;
		*output_device =
		    (config->output_device == CP_AUDIO_DEFAULT_DEVICE) ?
		    Pa_GetDefaultOutputDevice() :
		    (PaDeviceIndex)config->output_device;
	}

	free(candidates);
	if (*input_device == paNoDevice || *output_device == paNoDevice)
		return CP_PORTAUDIO_ERR_DEVICE;
	if (*input_device < 0 || *input_device >= count ||
	    *output_device < 0 || *output_device >= count)
		return CP_PORTAUDIO_ERR_DEVICE;

	input_info  = Pa_GetDeviceInfo(*input_device);
	output_info = Pa_GetDeviceInfo(*output_device);
	if (input_info == NULL || output_info == NULL)
		return CP_PORTAUDIO_ERR_DEVICE;
	if (input_info->maxInputChannels < (int)config->channels ||
	    output_info->maxOutputChannels < (int)config->channels)
		return CP_PORTAUDIO_ERR_DEVICE;

	return CP_PORTAUDIO_OK;
}

#ifdef CP_WITH_TUI
static void
cp_pa_store_control_request(struct cp_portaudio_runtime *runtime,
	const struct cp_control_command *command)
{
	if (runtime == NULL || command == NULL)
		return;

	atomic_store(&runtime->pending_am_preset, (int)command->am_preset);
	atomic_store(&runtime->pending_ssb_preset,
	    (int)command->ssb_preset);
	atomic_store(&runtime->pending_control_type, (int)command->type);
}
#endif

static void
cp_pa_store_meters(struct cp_portaudio_runtime *runtime)
{
	size_t band;
	size_t band_count;
	enum cp_am_preset am_preset;
	enum cp_ssb_preset ssb_preset;

	atomic_store(&runtime->input_peak,
	    cp_monitor_sample_to_level(runtime->processor.input_meter.peak[0]));
	atomic_store(&runtime->input_rms,
	    cp_monitor_sample_to_level(runtime->processor.input_meter.rms[0]));
	atomic_store(&runtime->output_peak,
	    cp_monitor_sample_to_level(runtime->processor.output_meter.peak[0]));
	atomic_store(&runtime->output_rms,
	    cp_monitor_sample_to_level(runtime->processor.output_meter.rms[0]));
	atomic_store(&runtime->agc_gain,
	    cp_monitor_sample_to_level(runtime->processor.agc.gain));
	atomic_store(&runtime->agc_gain_db_centibel,
	    cp_monitor_db_to_centibel(runtime->processor.agc.gain_db));
	atomic_store(&runtime->agc_state,
	    (int)runtime->processor.agc.gate_state);

	atomic_store(&runtime->dehummer_enabled,
	    runtime->processor.dehummer.config.enabled ? 1u : 0u);
	atomic_store(&runtime->dehummer_base_hz,
	    cp_pa_hz_to_uint(runtime->processor.dehummer.config.base_frequency));
	atomic_store(&runtime->dehummer_harmonic_count,
	    (unsigned int)runtime->processor.dehummer.config.harmonic_count);
	atomic_store(&runtime->multiband_enabled,
	    runtime->processor.multiband.config.enabled ? 1u : 0u);
	atomic_store(&runtime->multiband_preset,
	    (int)runtime->processor.multiband.config.preset);

	band_count = runtime->processor.multiband.band_count;
	if (band_count > CP_MONITOR_MAX_BANDS)
		band_count = CP_MONITOR_MAX_BANDS;
	atomic_store(&runtime->band_count, (unsigned int)band_count);
	for (band = 0; band < band_count; band++) {
		atomic_store(&runtime->band_rms[band],
		    cp_monitor_sample_to_level(
		    runtime->processor.multiband.band_rms[band]));
		atomic_store(&runtime->band_gr_db_centibel[band],
		    cp_monitor_db_to_centibel(
		    runtime->processor.multiband.band_gain_reduction_db[band]));
	}

	atomic_store(&runtime->am_enabled,
	    runtime->processor.am.config.enabled ? 1u : 0u);
	atomic_store(&runtime->am_highpass_hz,
	    cp_pa_hz_to_uint(runtime->processor.am.config.highpass_hz));
	atomic_store(&runtime->am_lowpass_hz,
	    cp_pa_hz_to_uint(runtime->processor.am.config.lowpass_hz));
	atomic_store(&runtime->am_positive_peak,
	    cp_monitor_sample_to_level(
	    runtime->processor.am.config.positive_peak_limit));
	atomic_store(&runtime->am_negative_peak,
	    cp_monitor_sample_to_level(
	    runtime->processor.am.config.negative_peak_limit));
	atomic_store(&runtime->am_asymmetry_enabled,
	    runtime->processor.am.config.asymmetry_enabled ? 1u : 0u);
	atomic_store(&runtime->am_asymmetry_ratio,
	    cp_monitor_sample_to_level(
	    runtime->processor.am.config.asymmetry_ratio));
	if (cp_am_preset_from_string(runtime->processor.am.config.preset_name,
	    &am_preset) == CP_OK)
		atomic_store(&runtime->am_preset, (int)am_preset);

	atomic_store(&runtime->ssb_enabled,
	    runtime->processor.ssb.config.enabled ? 1u : 0u);
	atomic_store(&runtime->ssb_highpass_hz,
	    cp_pa_hz_to_uint(runtime->processor.ssb.config.highpass_hz));
	atomic_store(&runtime->ssb_lowpass_hz,
	    cp_pa_hz_to_uint(runtime->processor.ssb.config.lowpass_hz));
	atomic_store(&runtime->ssb_peak_limit,
	    cp_monitor_sample_to_level(runtime->processor.ssb.config.peak_limit));
	atomic_store(&runtime->ssb_phase_rotator_enabled,
	    runtime->processor.ssb.config.phase_rotator_enabled ? 1u : 0u);
	if (cp_ssb_preset_from_string(runtime->processor.ssb.config.preset_name,
	    &ssb_preset) == CP_OK)
		atomic_store(&runtime->ssb_preset, (int)ssb_preset);
}

static int
cp_pa_stream_callback(const void *input_buffer, void *output_buffer,
	unsigned long frame_count, const PaStreamCallbackTimeInfo *time_info,
	PaStreamCallbackFlags status_flags, void *user_data)
{
	struct cp_portaudio_runtime *runtime;
	const cp_sample_t *input;
	cp_sample_t *output;
	int status;

	(void)time_info;

	runtime = user_data;
	output  = output_buffer;
	if (runtime == NULL || output == NULL)
		return paAbort;
	if (frame_count > runtime->block_size) {
		atomic_store(&runtime->dsp_status, CP_ERR_BUFFER);
		return paAbort;
	}

	input = input_buffer;
	if (input == NULL)
		input = runtime->zero_input;

	if (status_flags != 0)
		atomic_fetch_or(&runtime->status_flags,
		    cp_pa_flags_from_status(status_flags));

	cp_pa_apply_pending_control(runtime);
	status = cp_block_process(&runtime->processor, input, output,
	    runtime->scratch, runtime->block_size * runtime->channels,
	    (size_t)frame_count);
	if (status != CP_OK) {
		atomic_store(&runtime->dsp_status, status);
		return paAbort;
	}

	cp_pa_store_meters(runtime);

	return paContinue;
}

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_portaudio.c */

#include <sys/types.h>

#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <portaudio.h>

#include "cp_block.h"
#include "cp_portaudio.h"

#define CP_PA_METER_SCALE	1000000.0f
#define CP_PA_SLEEP_MS		50

enum cp_pa_flags {
	CP_PA_INPUT_UNDERFLOW  = 1u << 0,
	CP_PA_INPUT_OVERFLOW   = 1u << 1,
	CP_PA_OUTPUT_UNDERFLOW = 1u << 2,
	CP_PA_OUTPUT_OVERFLOW  = 1u << 3,
	CP_PA_PRIMING_OUTPUT   = 1u << 4
};

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
	atomic_uint status_flags;
	atomic_int dsp_status;
};

static unsigned int	cp_pa_flags_from_status(PaStreamCallbackFlags);
static int		cp_pa_init_processor(struct cp_portaudio_runtime *,
			    const struct cp_audio_config *);
static unsigned int	cp_pa_meter_value(cp_sample_t);
static void		cp_pa_print_flags(unsigned int);
static void		cp_pa_print_meters(struct cp_portaudio_runtime *);
static int		cp_pa_select_devices(const struct cp_audio_config *,
			    PaDeviceIndex *, PaDeviceIndex *);
static int		cp_pa_stream_callback(const void *, void *,
			    unsigned long, const PaStreamCallbackTimeInfo *,
			    PaStreamCallbackFlags, void *);

int
cp_portaudio_list_devices(void)
{
	const PaDeviceInfo *info;
	PaDeviceIndex count;
	PaDeviceIndex device;
	PaError error;

	error = Pa_Initialize();
	if (error != paNoError)
		return CP_PORTAUDIO_ERR_INIT;

	count = Pa_GetDeviceCount();
	if (count < 0) {
		Pa_Terminate();
		return CP_PORTAUDIO_ERR_DEVICE;
	}

	for (device = 0; device < count; device++) {
		info = Pa_GetDeviceInfo(device);
		if (info == NULL)
			continue;

		printf("%d: %s input=%d output=%d rate=%0.0f\n", device,
		    info->name, info->maxInputChannels, info->maxOutputChannels,
		    info->defaultSampleRate);
	}

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
	struct cp_portaudio_runtime runtime;
	unsigned int flags;
	int status;

	status = cp_audio_validate_config(config);
	if (status != CP_AUDIO_OK)
		return CP_PORTAUDIO_ERR_CONFIG;
	if (stop_requested == NULL)
		return CP_PORTAUDIO_ERR_CONFIG;

	error = Pa_Initialize();
	if (error != paNoError)
		return CP_PORTAUDIO_ERR_INIT;

	status = cp_pa_select_devices(config, &input_device, &output_device);
	if (status != CP_PORTAUDIO_OK) {
		Pa_Terminate();
		return status;
	}

	status = cp_pa_init_processor(&runtime, config);
	if (status != CP_PORTAUDIO_OK) {
		Pa_Terminate();
		return status;
	}

	input_params.device                    = input_device;
	input_params.channelCount              = (int)config->channels;
	input_params.sampleFormat              = paFloat32;
	input_params.suggestedLatency          =
	    Pa_GetDeviceInfo(input_device)->defaultLowInputLatency;
	input_params.hostApiSpecificStreamInfo = NULL;

	output_params.device                    = output_device;
	output_params.channelCount              = (int)config->channels;
	output_params.sampleFormat              = paFloat32;
	output_params.suggestedLatency          =
	    Pa_GetDeviceInfo(output_device)->defaultLowOutputLatency;
	output_params.hostApiSpecificStreamInfo = NULL;

	stream = NULL;
	error = Pa_OpenStream(&stream, &input_params, &output_params,
	    config->sample_rate, (unsigned long)config->block_size, paNoFlag,
	    cp_pa_stream_callback, &runtime);
	if (error != paNoError) {
		free(runtime.scratch);
		free(runtime.zero_input);
		Pa_Terminate();
		return CP_PORTAUDIO_ERR_STREAM;
	}

	error = Pa_StartStream(stream);
	if (error != paNoError) {
		Pa_CloseStream(stream);
		free(runtime.scratch);
		free(runtime.zero_input);
		Pa_Terminate();
		return CP_PORTAUDIO_ERR_START;
	}

	printf("live audio started. press Ctrl-C to stop.\n");
	while (!*stop_requested && Pa_IsStreamActive(stream) == 1) {
		Pa_Sleep((long)config->meter_interval_ms);
		cp_pa_print_meters(&runtime);
		flags = atomic_exchange(&runtime.status_flags, 0u);
		cp_pa_print_flags(flags);

		if (atomic_load(&runtime.dsp_status) != CP_OK)
			break;
	}

	error = Pa_StopStream(stream);
	if (error != paNoError)
		status = CP_PORTAUDIO_ERR_STOP;
	else
		status = CP_PORTAUDIO_OK;

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

static unsigned int
cp_pa_flags_from_status(PaStreamCallbackFlags status_flags)
{
	unsigned int flags;

	flags = 0u;
	if ((status_flags & paInputUnderflow) != 0)
		flags |= CP_PA_INPUT_UNDERFLOW;
	if ((status_flags & paInputOverflow) != 0)
		flags |= CP_PA_INPUT_OVERFLOW;
	if ((status_flags & paOutputUnderflow) != 0)
		flags |= CP_PA_OUTPUT_UNDERFLOW;
	if ((status_flags & paOutputOverflow) != 0)
		flags |= CP_PA_OUTPUT_OVERFLOW;
	if ((status_flags & paPrimingOutput) != 0)
		flags |= CP_PA_PRIMING_OUTPUT;

	return flags;
}

static int
cp_pa_init_processor(struct cp_portaudio_runtime *runtime,
	const struct cp_audio_config *config)
{
	struct cp_block_config block_config;
	size_t samples;
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
	atomic_init(&runtime->status_flags, 0u);
	atomic_init(&runtime->dsp_status, CP_OK);

	cp_block_default_config(&block_config, config->channels);
	block_config.sample_rate = (cp_sample_t)config->sample_rate;
	block_config.dehummer_enabled = config->dehummer_enabled;
	block_config.hum_base_frequency = config->hum_base_frequency;
	block_config.hum_harmonic_count = config->hum_harmonic_count;
	block_config.hum_q_factor = config->hum_q_factor;
	status = cp_block_init(&runtime->processor, &block_config);
	if (status != CP_OK) {
		free(runtime->scratch);
		free(runtime->zero_input);
		return CP_PORTAUDIO_ERR_DSP;
	}

	return CP_PORTAUDIO_OK;
}

static unsigned int
cp_pa_meter_value(cp_sample_t value)
{
	if (value <= 0.0f)
		return 0u;
	if (value >= 4.0f)
		return 4000000u;

	return (unsigned int)(value * CP_PA_METER_SCALE);
}

static void
cp_pa_print_flags(unsigned int flags)
{
	if (flags == 0u)
		return;

	printf("stream_status:");
	if ((flags & CP_PA_INPUT_UNDERFLOW) != 0)
		printf(" input_underflow");
	if ((flags & CP_PA_INPUT_OVERFLOW) != 0)
		printf(" input_overflow");
	if ((flags & CP_PA_OUTPUT_UNDERFLOW) != 0)
		printf(" output_underflow");
	if ((flags & CP_PA_OUTPUT_OVERFLOW) != 0)
		printf(" output_overflow");
	if ((flags & CP_PA_PRIMING_OUTPUT) != 0)
		printf(" priming_output");
	printf("\n");
}

static void
cp_pa_print_meters(struct cp_portaudio_runtime *runtime)
{
	printf("input_peak=%0.6f input_rms=%0.6f output_peak=%0.6f "
	    "output_rms=%0.6f\n",
	    (double)atomic_load(&runtime->input_peak) / CP_PA_METER_SCALE,
	    (double)atomic_load(&runtime->input_rms) / CP_PA_METER_SCALE,
	    (double)atomic_load(&runtime->output_peak) / CP_PA_METER_SCALE,
	    (double)atomic_load(&runtime->output_rms) / CP_PA_METER_SCALE);
}

static int
cp_pa_select_devices(const struct cp_audio_config *config,
	PaDeviceIndex *input_device, PaDeviceIndex *output_device)
{
	const PaDeviceInfo *input_info;
	const PaDeviceInfo *output_info;
	PaDeviceIndex count;

	count = Pa_GetDeviceCount();
	if (count < 0)
		return CP_PORTAUDIO_ERR_DEVICE;

	*input_device = (config->input_device == CP_AUDIO_DEFAULT_DEVICE) ?
	    Pa_GetDefaultInputDevice() : (PaDeviceIndex)config->input_device;
	*output_device = (config->output_device == CP_AUDIO_DEFAULT_DEVICE) ?
	    Pa_GetDefaultOutputDevice() : (PaDeviceIndex)config->output_device;

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

	status = cp_block_process(&runtime->processor, input, output,
	    runtime->scratch, runtime->block_size * runtime->channels,
	    (size_t)frame_count);
	if (status != CP_OK) {
		atomic_store(&runtime->dsp_status, status);
		return paAbort;
	}

	atomic_store(&runtime->input_peak,
	    cp_pa_meter_value(runtime->processor.input_meter.peak[0]));
	atomic_store(&runtime->input_rms,
	    cp_pa_meter_value(runtime->processor.input_meter.rms[0]));
	atomic_store(&runtime->output_peak,
	    cp_pa_meter_value(runtime->processor.output_meter.peak[0]));
	atomic_store(&runtime->output_rms,
	    cp_pa_meter_value(runtime->processor.output_meter.rms[0]));

	return paContinue;
}

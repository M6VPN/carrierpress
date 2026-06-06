/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_sndio.c */

#include <sys/types.h>

#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sndio.h>

#include "cp_agc.h"
#include "cp_block.h"
#include "cp_monitor.h"
#include "cp_sndio.h"

#define CP_SNDIO_PCM_SCALE	32768.0f
#define CP_SNDIO_PCM_MAX	32767.0f
#define CP_SNDIO_PCM_MIN	(-32768.0f)
#define CP_SNDIO_APPBUF_BLOCKS	4u

struct cp_sndio_runtime {
	struct cp_block_processor processor;
	struct sio_hdl *handle;
	int16_t *input_pcm;
	int16_t *output_pcm;
	cp_sample_t *input;
	cp_sample_t *output;
	cp_sample_t *scratch;
	size_t channels;
	size_t frames;
	unsigned int rate;
	unsigned int meter_interval_ms;
};

static int	cp_sndio_allocate_runtime(struct cp_sndio_runtime *);
static void	cp_sndio_free_runtime(struct cp_sndio_runtime *);
static void	cp_sndio_pcm_to_float(const int16_t *, cp_sample_t *, size_t);
static void	cp_sndio_float_to_pcm(const cp_sample_t *, int16_t *, size_t);
static void	cp_sndio_print_meters(const struct cp_monitor_snapshot *);
static int	cp_sndio_read_full(struct sio_hdl *, void *, size_t);
static int	cp_sndio_setup_params(struct sio_hdl *,
		    const struct cp_audio_config *, struct sio_par *);
static int	cp_sndio_write_full(struct sio_hdl *, const void *, size_t);

int
cp_sndio_run(const struct cp_audio_config *config,
	volatile sig_atomic_t *stop_requested)
{
	struct cp_block_config block_config;
	struct cp_monitor_snapshot snapshot;
	struct cp_sndio_runtime runtime;
	struct sio_par par;
	const char *device_name;
	uint64_t meter_frames;
	uint64_t meter_target;
	size_t sample_count;
	size_t byte_count;
	int status;

	if (config == NULL || stop_requested == NULL)
		return CP_SNDIO_ERR_CONFIG;
	if (config->input_device != CP_AUDIO_DEFAULT_DEVICE ||
	    config->output_device != CP_AUDIO_DEFAULT_DEVICE)
		return CP_SNDIO_ERR_CONFIG;
	if (config->tui_enabled)
		return CP_SNDIO_ERR_CONFIG;

	(void)memset(&runtime, 0, sizeof(runtime));
	runtime.channels = config->channels;
	runtime.frames = config->block_size;
	runtime.meter_interval_ms = config->meter_interval_ms;
	device_name = config->device_name;

	runtime.handle = sio_open(device_name, SIO_PLAY | SIO_REC, 0);
	if (runtime.handle == NULL)
		return CP_SNDIO_ERR_OPEN;

	status = cp_sndio_setup_params(runtime.handle, config, &par);
	if (status != CP_SNDIO_OK) {
		cp_sndio_free_runtime(&runtime);
		return status;
	}
	runtime.rate = par.rate;

	status = cp_block_config_from_audio(&block_config, config,
	    config->channels, (cp_sample_t)par.rate);
	if (status != CP_OK) {
		cp_sndio_free_runtime(&runtime);
		return CP_SNDIO_ERR_CONFIG;
	}
	status = cp_block_init(&runtime.processor, &block_config);
	if (status != CP_OK) {
		cp_sndio_free_runtime(&runtime);
		return CP_SNDIO_ERR_DSP;
	}

	if (cp_sndio_allocate_runtime(&runtime) != CP_SNDIO_OK) {
		cp_sndio_free_runtime(&runtime);
		return CP_SNDIO_ERR_ALLOC;
	}
	if (!sio_start(runtime.handle)) {
		cp_sndio_free_runtime(&runtime);
		return CP_SNDIO_ERR_START;
	}

	sample_count = runtime.frames * runtime.channels;
	byte_count = sample_count * sizeof(*runtime.input_pcm);
	meter_frames = 0;
	meter_target = ((uint64_t)runtime.rate *
	    (uint64_t)runtime.meter_interval_ms) / 1000u;
	if (meter_target == 0)
		meter_target = runtime.frames;

	printf("sndio live audio started. device=%s rate=%u channels=%zu "
	    "block_size=%zu. press Ctrl-C to stop.\n",
	    device_name == NULL ? "default" : device_name, runtime.rate,
	    runtime.channels, runtime.frames);

	while (!*stop_requested) {
		status = cp_sndio_read_full(runtime.handle, runtime.input_pcm,
		    byte_count);
		if (status != CP_SNDIO_OK) {
			cp_sndio_free_runtime(&runtime);
			return status;
		}

		cp_sndio_pcm_to_float(runtime.input_pcm, runtime.input,
		    sample_count);
		status = cp_block_process(&runtime.processor, runtime.input,
		    runtime.output, runtime.scratch, sample_count,
		    runtime.frames);
		if (status != CP_OK) {
			cp_sndio_free_runtime(&runtime);
			return CP_SNDIO_ERR_DSP;
		}
		cp_sndio_float_to_pcm(runtime.output, runtime.output_pcm,
		    sample_count);

		status = cp_sndio_write_full(runtime.handle, runtime.output_pcm,
		    byte_count);
		if (status != CP_SNDIO_OK) {
			cp_sndio_free_runtime(&runtime);
			return status;
		}

		meter_frames += runtime.frames;
		if (meter_frames >= meter_target) {
			meter_frames = 0;
			if (cp_monitor_snapshot_from_processor(&runtime.processor,
			    &snapshot) == CP_OK)
				cp_sndio_print_meters(&snapshot);
		}
	}

	cp_sndio_free_runtime(&runtime);
	return CP_SNDIO_OK;
}

const char *
cp_sndio_status_string(int status)
{
	switch (status) {
	case CP_SNDIO_OK:
		return "ok";
	case CP_SNDIO_ERR_CONFIG:
		return "invalid sndio config";
	case CP_SNDIO_ERR_OPEN:
		return "could not open sndio device";
	case CP_SNDIO_ERR_PARAM:
		return "unsupported sndio device parameters";
	case CP_SNDIO_ERR_ALLOC:
		return "could not allocate sndio buffers";
	case CP_SNDIO_ERR_DSP:
		return "DSP processing failed";
	case CP_SNDIO_ERR_START:
		return "could not start sndio stream";
	case CP_SNDIO_ERR_READ:
		return "sndio input failed";
	case CP_SNDIO_ERR_WRITE:
		return "sndio output failed";
	default:
		return "unknown sndio error";
	}
}

static int
cp_sndio_allocate_runtime(struct cp_sndio_runtime *runtime)
{
	size_t sample_count;

	if (runtime == NULL || runtime->channels == 0 || runtime->frames == 0)
		return CP_SNDIO_ERR_CONFIG;
	if (runtime->frames > SIZE_MAX / runtime->channels)
		return CP_SNDIO_ERR_ALLOC;
	sample_count = runtime->frames * runtime->channels;

	runtime->input_pcm = calloc(sample_count, sizeof(*runtime->input_pcm));
	runtime->output_pcm = calloc(sample_count,
	    sizeof(*runtime->output_pcm));
	runtime->input = calloc(sample_count, sizeof(*runtime->input));
	runtime->output = calloc(sample_count, sizeof(*runtime->output));
	runtime->scratch = calloc(sample_count, sizeof(*runtime->scratch));
	if (runtime->input_pcm == NULL || runtime->output_pcm == NULL ||
	    runtime->input == NULL || runtime->output == NULL ||
	    runtime->scratch == NULL)
		return CP_SNDIO_ERR_ALLOC;

	return CP_SNDIO_OK;
}

static void
cp_sndio_free_runtime(struct cp_sndio_runtime *runtime)
{
	if (runtime == NULL)
		return;

	if (runtime->handle != NULL) {
		sio_close(runtime->handle);
		runtime->handle = NULL;
	}
	free(runtime->scratch);
	free(runtime->output);
	free(runtime->input);
	free(runtime->output_pcm);
	free(runtime->input_pcm);
	runtime->scratch = NULL;
	runtime->output = NULL;
	runtime->input = NULL;
	runtime->output_pcm = NULL;
	runtime->input_pcm = NULL;
}

static void
cp_sndio_pcm_to_float(const int16_t *input, cp_sample_t *output,
	size_t sample_count)
{
	size_t sample;

	for (sample = 0; sample < sample_count; sample++)
		output[sample] = (cp_sample_t)input[sample] /
		    (cp_sample_t)CP_SNDIO_PCM_SCALE;
}

static void
cp_sndio_float_to_pcm(const cp_sample_t *input, int16_t *output,
	size_t sample_count)
{
	cp_sample_t value;
	cp_sample_t scaled;
	size_t sample;

	for (sample = 0; sample < sample_count; sample++) {
		value = input[sample];
		if (!isfinite(value))
			value = 0.0f;
		if (value > 1.0f)
			value = 1.0f;
		if (value < -1.0f)
			value = -1.0f;

		scaled = value * CP_SNDIO_PCM_MAX;
		if (scaled > CP_SNDIO_PCM_MAX)
			output[sample] = INT16_MAX;
		else if (scaled < CP_SNDIO_PCM_MIN)
			output[sample] = INT16_MIN;
		else
			output[sample] = (int16_t)lrintf(scaled);
	}
}

static void
cp_sndio_print_meters(const struct cp_monitor_snapshot *snapshot)
{
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
}

static int
cp_sndio_read_full(struct sio_hdl *handle, void *buffer, size_t byte_count)
{
	unsigned char *cursor;
	size_t done;
	size_t got;

	if (handle == NULL || buffer == NULL)
		return CP_SNDIO_ERR_READ;

	cursor = buffer;
	done = 0;
	while (done < byte_count) {
		got = sio_read(handle, cursor + done, byte_count - done);
		if (got == 0)
			return CP_SNDIO_ERR_READ;
		done += got;
	}

	return CP_SNDIO_OK;
}

static int
cp_sndio_setup_params(struct sio_hdl *handle,
	const struct cp_audio_config *config, struct sio_par *par)
{
	unsigned int requested_rate;
	unsigned int requested_channels;

	if (handle == NULL || config == NULL || par == NULL)
		return CP_SNDIO_ERR_CONFIG;
	if (config->sample_rate < 1.0 ||
	    config->sample_rate > (double)UINT_MAX)
		return CP_SNDIO_ERR_CONFIG;
	if (config->channels > UINT_MAX ||
	    config->block_size > UINT_MAX)
		return CP_SNDIO_ERR_CONFIG;

	requested_rate = (unsigned int)config->sample_rate;
	requested_channels = (unsigned int)config->channels;

	sio_initpar(par);
	par->bits = 16;
	par->sig = 1;
	par->le = SIO_LE_NATIVE;
	par->rchan = requested_channels;
	par->pchan = requested_channels;
	par->rate = requested_rate;
	par->round = (unsigned int)config->block_size;
	par->appbufsz = par->round * CP_SNDIO_APPBUF_BLOCKS;

	if (!sio_setpar(handle, par))
		return CP_SNDIO_ERR_PARAM;
	if (!sio_getpar(handle, par))
		return CP_SNDIO_ERR_PARAM;
	if (par->bits != 16 || par->sig == 0 ||
	    par->le != SIO_LE_NATIVE)
		return CP_SNDIO_ERR_PARAM;
	if (par->rchan != requested_channels ||
	    par->pchan != requested_channels)
		return CP_SNDIO_ERR_PARAM;
	if (config->sample_rate_explicit && par->rate != requested_rate)
		return CP_SNDIO_ERR_PARAM;

	return CP_SNDIO_OK;
}

static int
cp_sndio_write_full(struct sio_hdl *handle, const void *buffer,
	size_t byte_count)
{
	const unsigned char *cursor;
	size_t done;
	size_t wrote;

	if (handle == NULL || buffer == NULL)
		return CP_SNDIO_ERR_WRITE;

	cursor = buffer;
	done = 0;
	while (done < byte_count) {
		wrote = sio_write(handle, cursor + done, byte_count - done);
		if (wrote == 0)
			return CP_SNDIO_ERR_WRITE;
		done += wrote;
	}

	return CP_SNDIO_OK;
}

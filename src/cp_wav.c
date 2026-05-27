/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_wav.c */

#include <sys/types.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sndfile.h>

#include "cp_block.h"
#include "cp_wav.h"

int
cp_wav_process_file(const char *input_path, const char *output_path,
	size_t block_frames)
{
	SF_INFO input_info;
	SF_INFO output_info;
	SNDFILE *input_file;
	SNDFILE *output_file;
	cp_sample_t *input;
	cp_sample_t *output;
	cp_sample_t *scratch;
	struct cp_block_config config;
	struct cp_block_processor processor;
	sf_count_t frames_read;
	sf_count_t frames_written;
	size_t block_samples;
	size_t channels;
	int status;
	int result;

	if (input_path == NULL || output_path == NULL)
		return CP_WAV_ERR_NULL;
	if (block_frames == 0)
		return CP_WAV_ERR_FORMAT;

	memset(&input_info, 0, sizeof(input_info));
	memset(&output_info, 0, sizeof(output_info));

	input_file = sf_open(input_path, SFM_READ, &input_info);
	if (input_file == NULL)
		return CP_WAV_ERR_OPEN_IN;

	if (input_info.channels != CP_CHANNELS_MONO &&
	    input_info.channels != CP_CHANNELS_STEREO) {
		sf_close(input_file);
		return CP_WAV_ERR_CHANNELS;
	}

	channels = (size_t)input_info.channels;
	if (block_frames > (SIZE_MAX / channels)) {
		sf_close(input_file);
		return CP_WAV_ERR_FORMAT;
	}

	block_samples = block_frames * channels;
	input         = calloc(block_samples, sizeof(*input));
	output        = calloc(block_samples, sizeof(*output));
	scratch       = calloc(block_samples, sizeof(*scratch));
	if (input == NULL || output == NULL || scratch == NULL) {
		free(input);
		free(output);
		free(scratch);
		sf_close(input_file);
		return CP_WAV_ERR_ALLOC;
	}

	cp_block_default_config(&config, channels);
	config.sample_rate = (cp_sample_t)input_info.samplerate;
	status = cp_block_init(&processor, &config);
	if (status != CP_OK) {
		free(input);
		free(output);
		free(scratch);
		sf_close(input_file);
		return CP_WAV_ERR_PROCESS;
	}

	output_info = input_info;
	output_file = sf_open(output_path, SFM_WRITE, &output_info);
	if (output_file == NULL) {
		free(input);
		free(output);
		free(scratch);
		sf_close(input_file);
		return CP_WAV_ERR_OPEN_OUT;
	}

	result = CP_WAV_OK;
	while ((frames_read = sf_readf_float(input_file, input,
	    (sf_count_t)block_frames)) > 0) {
		status = cp_block_process(&processor, input, output, scratch,
		    block_samples, (size_t)frames_read);
		if (status != CP_OK) {
			result = CP_WAV_ERR_PROCESS;
			break;
		}

		frames_written = sf_writef_float(output_file, output,
		    frames_read);
		if (frames_written != frames_read) {
			result = CP_WAV_ERR_WRITE;
			break;
		}
	}

	if (frames_read < 0 && result == CP_WAV_OK)
		result = CP_WAV_ERR_READ;

	sf_close(output_file);
	sf_close(input_file);
	free(input);
	free(output);
	free(scratch);

	return result;
}

const char *
cp_wav_status_string(int status)
{
	switch (status) {
	case CP_WAV_OK:
		return "ok";
	case CP_WAV_ERR_NULL:
		return "missing input or output path";
	case CP_WAV_ERR_OPEN_IN:
		return "could not open input WAV";
	case CP_WAV_ERR_OPEN_OUT:
		return "could not open output WAV";
	case CP_WAV_ERR_CHANNELS:
		return "WAV must be mono or stereo";
	case CP_WAV_ERR_FORMAT:
		return "invalid WAV processing format";
	case CP_WAV_ERR_ALLOC:
		return "could not allocate WAV processing buffers";
	case CP_WAV_ERR_READ:
		return "could not read WAV data";
	case CP_WAV_ERR_WRITE:
		return "could not write WAV data";
	case CP_WAV_ERR_PROCESS:
		return "DSP processing failed";
	default:
		return "unknown WAV error";
	}
}

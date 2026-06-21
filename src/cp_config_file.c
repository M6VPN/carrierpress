/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_config_file.c */

#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cp_config_file.h"

static int	cp_config_file_copy_text(char *, size_t, const char *);
static int	cp_config_file_forbidden_key(const char *);
static int	cp_config_file_parse_bool(const char *, int *);
static int	cp_config_file_parse_double(const char *, double *);
static int	cp_config_file_parse_int(const char *, long *);
static int	cp_config_file_parse_uint(const char *, unsigned long *);
static int	cp_config_file_set_error(struct cp_config_file_error *,
		    size_t, const char *, const char *);
static char	*cp_config_file_trim(char *);
static int	cp_config_file_value_seen(unsigned int, size_t,
		    const char *, struct cp_config_file_error *);

void
cp_config_file_init(struct cp_config_file *config)
{
	if (config == NULL)
		return;

	(void)memset(config, 0, sizeof(*config));
	config->audio_backend = CP_AUDIO_BACKEND_AUTO;
	config->input_device = CP_AUDIO_DEFAULT_DEVICE;
	config->output_device = CP_AUDIO_DEFAULT_DEVICE;
	config->sample_rate = CP_AUDIO_DEFAULT_SAMPLE_RATE;
	config->channels = CP_AUDIO_DEFAULT_CHANNELS;
	config->block_size = CP_AUDIO_DEFAULT_BLOCK_SIZE;
	config->meter_interval_ms = CP_AUDIO_DEFAULT_METER_MS;
}

int
cp_config_file_apply_to_audio_config(const struct cp_config_file *config,
	struct cp_audio_config *audio_config)
{
	size_t channels;
	double sample_rate;

	if (config == NULL || audio_config == NULL)
		return CP_ERR_NULL;
	if (cp_config_file_validate(config, NULL) != CP_OK)
		return CP_ERR_RANGE;

	channels = audio_config->channels;
	sample_rate = audio_config->sample_rate;
	if (config->seen_audio_backend)
		audio_config->backend = config->audio_backend;
	if (config->seen_device) {
		audio_config->device_name = config->device_name;
		audio_config->input_device = CP_AUDIO_DEFAULT_DEVICE;
		audio_config->output_device = CP_AUDIO_DEFAULT_DEVICE;
	}
	if (config->seen_input_device || config->seen_output_device)
		audio_config->device_name = NULL;
	if (config->seen_input_device)
		audio_config->input_device = config->input_device;
	if (config->seen_output_device)
		audio_config->output_device = config->output_device;
	if (config->seen_sample_rate) {
		sample_rate = config->sample_rate;
		audio_config->sample_rate_explicit = 1;
	}
	if (config->seen_channels)
		channels = config->channels;
	if (cp_audio_config_set_format(audio_config, channels,
	    sample_rate) != CP_AUDIO_OK)
		return CP_ERR_RANGE;
	if (config->seen_block_size)
		audio_config->block_size = config->block_size;
	if (config->seen_meter_interval_ms)
		audio_config->meter_interval_ms = config->meter_interval_ms;
	if (config->seen_tui) {
		audio_config->tui_enabled = config->tui_enabled;
		if (config->tui_enabled)
			audio_config->gui_enabled = 0;
	}
	if (config->seen_gui) {
		audio_config->gui_enabled = config->gui_enabled;
		if (config->gui_enabled)
			audio_config->tui_enabled = 0;
	}

	if (cp_audio_validate_config(audio_config) != CP_AUDIO_OK)
		return CP_ERR_RANGE;

	return CP_OK;
}

int
cp_config_file_has_profile(const struct cp_config_file *config)
{
	if (config == NULL)
		return 0;

	return config->seen_profile && config->profile_path[0] != '\0';
}

int
cp_config_file_parse_file(const char *path, struct cp_config_file *config,
	struct cp_config_file_error *error)
{
	char line[CP_CONFIG_FILE_LINE_SIZE];
	FILE *file;
	size_t line_number;
	size_t length;
	int status;
	int ch;

	if (path == NULL || config == NULL)
		return CP_ERR_NULL;

	file = fopen(path, "r");
	if (file == NULL)
		return cp_config_file_set_error(error, 0, "",
		    "could not open file");

	cp_config_file_init(config);
	line_number = 0;
	while (fgets(line, (int)sizeof(line), file) != NULL) {
		line_number++;
		length = strlen(line);
		if (length > 0 && line[length - 1] != '\n' && !feof(file)) {
			while ((ch = fgetc(file)) != '\n' && ch != EOF)
				;
			(void)fclose(file);
			return cp_config_file_set_error(error, line_number, "",
			    "line too long");
		}
		status = cp_config_file_parse_line(line, line_number, config,
		    error);
		if (status != CP_OK) {
			(void)fclose(file);
			return status;
		}
	}

	if (ferror(file)) {
		(void)fclose(file);
		return cp_config_file_set_error(error, line_number, "",
		    "file read failed");
	}
	(void)fclose(file);

	return cp_config_file_validate(config, error);
}

int
cp_config_file_parse_line(const char *line, size_t line_number,
	struct cp_config_file *config, struct cp_config_file_error *error)
{
	char buffer[CP_CONFIG_FILE_LINE_SIZE];
	char *equals;
	char *key;
	char *value;
	enum cp_audio_backend backend;
	unsigned long parsed_uint;
	long parsed_int;
	double parsed_double;
	int parsed_bool;

	if (line == NULL || config == NULL)
		return CP_ERR_NULL;
	if (strlen(line) >= sizeof(buffer))
		return cp_config_file_set_error(error, line_number, "",
		    "line too long");

	(void)snprintf(buffer, sizeof(buffer), "%s", line);
	buffer[strcspn(buffer, "\r\n")] = '\0';
	key = cp_config_file_trim(buffer);
	if (key[0] == '\0' || key[0] == '#')
		return CP_OK;

	equals = strchr(key, '=');
	if (equals == NULL)
		return cp_config_file_set_error(error, line_number, key,
		    "expected key = value");
	*equals = '\0';
	value = cp_config_file_trim(equals + 1);
	key = cp_config_file_trim(key);
	if (key[0] == '\0')
		return cp_config_file_set_error(error, line_number, key,
		    "empty key");
	if (value[0] == '\0')
		return cp_config_file_set_error(error, line_number, key,
		    "empty value");
	if (strchr(value, '=') != NULL)
		return cp_config_file_set_error(error, line_number, key,
		    "invalid value");
	if (cp_config_file_forbidden_key(key))
		return cp_config_file_set_error(error, line_number, key,
		    "control or profile key is not allowed in config files");

	if (strcmp(key, "profile") == 0) {
		if (cp_config_file_value_seen(config->seen_profile,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_config_file_copy_text(config->profile_path,
		    sizeof(config->profile_path), value) != CP_OK)
			return cp_config_file_set_error(error, line_number,
			    key, "value too long");
		config->seen_profile = 1;
		return CP_OK;
	}
	if (strcmp(key, "audio_backend") == 0) {
		if (cp_config_file_value_seen(config->seen_audio_backend,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_audio_backend_from_string(value, &backend) !=
		    CP_AUDIO_OK)
			return cp_config_file_set_error(error, line_number,
			    key, "invalid audio backend");
		config->audio_backend = backend;
		config->seen_audio_backend = 1;
		return CP_OK;
	}
	if (strcmp(key, "device") == 0) {
		if (cp_config_file_value_seen(config->seen_device,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_config_file_copy_text(config->device_name,
		    sizeof(config->device_name), value) != CP_OK)
			return cp_config_file_set_error(error, line_number,
			    key, "value too long");
		config->seen_device = 1;
		return CP_OK;
	}
	if (strcmp(key, "input_device") == 0) {
		if (cp_config_file_value_seen(config->seen_input_device,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_config_file_parse_int(value, &parsed_int) != CP_OK ||
		    parsed_int < CP_AUDIO_DEFAULT_DEVICE ||
		    parsed_int > INT_MAX)
			return cp_config_file_set_error(error, line_number,
			    key, "invalid input device");
		config->input_device = (int)parsed_int;
		config->seen_input_device = 1;
		return CP_OK;
	}
	if (strcmp(key, "output_device") == 0) {
		if (cp_config_file_value_seen(config->seen_output_device,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_config_file_parse_int(value, &parsed_int) != CP_OK ||
		    parsed_int < CP_AUDIO_DEFAULT_DEVICE ||
		    parsed_int > INT_MAX)
			return cp_config_file_set_error(error, line_number,
			    key, "invalid output device");
		config->output_device = (int)parsed_int;
		config->seen_output_device = 1;
		return CP_OK;
	}
	if (strcmp(key, "sample_rate") == 0) {
		if (cp_config_file_value_seen(config->seen_sample_rate,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_config_file_parse_double(value, &parsed_double) !=
		    CP_OK || parsed_double < CP_AUDIO_MIN_SAMPLE_RATE ||
		    parsed_double > CP_AUDIO_MAX_SAMPLE_RATE)
			return cp_config_file_set_error(error, line_number,
			    key, "invalid sample rate");
		config->sample_rate = parsed_double;
		config->seen_sample_rate = 1;
		return CP_OK;
	}
	if (strcmp(key, "channels") == 0) {
		if (cp_config_file_value_seen(config->seen_channels,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_config_file_parse_uint(value, &parsed_uint) != CP_OK ||
		    (parsed_uint != CP_CHANNELS_MONO &&
		    parsed_uint != CP_CHANNELS_STEREO))
			return cp_config_file_set_error(error, line_number,
			    key, "expected 1 or 2");
		config->channels = (size_t)parsed_uint;
		config->seen_channels = 1;
		return CP_OK;
	}
	if (strcmp(key, "block_size") == 0) {
		if (cp_config_file_value_seen(config->seen_block_size,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_config_file_parse_uint(value, &parsed_uint) != CP_OK ||
		    parsed_uint < CP_AUDIO_MIN_BLOCK_SIZE ||
		    parsed_uint > CP_AUDIO_MAX_BLOCK_SIZE)
			return cp_config_file_set_error(error, line_number,
			    key, "invalid block size");
		config->block_size = (size_t)parsed_uint;
		config->seen_block_size = 1;
		return CP_OK;
	}
	if (strcmp(key, "meter_interval_ms") == 0) {
		if (cp_config_file_value_seen(
		    config->seen_meter_interval_ms, line_number, key,
		    error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_config_file_parse_uint(value, &parsed_uint) != CP_OK ||
		    parsed_uint < CP_AUDIO_MIN_METER_MS ||
		    parsed_uint > CP_AUDIO_MAX_METER_MS ||
		    parsed_uint > UINT_MAX)
			return cp_config_file_set_error(error, line_number,
			    key, "invalid meter interval");
		config->meter_interval_ms = (unsigned int)parsed_uint;
		config->seen_meter_interval_ms = 1;
		return CP_OK;
	}
	if (strcmp(key, "tui") == 0) {
		if (cp_config_file_value_seen(config->seen_tui, line_number,
		    key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_config_file_parse_bool(value, &parsed_bool) != CP_OK)
			return cp_config_file_set_error(error, line_number,
			    key, "expected off or on");
		config->tui_enabled = parsed_bool;
		config->seen_tui = 1;
		return CP_OK;
	}
	if (strcmp(key, "gui") == 0) {
		if (cp_config_file_value_seen(config->seen_gui, line_number,
		    key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_config_file_parse_bool(value, &parsed_bool) != CP_OK)
			return cp_config_file_set_error(error, line_number,
			    key, "expected off or on");
		config->gui_enabled = parsed_bool;
		config->seen_gui = 1;
		return CP_OK;
	}

	return cp_config_file_set_error(error, line_number, key,
	    "unknown key");
}

const char *
cp_config_file_profile_path(const struct cp_config_file *config)
{
	if (!cp_config_file_has_profile(config))
		return NULL;

	return config->profile_path;
}

int
cp_config_file_validate(const struct cp_config_file *config,
	struct cp_config_file_error *error)
{
	if (config == NULL)
		return CP_ERR_NULL;
	if (config->seen_profile && config->profile_path[0] == '\0')
		return cp_config_file_set_error(error, 0, "profile",
		    "profile path is required");
	if (config->seen_device && config->device_name[0] == '\0')
		return cp_config_file_set_error(error, 0, "device",
		    "device name is required");
	if (config->seen_device &&
	    (config->seen_input_device || config->seen_output_device))
		return cp_config_file_set_error(error, 0, "device",
		    "device name cannot be combined with device indices");
	if (config->input_device < CP_AUDIO_DEFAULT_DEVICE)
		return cp_config_file_set_error(error, 0, "input_device",
		    "invalid input device");
	if (config->output_device < CP_AUDIO_DEFAULT_DEVICE)
		return cp_config_file_set_error(error, 0, "output_device",
		    "invalid output device");
	if (config->sample_rate < CP_AUDIO_MIN_SAMPLE_RATE ||
	    config->sample_rate > CP_AUDIO_MAX_SAMPLE_RATE)
		return cp_config_file_set_error(error, 0, "sample_rate",
		    "invalid sample rate");
	if (config->channels != CP_CHANNELS_MONO &&
	    config->channels != CP_CHANNELS_STEREO)
		return cp_config_file_set_error(error, 0, "channels",
		    "invalid channel count");
	if (config->block_size < CP_AUDIO_MIN_BLOCK_SIZE ||
	    config->block_size > CP_AUDIO_MAX_BLOCK_SIZE)
		return cp_config_file_set_error(error, 0, "block_size",
		    "invalid block size");
	if (config->meter_interval_ms < CP_AUDIO_MIN_METER_MS ||
	    config->meter_interval_ms > CP_AUDIO_MAX_METER_MS)
		return cp_config_file_set_error(error, 0,
		    "meter_interval_ms", "invalid meter interval");
	if (config->seen_tui && config->seen_gui &&
	    config->tui_enabled && config->gui_enabled)
		return cp_config_file_set_error(error, 0, "gui",
		    "tui and gui cannot both be enabled");

	return CP_OK;
}

static int
cp_config_file_copy_text(char *dst, size_t dst_size, const char *src)
{
	int written;

	if (dst == NULL || src == NULL || dst_size == 0)
		return CP_ERR_NULL;
	written = snprintf(dst, dst_size, "%s", src);
	if (written < 0 || (size_t)written >= dst_size)
		return CP_ERR_BUFFER;

	return CP_OK;
}

static int
cp_config_file_forbidden_key(const char *key)
{
	static const char *forbidden[] = {
		"ptt",
		"transmit",
		"tx",
		"cat_ptt",
		"frequency",
		"rig_frequency",
		"rig_mode",
		"mode_control",
		"flrig",
		"hamlib",
		"cat_backend",
		"cat_host",
		"cat_port",
		"rig",
		"rig_path",
		"rig_model",
		"radio",
		"station_control",
		"am_preset",
		"ssb_preset",
		"dehummer",
		"hum_frequency",
		"hum_harmonics",
		"multiband",
		"multiband_bands",
		"multiband2",
		"multiband2_bands",
		"bass_eq",
		"natural_dynamics",
		"low_level_boost",
		"restoration_analysis",
		"declipper"
	};
	size_t i;

	if (key == NULL)
		return 0;
	for (i = 0; i < sizeof(forbidden) / sizeof(forbidden[0]); i++) {
		if (strcmp(key, forbidden[i]) == 0)
			return 1;
	}

	return 0;
}

static int
cp_config_file_parse_bool(const char *text, int *value)
{
	if (text == NULL || value == NULL)
		return CP_ERR_NULL;
	if (strcmp(text, "off") == 0) {
		*value = 0;
		return CP_OK;
	}
	if (strcmp(text, "on") == 0) {
		*value = 1;
		return CP_OK;
	}

	return CP_ERR_RANGE;
}

static int
cp_config_file_parse_double(const char *text, double *value)
{
	char *end;
	double parsed;

	if (text == NULL || value == NULL)
		return CP_ERR_NULL;

	errno = 0;
	parsed = strtod(text, &end);
	if (errno != 0 || end == text || *end != '\0' || !isfinite(parsed))
		return CP_ERR_RANGE;
	*value = parsed;
	return CP_OK;
}

static int
cp_config_file_parse_int(const char *text, long *value)
{
	char *end;
	long parsed;

	if (text == NULL || value == NULL)
		return CP_ERR_NULL;

	errno = 0;
	parsed = strtol(text, &end, 10);
	if (errno != 0 || end == text || *end != '\0')
		return CP_ERR_RANGE;
	*value = parsed;
	return CP_OK;
}

static int
cp_config_file_parse_uint(const char *text, unsigned long *value)
{
	char *end;
	unsigned long parsed;

	if (text == NULL || value == NULL)
		return CP_ERR_NULL;
	if (text[0] == '-')
		return CP_ERR_RANGE;

	errno = 0;
	parsed = strtoul(text, &end, 10);
	if (errno != 0 || end == text || *end != '\0')
		return CP_ERR_RANGE;
	*value = parsed;
	return CP_OK;
}

static int
cp_config_file_set_error(struct cp_config_file_error *error,
	size_t line_number, const char *key, const char *message)
{
	if (error != NULL) {
		(void)memset(error, 0, sizeof(*error));
		error->line_number = line_number;
		if (key != NULL)
			(void)snprintf(error->key, sizeof(error->key), "%s",
			    key);
		if (message != NULL)
			(void)snprintf(error->message,
			    sizeof(error->message), "%s", message);
	}

	return CP_ERR_RANGE;
}

static char *
cp_config_file_trim(char *text)
{
	char *end;

	while (*text != '\0' && isspace((unsigned char)*text))
		text++;
	if (*text == '\0')
		return text;
	end = text + strlen(text) - 1;
	while (end > text && isspace((unsigned char)*end)) {
		*end = '\0';
		end--;
	}

	return text;
}

static int
cp_config_file_value_seen(unsigned int seen, size_t line_number,
	const char *key, struct cp_config_file_error *error)
{
	if (seen)
		return cp_config_file_set_error(error, line_number, key,
		    "duplicate key");

	return CP_OK;
}

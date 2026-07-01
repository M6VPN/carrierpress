/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_bulletin.c */

#include <sys/types.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cp_bulletin.h"

static int	cp_bulletin_apply_am_broadcast(struct cp_block_config *,
		    struct cp_audio_config *);
static int	cp_bulletin_apply_data_clean(struct cp_block_config *,
		    struct cp_audio_config *);
static int	cp_bulletin_apply_hf_narrow(struct cp_block_config *,
		    struct cp_audio_config *);
static int	cp_bulletin_apply_hf_voice(struct cp_block_config *,
		    struct cp_audio_config *);
static int	cp_bulletin_apply_vhf_fm(struct cp_block_config *,
		    struct cp_audio_config *);
static int	cp_bulletin_copy(char *, size_t, const char *);
static int	cp_bulletin_parse_item_type(const char *,
		    enum cp_bulletin_item_type *);
static int	cp_bulletin_parse_uint(const char *, unsigned int *);
static int	cp_bulletin_schedule_add_item(struct cp_bulletin_schedule *);
static int	cp_bulletin_set_error(char *, size_t, size_t, const char *);
static char	*cp_bulletin_trim(char *);
static char	*cp_bulletin_unquote(char *);

static const struct cp_bulletin_profile_summary profile_summaries[] = {
	{
		CP_BULLETIN_PROFILE_HF_SSB_VOICE,
		"hf-ssb-voice",
		"SSB speech bulletin voice profile",
		48000.0,
		CP_CHANNELS_MONO,
		150.0,
		2700.0,
		1,
		1,
		1,
		0,
		0,
		-1.0
	},
	{
		CP_BULLETIN_PROFILE_HF_SSB_NARROW,
		"hf-ssb-narrow",
		"Narrow SSB speech profile for weak paths",
		48000.0,
		CP_CHANNELS_MONO,
		200.0,
		2400.0,
		1,
		1,
		1,
		0,
		0,
		-1.0
	},
	{
		CP_BULLETIN_PROFILE_VHF_FM_VOICE,
		"vhf-fm-voice",
		"VHF FM voice playout profile",
		48000.0,
		CP_CHANNELS_MONO,
		120.0,
		3000.0,
		1,
		1,
		0,
		0,
		0,
		-1.0
	},
	{
		CP_BULLETIN_PROFILE_AM_BROADCAST_STYLE,
		"am-broadcast-style",
		"Moderate AM-style local audio profile",
		48000.0,
		CP_CHANNELS_MONO,
		60.0,
		5000.0,
		1,
		1,
		1,
		0,
		0,
		-1.0
	},
	{
		CP_BULLETIN_PROFILE_DATA_CLEAN_PASS_THROUGH,
		"data-clean-pass-through",
		"Clean pass-through profile for external digital modes",
		48000.0,
		CP_CHANNELS_STEREO,
		0.0,
		0.0,
		0,
		0,
		0,
		0,
		1,
		0.0
	}
};

int
cp_bulletin_apply_profile(enum cp_bulletin_profile_id id,
	struct cp_block_config *block_config, struct cp_audio_config *audio_config)
{
	if (block_config == NULL || audio_config == NULL)
		return CP_BULLETIN_ERR_NULL;

	switch (id) {
	case CP_BULLETIN_PROFILE_HF_SSB_VOICE:
		return cp_bulletin_apply_hf_voice(block_config, audio_config);
	case CP_BULLETIN_PROFILE_HF_SSB_NARROW:
		return cp_bulletin_apply_hf_narrow(block_config, audio_config);
	case CP_BULLETIN_PROFILE_VHF_FM_VOICE:
		return cp_bulletin_apply_vhf_fm(block_config, audio_config);
	case CP_BULLETIN_PROFILE_AM_BROADCAST_STYLE:
		return cp_bulletin_apply_am_broadcast(block_config,
		    audio_config);
	case CP_BULLETIN_PROFILE_DATA_CLEAN_PASS_THROUGH:
		return cp_bulletin_apply_data_clean(block_config, audio_config);
	default:
		return CP_BULLETIN_ERR_RANGE;
	}
}

const struct cp_bulletin_profile_summary *
cp_bulletin_profile_summary(enum cp_bulletin_profile_id id)
{
	size_t i;

	for (i = 0; i < sizeof(profile_summaries) /
	    sizeof(profile_summaries[0]); i++) {
		if (profile_summaries[i].id == id)
			return &profile_summaries[i];
	}

	return NULL;
}

int
cp_bulletin_profile_from_string(const char *text,
	enum cp_bulletin_profile_id *id)
{
	size_t i;

	if (text == NULL || id == NULL)
		return CP_BULLETIN_ERR_NULL;
	for (i = 0; i < sizeof(profile_summaries) /
	    sizeof(profile_summaries[0]); i++) {
		if (strcmp(text, profile_summaries[i].name) == 0) {
			*id = profile_summaries[i].id;
			return CP_BULLETIN_OK;
		}
	}

	return CP_BULLETIN_ERR_RANGE;
}

const char *
cp_bulletin_profile_string(enum cp_bulletin_profile_id id)
{
	const struct cp_bulletin_profile_summary *summary;

	summary = cp_bulletin_profile_summary(id);
	if (summary == NULL)
		return NULL;

	return summary->name;
}

void
cp_bulletin_schedule_init(struct cp_bulletin_schedule *schedule)
{
	if (schedule == NULL)
		return;

	(void)memset(schedule, 0, sizeof(*schedule));
	schedule->profile_id = CP_BULLETIN_PROFILE_HF_SSB_VOICE;
	schedule->repeat = CP_BULLETIN_DEFAULT_REPEAT;
	schedule->pre_roll_ms = CP_BULLETIN_DEFAULT_PRE_ROLL_MS;
	schedule->post_roll_ms = CP_BULLETIN_DEFAULT_POST_ROLL_MS;
}

int
cp_bulletin_schedule_parse_file(const char *path,
	struct cp_bulletin_schedule *schedule, char *error, size_t error_size)
{
	char line[CP_BULLETIN_LINE_SIZE];
	FILE *file;
	size_t line_number;
	size_t length;
	int ch;
	int status;

	if (path == NULL || schedule == NULL)
		return CP_BULLETIN_ERR_NULL;

	file = fopen(path, "r");
	if (file == NULL)
		return cp_bulletin_set_error(error, error_size, 0,
		    "could not open schedule");

	cp_bulletin_schedule_init(schedule);
	line_number = 0;
	while (fgets(line, (int)sizeof(line), file) != NULL) {
		line_number++;
		length = strlen(line);
		if (length > 0 && line[length - 1] != '\n' && !feof(file)) {
			while ((ch = fgetc(file)) != '\n' && ch != EOF)
				;
			(void)fclose(file);
			return cp_bulletin_set_error(error, error_size,
			    line_number, "line too long");
		}
		status = cp_bulletin_schedule_parse_line(line, line_number,
		    schedule, error, error_size);
		if (status != CP_BULLETIN_OK) {
			(void)fclose(file);
			return status;
		}
	}
	if (ferror(file)) {
		(void)fclose(file);
		return cp_bulletin_set_error(error, error_size, line_number,
		    "schedule read failed");
	}
	(void)fclose(file);

	return CP_BULLETIN_OK;
}

int
cp_bulletin_schedule_parse_line(const char *line, size_t line_number,
	struct cp_bulletin_schedule *schedule, char *error, size_t error_size)
{
	struct cp_bulletin_item *item;
	enum cp_bulletin_profile_id profile_id;
	enum cp_bulletin_item_type item_type;
	char buffer[CP_BULLETIN_LINE_SIZE];
	char *equals;
	char *key;
	char *value;
	unsigned int parsed;
	int status;

	if (line == NULL || schedule == NULL)
		return CP_BULLETIN_ERR_NULL;
	if (strlen(line) >= sizeof(buffer))
		return cp_bulletin_set_error(error, error_size, line_number,
		    "line too long");

	(void)snprintf(buffer, sizeof(buffer), "%s", line);
	buffer[strcspn(buffer, "\r\n")] = '\0';
	key = cp_bulletin_trim(buffer);
	if (key[0] == '\0' || key[0] == '#')
		return CP_BULLETIN_OK;
	if (strcmp(key, "[[items]]") == 0)
		return cp_bulletin_schedule_add_item(schedule);

	equals = strchr(key, '=');
	if (equals == NULL)
		return cp_bulletin_set_error(error, error_size, line_number,
		    "expected key = value");
	*equals = '\0';
	value = cp_bulletin_unquote(cp_bulletin_trim(equals + 1));
	key = cp_bulletin_trim(key);
	if (key[0] == '\0' || value[0] == '\0')
		return cp_bulletin_set_error(error, error_size, line_number,
		    "empty key or value");

	item = NULL;
	if (schedule->item_count > 0)
		item = &schedule->items[schedule->item_count - 1];

	if (item != NULL && strcmp(key, "type") == 0) {
		status = cp_bulletin_parse_item_type(value, &item_type);
		if (status != CP_BULLETIN_OK)
			return cp_bulletin_set_error(error, error_size,
			    line_number, "invalid item type");
		item->type = item_type;
		return CP_BULLETIN_OK;
	}
	if (item != NULL && strcmp(key, "text") == 0)
		return cp_bulletin_copy(item->text, sizeof(item->text), value);
	if (item != NULL && strcmp(key, "path") == 0)
		return cp_bulletin_copy(item->path, sizeof(item->path), value);
	if (item != NULL && strcmp(key, "seconds") == 0) {
		if (cp_bulletin_parse_uint(value, &parsed) !=
		    CP_BULLETIN_OK)
			return cp_bulletin_set_error(error, error_size,
			    line_number, "invalid pause seconds");
		item->seconds = parsed;
		return CP_BULLETIN_OK;
	}

	if (strcmp(key, "callsign") == 0)
		return cp_bulletin_copy(schedule->callsign,
		    sizeof(schedule->callsign), value);
	if (strcmp(key, "profile") == 0) {
		status = cp_bulletin_profile_from_string(value, &profile_id);
		if (status != CP_BULLETIN_OK)
			return cp_bulletin_set_error(error, error_size,
			    line_number, "invalid profile");
		schedule->profile_id = profile_id;
		return CP_BULLETIN_OK;
	}
	if (strcmp(key, "repeat") == 0) {
		if (cp_bulletin_parse_uint(value, &parsed) !=
		    CP_BULLETIN_OK || parsed == 0)
			return cp_bulletin_set_error(error, error_size,
			    line_number, "invalid repeat");
		schedule->repeat = parsed;
		return CP_BULLETIN_OK;
	}
	if (strcmp(key, "pre_roll_ms") == 0) {
		if (cp_bulletin_parse_uint(value, &parsed) !=
		    CP_BULLETIN_OK)
			return cp_bulletin_set_error(error, error_size,
			    line_number, "invalid pre-roll");
		schedule->pre_roll_ms = parsed;
		return CP_BULLETIN_OK;
	}
	if (strcmp(key, "post_roll_ms") == 0) {
		if (cp_bulletin_parse_uint(value, &parsed) !=
		    CP_BULLETIN_OK)
			return cp_bulletin_set_error(error, error_size,
			    line_number, "invalid post-roll");
		schedule->post_roll_ms = parsed;
		return CP_BULLETIN_OK;
	}

	return cp_bulletin_set_error(error, error_size, line_number,
	    "unknown schedule key");
}

int
cp_bulletin_schedule_print_plan(const struct cp_bulletin_schedule *schedule,
	FILE *out)
{
	const char *profile;
	size_t i;

	if (schedule == NULL || out == NULL)
		return CP_BULLETIN_ERR_NULL;

	profile = cp_bulletin_profile_string(schedule->profile_id);
	if (profile == NULL)
		return CP_BULLETIN_ERR_RANGE;

	fprintf(out, "bulletin_plan=1\n");
	fprintf(out, "callsign=%s\n", schedule->callsign);
	fprintf(out, "profile=%s\n", profile);
	fprintf(out, "repeat=%u\n", schedule->repeat);
	fprintf(out, "pre_roll_ms=%u\n", schedule->pre_roll_ms);
	fprintf(out, "post_roll_ms=%u\n", schedule->post_roll_ms);
	fprintf(out, "items=%zu\n", schedule->item_count);
	for (i = 0; i < schedule->item_count; i++) {
		fprintf(out, "item%zu=%s", i + 1,
		    schedule->items[i].type == CP_BULLETIN_ITEM_ID ? "id" :
		    schedule->items[i].type == CP_BULLETIN_ITEM_FILE ? "file" :
		    schedule->items[i].type == CP_BULLETIN_ITEM_TTS ? "tts" :
		    "pause");
		if (schedule->items[i].path[0] != '\0')
			fprintf(out, " path=%s", schedule->items[i].path);
		if (schedule->items[i].text[0] != '\0')
			fprintf(out, " text=%s", schedule->items[i].text);
		if (schedule->items[i].seconds > 0)
			fprintf(out, " seconds=%u", schedule->items[i].seconds);
		fprintf(out, "\n");
	}

	return CP_BULLETIN_OK;
}

int
cp_bulletin_tx_gate_validate(const struct cp_bulletin_tx_gate *gate,
	char *reason, size_t reason_size)
{
	if (gate == NULL)
		return CP_BULLETIN_ERR_NULL;
	if (gate->ptt_mode != CP_BULLETIN_PTT_NONE &&
	    gate->ptt_mode != CP_BULLETIN_PTT_VOX && !gate->arm_tx) {
		(void)cp_bulletin_copy(reason, reason_size,
		    "PTT requires --arm-tx");
		return CP_BULLETIN_ERR_TX_ARM;
	}
	if (gate->arm_tx && gate->ptt_mode != CP_BULLETIN_PTT_NONE &&
	    gate->ptt_mode != CP_BULLETIN_PTT_VOX &&
	    (gate->station_id == NULL || gate->station_id[0] == '\0')) {
		(void)cp_bulletin_copy(reason, reason_size,
		    "armed PTT should include --id");
		return CP_BULLETIN_ERR_TX_ARM;
	}

	if (reason != NULL && reason_size > 0)
		reason[0] = '\0';
	return CP_BULLETIN_OK;
}

const char *
cp_bulletin_ptt_mode_string(enum cp_bulletin_ptt_mode mode)
{
	switch (mode) {
	case CP_BULLETIN_PTT_NONE:
		return "none";
	case CP_BULLETIN_PTT_CAT:
		return "cat";
	case CP_BULLETIN_PTT_SERIAL:
		return "serial";
	case CP_BULLETIN_PTT_VOX:
		return "vox";
	default:
		return "unknown";
	}
}

int
cp_bulletin_ptt_mode_from_string(const char *text,
	enum cp_bulletin_ptt_mode *mode)
{
	if (text == NULL || mode == NULL)
		return CP_BULLETIN_ERR_NULL;
	if (strcmp(text, "none") == 0) {
		*mode = CP_BULLETIN_PTT_NONE;
		return CP_BULLETIN_OK;
	}
	if (strcmp(text, "cat") == 0) {
		*mode = CP_BULLETIN_PTT_CAT;
		return CP_BULLETIN_OK;
	}
	if (strcmp(text, "serial") == 0 ||
	    strcmp(text, "rts") == 0 || strcmp(text, "dtr") == 0) {
		*mode = CP_BULLETIN_PTT_SERIAL;
		return CP_BULLETIN_OK;
	}
	if (strcmp(text, "vox") == 0) {
		*mode = CP_BULLETIN_PTT_VOX;
		return CP_BULLETIN_OK;
	}

	return CP_BULLETIN_ERR_RANGE;
}

const char *
cp_bulletin_status_string(int status)
{
	switch (status) {
	case CP_BULLETIN_OK:
		return "ok";
	case CP_BULLETIN_ERR_NULL:
		return "null argument";
	case CP_BULLETIN_ERR_RANGE:
		return "value out of range";
	case CP_BULLETIN_ERR_BUFFER:
		return "buffer too small";
	case CP_BULLETIN_ERR_PARSE:
		return "parse failed";
	case CP_BULLETIN_ERR_OPEN:
		return "open failed";
	case CP_BULLETIN_ERR_TX_ARM:
		return "TX not armed";
	default:
		return "unknown bulletin error";
	}
}

static int
cp_bulletin_apply_am_broadcast(struct cp_block_config *block_config,
	struct cp_audio_config *audio_config)
{
	(void)cp_audio_config_set_format(audio_config, CP_CHANNELS_MONO,
	    48000.0);
	block_config->channels = CP_CHANNELS_MONO;
	block_config->sample_rate = 48000.0f;
	block_config->multiband_enabled = 1;
	block_config->multiband_preset = CP_MULTIBAND_PRESET_MUSIC;
	block_config->bass_eq_config.enabled = 1;
	(void)cp_bass_eq_apply_preset(&block_config->bass_eq_config, "warm");
	(void)cp_am_apply_preset(&block_config->am_config, "am-wide");
	block_config->am_config.enabled = 1;
	block_config->ssb_config.enabled = 0;
	block_config->limiter_ceiling = 0.89125094f;
	audio_config->multiband_enabled = block_config->multiband_enabled;
	audio_config->multiband_preset = block_config->multiband_preset;
	audio_config->bass_eq_config = block_config->bass_eq_config;
	audio_config->am_config = block_config->am_config;
	audio_config->ssb_config = block_config->ssb_config;
	return CP_BULLETIN_OK;
}

static int
cp_bulletin_apply_data_clean(struct cp_block_config *block_config,
	struct cp_audio_config *audio_config)
{
	(void)cp_audio_config_set_format(audio_config, CP_CHANNELS_STEREO,
	    48000.0);
	block_config->channels = CP_CHANNELS_STEREO;
	block_config->sample_rate = 48000.0f;
	block_config->dehummer_enabled = 0;
	block_config->multiband_enabled = 0;
	block_config->multiband2_enabled = 0;
	block_config->bass_eq_config.enabled = 0;
	block_config->natural_dynamics_config.enabled = 0;
	block_config->low_level_boost_config.enabled = 0;
	block_config->restoration_config.enabled = 0;
	block_config->declipper_config.enabled = 0;
	block_config->am_config.enabled = 0;
	block_config->ssb_config.enabled = 0;
	block_config->limiter_ceiling = 1.0f;
	audio_config->dehummer_enabled = 0;
	audio_config->multiband_enabled = 0;
	audio_config->multiband2_enabled = 0;
	audio_config->bass_eq_config = block_config->bass_eq_config;
	audio_config->natural_dynamics_config =
	    block_config->natural_dynamics_config;
	audio_config->low_level_boost_config =
	    block_config->low_level_boost_config;
	audio_config->restoration_config = block_config->restoration_config;
	audio_config->declipper_config = block_config->declipper_config;
	audio_config->am_config = block_config->am_config;
	audio_config->ssb_config = block_config->ssb_config;
	return CP_BULLETIN_OK;
}

static int
cp_bulletin_apply_hf_narrow(struct cp_block_config *block_config,
	struct cp_audio_config *audio_config)
{
	(void)cp_audio_config_set_format(audio_config, CP_CHANNELS_MONO,
	    48000.0);
	block_config->channels = CP_CHANNELS_MONO;
	block_config->sample_rate = 48000.0f;
	(void)cp_ssb_apply_preset(&block_config->ssb_config, "hf-ssb-narrow");
	block_config->ssb_config.enabled = 1;
	block_config->am_config.enabled = 0;
	block_config->multiband_enabled = 1;
	block_config->multiband_preset = CP_MULTIBAND_PRESET_SPEECH;
	block_config->multiband2_enabled = 1;
	block_config->multiband2_preset = CP_MULTIBAND_PRESET_SPEECH;
	block_config->bass_eq_config.enabled = 1;
	(void)cp_bass_eq_apply_preset(&block_config->bass_eq_config,
	    "speech");
	block_config->natural_dynamics_config.enabled = 1;
	block_config->limiter_ceiling = 0.89125094f;
	audio_config->ssb_config = block_config->ssb_config;
	audio_config->am_config = block_config->am_config;
	audio_config->multiband_enabled = block_config->multiband_enabled;
	audio_config->multiband_preset = block_config->multiband_preset;
	audio_config->multiband2_enabled = block_config->multiband2_enabled;
	audio_config->multiband2_preset = block_config->multiband2_preset;
	audio_config->bass_eq_config = block_config->bass_eq_config;
	audio_config->natural_dynamics_config =
	    block_config->natural_dynamics_config;
	return CP_BULLETIN_OK;
}

static int
cp_bulletin_apply_hf_voice(struct cp_block_config *block_config,
	struct cp_audio_config *audio_config)
{
	(void)cp_audio_config_set_format(audio_config, CP_CHANNELS_MONO,
	    48000.0);
	block_config->channels = CP_CHANNELS_MONO;
	block_config->sample_rate = 48000.0f;
	(void)cp_ssb_apply_preset(&block_config->ssb_config, "hf-ssb-voice");
	block_config->ssb_config.enabled = 1;
	block_config->am_config.enabled = 0;
	block_config->multiband_enabled = 1;
	block_config->multiband_preset = CP_MULTIBAND_PRESET_SPEECH;
	block_config->bass_eq_config.enabled = 1;
	(void)cp_bass_eq_apply_preset(&block_config->bass_eq_config,
	    "speech");
	block_config->natural_dynamics_config.enabled = 1;
	block_config->limiter_ceiling = 0.89125094f;
	audio_config->ssb_config = block_config->ssb_config;
	audio_config->am_config = block_config->am_config;
	audio_config->multiband_enabled = block_config->multiband_enabled;
	audio_config->multiband_preset = block_config->multiband_preset;
	audio_config->bass_eq_config = block_config->bass_eq_config;
	audio_config->natural_dynamics_config =
	    block_config->natural_dynamics_config;
	return CP_BULLETIN_OK;
}

static int
cp_bulletin_apply_vhf_fm(struct cp_block_config *block_config,
	struct cp_audio_config *audio_config)
{
	(void)cp_audio_config_set_format(audio_config, CP_CHANNELS_MONO,
	    48000.0);
	block_config->channels = CP_CHANNELS_MONO;
	block_config->sample_rate = 48000.0f;
	(void)cp_ssb_apply_preset(&block_config->ssb_config, "vhf-fm-voice");
	block_config->ssb_config.enabled = 1;
	block_config->am_config.enabled = 0;
	block_config->multiband_enabled = 1;
	block_config->multiband_preset = CP_MULTIBAND_PRESET_SPEECH;
	block_config->bass_eq_config.enabled = 0;
	block_config->natural_dynamics_config.enabled = 1;
	block_config->limiter_ceiling = 0.89125094f;
	audio_config->ssb_config = block_config->ssb_config;
	audio_config->am_config = block_config->am_config;
	audio_config->multiband_enabled = block_config->multiband_enabled;
	audio_config->multiband_preset = block_config->multiband_preset;
	audio_config->bass_eq_config = block_config->bass_eq_config;
	audio_config->natural_dynamics_config =
	    block_config->natural_dynamics_config;
	return CP_BULLETIN_OK;
}

static int
cp_bulletin_copy(char *dst, size_t dst_size, const char *src)
{
	int written;

	if (dst == NULL || src == NULL || dst_size == 0)
		return CP_BULLETIN_ERR_NULL;
	written = snprintf(dst, dst_size, "%s", src);
	if (written < 0 || (size_t)written >= dst_size)
		return CP_BULLETIN_ERR_BUFFER;

	return CP_BULLETIN_OK;
}

static int
cp_bulletin_parse_item_type(const char *text, enum cp_bulletin_item_type *type)
{
	if (text == NULL || type == NULL)
		return CP_BULLETIN_ERR_NULL;
	if (strcmp(text, "id") == 0) {
		*type = CP_BULLETIN_ITEM_ID;
		return CP_BULLETIN_OK;
	}
	if (strcmp(text, "file") == 0) {
		*type = CP_BULLETIN_ITEM_FILE;
		return CP_BULLETIN_OK;
	}
	if (strcmp(text, "tts") == 0) {
		*type = CP_BULLETIN_ITEM_TTS;
		return CP_BULLETIN_OK;
	}
	if (strcmp(text, "pause") == 0) {
		*type = CP_BULLETIN_ITEM_PAUSE;
		return CP_BULLETIN_OK;
	}

	return CP_BULLETIN_ERR_RANGE;
}

static int
cp_bulletin_parse_uint(const char *text, unsigned int *value)
{
	char *end;
	unsigned long parsed;

	if (text == NULL || value == NULL || text[0] == '\0' ||
	    text[0] == '-')
		return CP_BULLETIN_ERR_NULL;

	parsed = strtoul(text, &end, 10);
	if (end == text || *end != '\0' || parsed > 4294967295UL)
		return CP_BULLETIN_ERR_RANGE;

	*value = (unsigned int)parsed;
	return CP_BULLETIN_OK;
}

static int
cp_bulletin_schedule_add_item(struct cp_bulletin_schedule *schedule)
{
	struct cp_bulletin_item *item;

	if (schedule == NULL)
		return CP_BULLETIN_ERR_NULL;
	if (schedule->item_count >= CP_BULLETIN_ITEMS_MAX)
		return CP_BULLETIN_ERR_RANGE;

	item = &schedule->items[schedule->item_count++];
	(void)memset(item, 0, sizeof(*item));
	item->type = CP_BULLETIN_ITEM_FILE;
	return CP_BULLETIN_OK;
}

static int
cp_bulletin_set_error(char *error, size_t error_size, size_t line_number,
	const char *message)
{
	int written;

	if (error != NULL && error_size > 0) {
		if (line_number > 0) {
			written = snprintf(error, error_size, "line %zu: %s",
			    line_number, message == NULL ? "error" : message);
		} else {
			written = snprintf(error, error_size, "%s",
			    message == NULL ? "error" : message);
		}
		if (written < 0 || (size_t)written >= error_size)
			error[error_size - 1] = '\0';
	}

	return CP_BULLETIN_ERR_PARSE;
}

static char *
cp_bulletin_trim(char *text)
{
	char *end;

	while (isspace((unsigned char)*text))
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

static char *
cp_bulletin_unquote(char *text)
{
	size_t length;

	length = strlen(text);
	if (length >= 2 && text[0] == '"' && text[length - 1] == '"') {
		text[length - 1] = '\0';
		return text + 1;
	}

	return text;
}

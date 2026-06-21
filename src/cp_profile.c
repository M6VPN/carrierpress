/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_profile.c */

#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cp_profile.h"

static int	cp_profile_copy_text(char *, size_t, const char *);
static int	cp_profile_forbidden_key(const char *);
static int	cp_profile_parse_am_preset(const char *,
		    enum cp_profile_am_preset_setting *);
static int	cp_profile_parse_bass_eq(const char *,
		    enum cp_profile_bass_eq_setting *);
static int	cp_profile_parse_multiband(const char *,
		    enum cp_profile_multiband_setting *);
static int	cp_profile_parse_ssb_preset(const char *,
		    enum cp_profile_ssb_preset_setting *);
static int	cp_profile_parse_switch(const char *,
		    enum cp_profile_switch *);
static int	cp_profile_parse_uint(const char *, unsigned long *);
static int	cp_profile_set_error(struct cp_profile_error *, size_t,
		    const char *, const char *);
static char	*cp_profile_trim(char *);
static int	cp_profile_value_seen(unsigned int, size_t, const char *,
		    struct cp_profile_error *);

void
cp_profile_init(struct cp_profile *profile)
{
	if (profile == NULL)
		return;

	(void)memset(profile, 0, sizeof(*profile));
	profile->mode = CP_PROFILE_MODE_UNSET;
	profile->dehummer = CP_PROFILE_SWITCH_UNSET;
	profile->multiband = CP_PROFILE_MULTIBAND_UNSET;
	profile->multiband2 = CP_PROFILE_MULTIBAND_UNSET;
	profile->bass_eq = CP_PROFILE_BASS_EQ_UNSET;
	profile->natural_dynamics = CP_PROFILE_SWITCH_UNSET;
	profile->low_level_boost = CP_PROFILE_SWITCH_UNSET;
	profile->restoration_analysis = CP_PROFILE_SWITCH_UNSET;
	profile->declipper = CP_PROFILE_SWITCH_UNSET;
	profile->am_preset = CP_PROFILE_AM_PRESET_UNSET;
	profile->ssb_preset = CP_PROFILE_SSB_PRESET_UNSET;
}

int
cp_profile_mode_from_string(const char *text, enum cp_profile_mode *mode)
{
	if (text == NULL || mode == NULL)
		return CP_ERR_NULL;
	if (strcmp(text, "neutral") == 0) {
		*mode = CP_PROFILE_MODE_NEUTRAL;
		return CP_OK;
	}
	if (strcmp(text, "am") == 0) {
		*mode = CP_PROFILE_MODE_AM;
		return CP_OK;
	}
	if (strcmp(text, "ssb") == 0) {
		*mode = CP_PROFILE_MODE_SSB;
		return CP_OK;
	}
	if (strcmp(text, "file-cleanup") == 0) {
		*mode = CP_PROFILE_MODE_FILE_CLEANUP;
		return CP_OK;
	}

	return CP_ERR_RANGE;
}

const char *
cp_profile_mode_string(enum cp_profile_mode mode)
{
	switch (mode) {
	case CP_PROFILE_MODE_NEUTRAL:
		return "neutral";
	case CP_PROFILE_MODE_AM:
		return "am";
	case CP_PROFILE_MODE_SSB:
		return "ssb";
	case CP_PROFILE_MODE_FILE_CLEANUP:
		return "file-cleanup";
	default:
		return "unset";
	}
}

int
cp_profile_parse_file(const char *path, struct cp_profile *profile,
	struct cp_profile_error *error)
{
	char line[CP_PROFILE_LINE_SIZE];
	FILE *file;
	size_t line_number;
	size_t length;
	int status;
	int ch;

	if (path == NULL || profile == NULL)
		return CP_ERR_NULL;

	file = fopen(path, "r");
	if (file == NULL)
		return cp_profile_set_error(error, 0, "", "could not open file");

	cp_profile_init(profile);
	line_number = 0;
	while (fgets(line, (int)sizeof(line), file) != NULL) {
		line_number++;
		length = strlen(line);
		if (length > 0 && line[length - 1] != '\n' && !feof(file)) {
			while ((ch = fgetc(file)) != '\n' && ch != EOF)
				;
			(void)fclose(file);
			return cp_profile_set_error(error, line_number, "",
			    "line too long");
		}
		status = cp_profile_parse_line(line, line_number, profile,
		    error);
		if (status != CP_OK) {
			(void)fclose(file);
			return status;
		}
	}

	if (ferror(file)) {
		(void)fclose(file);
		return cp_profile_set_error(error, line_number, "",
		    "file read failed");
	}
	(void)fclose(file);

	return cp_profile_validate(profile, error);
}

int
cp_profile_parse_line(const char *line, size_t line_number,
	struct cp_profile *profile, struct cp_profile_error *error)
{
	char buffer[CP_PROFILE_LINE_SIZE];
	char *equals;
	char *key;
	char *value;
	unsigned long parsed;

	if (line == NULL || profile == NULL)
		return CP_ERR_NULL;
	if (strlen(line) >= sizeof(buffer))
		return cp_profile_set_error(error, line_number, "",
		    "line too long");

	(void)snprintf(buffer, sizeof(buffer), "%s", line);
	buffer[strcspn(buffer, "\r\n")] = '\0';
	key = cp_profile_trim(buffer);
	if (key[0] == '\0' || key[0] == '#')
		return CP_OK;

	equals = strchr(key, '=');
	if (equals == NULL)
		return cp_profile_set_error(error, line_number, key,
		    "expected key = value");
	*equals = '\0';
	value = cp_profile_trim(equals + 1);
	key = cp_profile_trim(key);
	if (key[0] == '\0')
		return cp_profile_set_error(error, line_number, key,
		    "empty key");
	if (value[0] == '\0')
		return cp_profile_set_error(error, line_number, key,
		    "empty value");
	if (strchr(value, '=') != NULL)
		return cp_profile_set_error(error, line_number, key,
		    "invalid value");
	if (cp_profile_forbidden_key(key))
		return cp_profile_set_error(error, line_number, key,
		    "station-control key is not allowed in profiles");

	if (strcmp(key, "name") == 0) {
		if (cp_profile_value_seen(profile->seen_name, line_number,
		    key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_copy_text(profile->name,
		    sizeof(profile->name), value) != CP_OK)
			return cp_profile_set_error(error, line_number, key,
			    "value too long");
		profile->seen_name = 1;
		return CP_OK;
	}
	if (strcmp(key, "description") == 0) {
		if (cp_profile_value_seen(profile->seen_description,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_copy_text(profile->description,
		    sizeof(profile->description), value) != CP_OK)
			return cp_profile_set_error(error, line_number, key,
			    "value too long");
		profile->seen_description = 1;
		return CP_OK;
	}
	if (strcmp(key, "mode") == 0) {
		if (cp_profile_value_seen(profile->seen_mode, line_number,
		    key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_mode_from_string(value, &profile->mode) !=
		    CP_OK)
			return cp_profile_set_error(error, line_number, key,
			    "invalid mode");
		profile->seen_mode = 1;
		return CP_OK;
	}
	if (strcmp(key, "dehummer") == 0) {
		if (cp_profile_value_seen(profile->seen_dehummer,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_parse_switch(value, &profile->dehummer) !=
		    CP_OK)
			return cp_profile_set_error(error, line_number, key,
			    "expected off or on");
		profile->seen_dehummer = 1;
		return CP_OK;
	}
	if (strcmp(key, "hum_frequency") == 0) {
		if (cp_profile_value_seen(profile->seen_hum_frequency,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_parse_uint(value, &parsed) != CP_OK ||
		    (parsed != 50UL && parsed != 60UL))
			return cp_profile_set_error(error, line_number, key,
			    "expected 50 or 60");
		profile->hum_frequency = (unsigned int)parsed;
		profile->seen_hum_frequency = 1;
		return CP_OK;
	}
	if (strcmp(key, "hum_harmonics") == 0) {
		if (cp_profile_value_seen(profile->seen_hum_harmonics,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_parse_uint(value, &parsed) != CP_OK ||
		    parsed == 0UL || parsed > CP_DEHUMMER_MAX_HARMONICS)
			return cp_profile_set_error(error, line_number, key,
			    "invalid harmonic count");
		profile->hum_harmonics = (size_t)parsed;
		profile->seen_hum_harmonics = 1;
		return CP_OK;
	}
	if (strcmp(key, "multiband") == 0) {
		if (cp_profile_value_seen(profile->seen_multiband,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_parse_multiband(value,
		    &profile->multiband) != CP_OK)
			return cp_profile_set_error(error, line_number, key,
			    "invalid multiband preset");
		profile->seen_multiband = 1;
		return CP_OK;
	}
	if (strcmp(key, "multiband_bands") == 0) {
		if (cp_profile_value_seen(profile->seen_multiband_bands,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_parse_uint(value, &parsed) != CP_OK ||
		    parsed < CP_MULTIBAND_MIN_BANDS ||
		    parsed > CP_MULTIBAND_M5_MAX_BANDS)
			return cp_profile_set_error(error, line_number, key,
			    "invalid band count");
		profile->multiband_bands = (size_t)parsed;
		profile->seen_multiband_bands = 1;
		return CP_OK;
	}
	if (strcmp(key, "multiband2") == 0) {
		if (cp_profile_value_seen(profile->seen_multiband2,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_parse_multiband(value,
		    &profile->multiband2) != CP_OK)
			return cp_profile_set_error(error, line_number, key,
			    "invalid multiband preset");
		profile->seen_multiband2 = 1;
		return CP_OK;
	}
	if (strcmp(key, "multiband2_bands") == 0) {
		if (cp_profile_value_seen(profile->seen_multiband2_bands,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_parse_uint(value, &parsed) != CP_OK ||
		    parsed < CP_MULTIBAND_MIN_BANDS ||
		    parsed > CP_MULTIBAND_M5_MAX_BANDS)
			return cp_profile_set_error(error, line_number, key,
			    "invalid band count");
		profile->multiband2_bands = (size_t)parsed;
		profile->seen_multiband2_bands = 1;
		return CP_OK;
	}
	if (strcmp(key, "bass_eq") == 0) {
		if (cp_profile_value_seen(profile->seen_bass_eq,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_parse_bass_eq(value, &profile->bass_eq) !=
		    CP_OK)
			return cp_profile_set_error(error, line_number, key,
			    "invalid bass EQ preset");
		profile->seen_bass_eq = 1;
		return CP_OK;
	}
	if (strcmp(key, "natural_dynamics") == 0) {
		if (cp_profile_value_seen(profile->seen_natural_dynamics,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_parse_switch(value,
		    &profile->natural_dynamics) != CP_OK)
			return cp_profile_set_error(error, line_number, key,
			    "expected off or on");
		profile->seen_natural_dynamics = 1;
		return CP_OK;
	}
	if (strcmp(key, "low_level_boost") == 0) {
		if (cp_profile_value_seen(profile->seen_low_level_boost,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_parse_switch(value,
		    &profile->low_level_boost) != CP_OK)
			return cp_profile_set_error(error, line_number, key,
			    "expected off or on");
		profile->seen_low_level_boost = 1;
		return CP_OK;
	}
	if (strcmp(key, "restoration_analysis") == 0) {
		if (cp_profile_value_seen(profile->seen_restoration_analysis,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_parse_switch(value,
		    &profile->restoration_analysis) != CP_OK)
			return cp_profile_set_error(error, line_number, key,
			    "expected off or on");
		profile->seen_restoration_analysis = 1;
		return CP_OK;
	}
	if (strcmp(key, "declipper") == 0) {
		if (cp_profile_value_seen(profile->seen_declipper,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_parse_switch(value, &profile->declipper) !=
		    CP_OK)
			return cp_profile_set_error(error, line_number, key,
			    "expected off or on");
		profile->seen_declipper = 1;
		return CP_OK;
	}
	if (strcmp(key, "am_preset") == 0) {
		if (cp_profile_value_seen(profile->seen_am_preset,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_parse_am_preset(value,
		    &profile->am_preset) != CP_OK)
			return cp_profile_set_error(error, line_number, key,
			    "invalid AM preset");
		profile->seen_am_preset = 1;
		return CP_OK;
	}
	if (strcmp(key, "ssb_preset") == 0) {
		if (cp_profile_value_seen(profile->seen_ssb_preset,
		    line_number, key, error) != CP_OK)
			return CP_ERR_RANGE;
		if (cp_profile_parse_ssb_preset(value,
		    &profile->ssb_preset) != CP_OK)
			return cp_profile_set_error(error, line_number, key,
			    "invalid SSB preset");
		profile->seen_ssb_preset = 1;
		return CP_OK;
	}

	return cp_profile_set_error(error, line_number, key, "unknown key");
}

int
cp_profile_validate(const struct cp_profile *profile,
	struct cp_profile_error *error)
{
	if (profile == NULL)
		return CP_ERR_NULL;
	if (!profile->seen_name || profile->name[0] == '\0')
		return cp_profile_set_error(error, 0, "name",
		    "name is required");
	if (!profile->seen_mode || profile->mode == CP_PROFILE_MODE_UNSET)
		return cp_profile_set_error(error, 0, "mode",
		    "mode is required");
	if (profile->mode == CP_PROFILE_MODE_AM &&
	    profile->ssb_preset != CP_PROFILE_SSB_PRESET_UNSET &&
	    profile->ssb_preset != CP_PROFILE_SSB_PRESET_OFF)
		return cp_profile_set_error(error, 0, "ssb_preset",
		    "AM profiles must not enable SSB presets");
	if (profile->mode == CP_PROFILE_MODE_SSB &&
	    profile->am_preset != CP_PROFILE_AM_PRESET_UNSET &&
	    profile->am_preset != CP_PROFILE_AM_PRESET_OFF)
		return cp_profile_set_error(error, 0, "am_preset",
		    "SSB profiles must not enable AM presets");
	if ((profile->mode == CP_PROFILE_MODE_NEUTRAL ||
	    profile->mode == CP_PROFILE_MODE_FILE_CLEANUP) &&
	    profile->am_preset != CP_PROFILE_AM_PRESET_UNSET &&
	    profile->am_preset != CP_PROFILE_AM_PRESET_OFF)
		return cp_profile_set_error(error, 0, "am_preset",
		    "neutral and file-cleanup profiles must not force AM");
	if ((profile->mode == CP_PROFILE_MODE_NEUTRAL ||
	    profile->mode == CP_PROFILE_MODE_FILE_CLEANUP) &&
	    profile->ssb_preset != CP_PROFILE_SSB_PRESET_UNSET &&
	    profile->ssb_preset != CP_PROFILE_SSB_PRESET_OFF)
		return cp_profile_set_error(error, 0, "ssb_preset",
		    "neutral and file-cleanup profiles must not force SSB");

	return CP_OK;
}

static int
cp_profile_copy_text(char *dst, size_t dst_size, const char *src)
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
cp_profile_forbidden_key(const char *key)
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
		"station_control"
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
cp_profile_parse_am_preset(const char *text,
	enum cp_profile_am_preset_setting *preset)
{
	if (text == NULL || preset == NULL)
		return CP_ERR_NULL;
	if (strcmp(text, "off") == 0) {
		*preset = CP_PROFILE_AM_PRESET_OFF;
		return CP_OK;
	}
	if (strcmp(text, "am-safe") == 0) {
		*preset = CP_PROFILE_AM_PRESET_SAFE;
		return CP_OK;
	}
	if (strcmp(text, "am-shortwave") == 0) {
		*preset = CP_PROFILE_AM_PRESET_SHORTWAVE;
		return CP_OK;
	}
	if (strcmp(text, "am-wide") == 0) {
		*preset = CP_PROFILE_AM_PRESET_WIDE;
		return CP_OK;
	}
	if (strcmp(text, "am-voice") == 0) {
		*preset = CP_PROFILE_AM_PRESET_VOICE;
		return CP_OK;
	}

	return CP_ERR_RANGE;
}

static int
cp_profile_parse_bass_eq(const char *text,
	enum cp_profile_bass_eq_setting *setting)
{
	if (text == NULL || setting == NULL)
		return CP_ERR_NULL;
	if (strcmp(text, "off") == 0) {
		*setting = CP_PROFILE_BASS_EQ_OFF;
		return CP_OK;
	}
	if (strcmp(text, "warm") == 0) {
		*setting = CP_PROFILE_BASS_EQ_WARM;
		return CP_OK;
	}
	if (strcmp(text, "music") == 0) {
		*setting = CP_PROFILE_BASS_EQ_MUSIC;
		return CP_OK;
	}
	if (strcmp(text, "speech") == 0) {
		*setting = CP_PROFILE_BASS_EQ_SPEECH;
		return CP_OK;
	}

	return CP_ERR_RANGE;
}

static int
cp_profile_parse_multiband(const char *text,
	enum cp_profile_multiband_setting *setting)
{
	if (text == NULL || setting == NULL)
		return CP_ERR_NULL;
	if (strcmp(text, "off") == 0) {
		*setting = CP_PROFILE_MULTIBAND_OFF;
		return CP_OK;
	}
	if (strcmp(text, "speech") == 0) {
		*setting = CP_PROFILE_MULTIBAND_SPEECH;
		return CP_OK;
	}
	if (strcmp(text, "music") == 0) {
		*setting = CP_PROFILE_MULTIBAND_MUSIC;
		return CP_OK;
	}

	return CP_ERR_RANGE;
}

static int
cp_profile_parse_ssb_preset(const char *text,
	enum cp_profile_ssb_preset_setting *preset)
{
	if (text == NULL || preset == NULL)
		return CP_ERR_NULL;
	if (strcmp(text, "off") == 0) {
		*preset = CP_PROFILE_SSB_PRESET_OFF;
		return CP_OK;
	}
	if (strcmp(text, "ssb-speech") == 0) {
		*preset = CP_PROFILE_SSB_PRESET_SPEECH;
		return CP_OK;
	}
	if (strcmp(text, "ssb-narrow") == 0) {
		*preset = CP_PROFILE_SSB_PRESET_NARROW;
		return CP_OK;
	}
	if (strcmp(text, "ssb-wide") == 0) {
		*preset = CP_PROFILE_SSB_PRESET_WIDE;
		return CP_OK;
	}
	if (strcmp(text, "ssb-gentle") == 0) {
		*preset = CP_PROFILE_SSB_PRESET_GENTLE;
		return CP_OK;
	}

	return CP_ERR_RANGE;
}

static int
cp_profile_parse_switch(const char *text, enum cp_profile_switch *setting)
{
	if (text == NULL || setting == NULL)
		return CP_ERR_NULL;
	if (strcmp(text, "off") == 0) {
		*setting = CP_PROFILE_SWITCH_OFF;
		return CP_OK;
	}
	if (strcmp(text, "on") == 0) {
		*setting = CP_PROFILE_SWITCH_ON;
		return CP_OK;
	}

	return CP_ERR_RANGE;
}

static int
cp_profile_parse_uint(const char *text, unsigned long *value)
{
	char *end;
	unsigned long parsed;

	if (text == NULL || value == NULL)
		return CP_ERR_NULL;

	errno = 0;
	parsed = strtoul(text, &end, 10);
	if (errno != 0 || end == text || *end != '\0')
		return CP_ERR_RANGE;

	*value = parsed;
	return CP_OK;
}

static int
cp_profile_set_error(struct cp_profile_error *error, size_t line_number,
	const char *key, const char *message)
{
	if (error != NULL) {
		error->line_number = line_number;
		if (key != NULL) {
			(void)snprintf(error->key, sizeof(error->key), "%s",
			    key);
		} else {
			error->key[0] = '\0';
		}
		if (message != NULL) {
			(void)snprintf(error->message, sizeof(error->message),
			    "%s", message);
		} else {
			error->message[0] = '\0';
		}
	}

	return CP_ERR_RANGE;
}

static char *
cp_profile_trim(char *text)
{
	char *end;

	if (text == NULL)
		return NULL;
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

static int
cp_profile_value_seen(unsigned int seen, size_t line_number, const char *key,
	struct cp_profile_error *error)
{
	if (seen)
		return cp_profile_set_error(error, line_number, key,
		    "duplicate key");

	return CP_OK;
}

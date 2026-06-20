/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_cat.c */

#include <sys/types.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cp_cat.h"
#ifdef CP_WITH_FLRIG
#include "cp_cat_flrig.h"
#endif
#ifdef CP_WITH_HAMLIB
#include "cp_cat_hamlib.h"
#endif

static int	cp_cat_snprintf(char *, size_t, const char *, ...);
static int	cp_cat_string_equal(const char *, const char *);
static int	cp_cat_text_copy(char *, size_t, const char *);

const char *
cp_cat_backend_string(enum cp_cat_backend backend)
{
	switch (backend) {
	case CP_CAT_BACKEND_NONE:
		return "none";
	case CP_CAT_BACKEND_MOCK:
		return "mock";
	case CP_CAT_BACKEND_FLRIG:
		return "flrig";
	case CP_CAT_BACKEND_HAMLIB:
		return "hamlib";
	}

	return "unknown";
}

int
cp_cat_backend_from_string(const char *text, enum cp_cat_backend *backend)
{
	if (text == NULL || backend == NULL)
		return CP_ERR_NULL;
	if (cp_cat_string_equal(text, "none")) {
		*backend = CP_CAT_BACKEND_NONE;
		return CP_OK;
	}
	if (cp_cat_string_equal(text, "mock")) {
		*backend = CP_CAT_BACKEND_MOCK;
		return CP_OK;
	}
	if (cp_cat_string_equal(text, "flrig")) {
		*backend = CP_CAT_BACKEND_FLRIG;
		return CP_OK;
	}
	if (cp_cat_string_equal(text, "hamlib")) {
		*backend = CP_CAT_BACKEND_HAMLIB;
		return CP_OK;
	}

	return CP_ERR_RANGE;
}

int
cp_cat_config_set_flrig_host(struct cp_cat_config *config, const char *host)
{
	if (config == NULL || host == NULL)
		return CP_ERR_NULL;
	if (host[0] == '\0')
		return CP_ERR_RANGE;

	return cp_cat_text_copy(config->flrig_host,
	    sizeof(config->flrig_host), host);
}

int
cp_cat_config_set_hamlib_path(struct cp_cat_config *config, const char *path)
{
	if (config == NULL || path == NULL)
		return CP_ERR_NULL;

	return cp_cat_text_copy(config->hamlib_rig_path,
	    sizeof(config->hamlib_rig_path), path);
}

void
cp_cat_default_config(struct cp_cat_config *config)
{
	if (config == NULL)
		return;

	memset(config, 0, sizeof(*config));
	config->backend = CP_CAT_BACKEND_NONE;
	config->enabled = 0;
	config->mock_frequency_hz = CP_CAT_DEFAULT_FREQUENCY_HZ;
	(void)cp_cat_text_copy(config->mock_mode,
	    sizeof(config->mock_mode), "USB");
	config->mock_ptt = CP_CAT_PTT_UNKNOWN;
	config->mock_status = CP_CAT_STATUS_OK;
	(void)cp_cat_config_set_flrig_host(config,
	    CP_CAT_FLRIG_DEFAULT_HOST);
	config->flrig_port = CP_CAT_FLRIG_DEFAULT_PORT;
	config->timeout_ms = CP_CAT_DEFAULT_TIMEOUT_MS;
	config->hamlib_rig_model = CP_CAT_HAMLIB_DEFAULT_RIG_MODEL;
	config->hamlib_rig_speed = CP_CAT_HAMLIB_DEFAULT_RIG_SPEED;
}

int
cp_cat_format_frequency(uint64_t frequency_hz, char *buffer,
	size_t buffer_size)
{
	return cp_cat_snprintf(buffer, buffer_size, "%llu Hz",
	    (unsigned long long)frequency_hz);
}

int
cp_cat_mode_set(char *buffer, size_t buffer_size, const char *mode)
{
	size_t index;
	size_t length;
	unsigned char ch;

	if (buffer == NULL || mode == NULL || buffer_size == 0)
		return CP_ERR_NULL;

	length = strlen(mode);
	if (length == 0 || length >= buffer_size)
		return CP_ERR_RANGE;

	for (index = 0; index < length; index++) {
		ch = (unsigned char)mode[index];
		if (!isalnum(ch) && ch != '-' && ch != '_')
			return CP_ERR_RANGE;
	}

	return cp_cat_text_copy(buffer, buffer_size, mode);
}

const char *
cp_cat_ptt_string(enum cp_cat_ptt_state ptt)
{
	switch (ptt) {
	case CP_CAT_PTT_UNKNOWN:
		return "unknown";
	case CP_CAT_PTT_OFF:
		return "RX";
	case CP_CAT_PTT_ON:
		return "TX";
	}

	return "unknown";
}

int
cp_cat_ptt_from_string(const char *text, enum cp_cat_ptt_state *ptt)
{
	if (text == NULL || ptt == NULL)
		return CP_ERR_NULL;
	if (cp_cat_string_equal(text, "unknown")) {
		*ptt = CP_CAT_PTT_UNKNOWN;
		return CP_OK;
	}
	if (cp_cat_string_equal(text, "off") ||
	    cp_cat_string_equal(text, "rx")) {
		*ptt = CP_CAT_PTT_OFF;
		return CP_OK;
	}
	if (cp_cat_string_equal(text, "on") ||
	    cp_cat_string_equal(text, "tx")) {
		*ptt = CP_CAT_PTT_ON;
		return CP_OK;
	}

	return CP_ERR_RANGE;
}

int
cp_cat_snapshot_format(const struct cp_cat_snapshot *snapshot,
	char *buffer, size_t buffer_size)
{
	char frequency[32];

	if (snapshot == NULL || buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;
	if (!snapshot->enabled ||
	    snapshot->status == CP_CAT_STATUS_DISABLED) {
		return cp_cat_snprintf(buffer, buffer_size, "CAT disabled");
	}
	if (cp_cat_format_frequency(snapshot->frequency_hz, frequency,
	    sizeof(frequency)) != CP_OK)
		return CP_ERR_RANGE;

	return cp_cat_snprintf(buffer, buffer_size,
	    "CAT %s %s freq=%s mode=%s ptt=%s",
	    cp_cat_backend_string(snapshot->backend),
	    cp_cat_status_string(snapshot->status), frequency,
	    snapshot->mode, cp_cat_ptt_string(snapshot->ptt));
}

int
cp_cat_snapshot_update(const struct cp_cat_config *config,
	struct cp_cat_snapshot *snapshot)
{
	if (config == NULL || snapshot == NULL)
		return CP_ERR_NULL;

	memset(snapshot, 0, sizeof(*snapshot));
	snapshot->backend = config->backend;
	snapshot->enabled = config->enabled && config->backend !=
	    CP_CAT_BACKEND_NONE;
	snapshot->ptt = CP_CAT_PTT_UNKNOWN;
	(void)cp_cat_text_copy(snapshot->mode, sizeof(snapshot->mode),
	    "unknown");

	if (!snapshot->enabled) {
		snapshot->status = CP_CAT_STATUS_DISABLED;
		(void)cp_cat_text_copy(snapshot->status_text,
		    sizeof(snapshot->status_text), "disabled");
		return CP_OK;
	}

	if (config->backend == CP_CAT_BACKEND_MOCK) {
		snapshot->connected = config->mock_status ==
		    CP_CAT_STATUS_OK || config->mock_status ==
		    CP_CAT_STATUS_STALE;
		snapshot->status = config->mock_status;
		snapshot->frequency_hz = config->mock_frequency_hz;
		snapshot->ptt = config->mock_ptt;
		(void)cp_cat_text_copy(snapshot->mode,
		    sizeof(snapshot->mode), config->mock_mode);
		(void)cp_cat_text_copy(snapshot->status_text,
		    sizeof(snapshot->status_text),
		    cp_cat_status_string(snapshot->status));
		return CP_OK;
	}

	if (config->backend == CP_CAT_BACKEND_FLRIG) {
#ifdef CP_WITH_FLRIG
		return cp_cat_flrig_snapshot_update(config, snapshot);
#else
		snapshot->status = CP_CAT_STATUS_UNAVAILABLE;
		(void)cp_cat_text_copy(snapshot->status_text,
		    sizeof(snapshot->status_text),
		    "flrig support not enabled");
		return CP_OK;
#endif
	}

	if (config->backend == CP_CAT_BACKEND_HAMLIB) {
#ifdef CP_WITH_HAMLIB
		return cp_cat_hamlib_snapshot_update(config, snapshot);
#else
		snapshot->status = CP_CAT_STATUS_UNAVAILABLE;
		(void)cp_cat_text_copy(snapshot->status_text,
		    sizeof(snapshot->status_text),
		    "hamlib support not enabled");
		return CP_OK;
#endif
	}

	snapshot->status = CP_CAT_STATUS_UNAVAILABLE;
	(void)cp_cat_text_copy(snapshot->status_text,
	    sizeof(snapshot->status_text), "backend reserved");

	return CP_OK;
}

const char *
cp_cat_status_string(enum cp_cat_status status)
{
	switch (status) {
	case CP_CAT_STATUS_DISABLED:
		return "disabled";
	case CP_CAT_STATUS_UNAVAILABLE:
		return "unavailable";
	case CP_CAT_STATUS_OK:
		return "ok";
	case CP_CAT_STATUS_STALE:
		return "stale";
	case CP_CAT_STATUS_ERROR:
		return "error";
	}

	return "unknown";
}

int
cp_cat_validate_config(const struct cp_cat_config *config)
{
	if (config == NULL)
		return CP_ERR_NULL;
	if (config->backend == CP_CAT_BACKEND_FLRIG) {
		if (!config->enabled)
			return CP_OK;
		if (config->flrig_host[0] == '\0')
			return CP_ERR_RANGE;
		if (config->flrig_port < CP_CAT_MIN_PORT ||
		    config->flrig_port > CP_CAT_MAX_PORT)
			return CP_ERR_RANGE;
		if (config->timeout_ms < CP_CAT_MIN_TIMEOUT_MS ||
		    config->timeout_ms > CP_CAT_MAX_TIMEOUT_MS)
			return CP_ERR_RANGE;
	}
	if (config->backend == CP_CAT_BACKEND_HAMLIB) {
		if (!config->enabled)
			return CP_OK;
		if (config->timeout_ms < CP_CAT_MIN_TIMEOUT_MS ||
		    config->timeout_ms > CP_CAT_MAX_TIMEOUT_MS)
			return CP_ERR_RANGE;
		if (config->hamlib_rig_speed != 0 &&
		    (config->hamlib_rig_speed < CP_CAT_MIN_RIG_SPEED ||
		    config->hamlib_rig_speed > CP_CAT_MAX_RIG_SPEED))
			return CP_ERR_RANGE;
	}

	return CP_OK;
}

static int
cp_cat_snprintf(char *buffer, size_t buffer_size, const char *format, ...)
{
	va_list ap;
	int written;

	if (buffer == NULL || buffer_size == 0 || format == NULL)
		return CP_ERR_NULL;

	va_start(ap, format);
	written = vsnprintf(buffer, buffer_size, format, ap);
	va_end(ap);
	if (written < 0)
		return CP_ERR_RANGE;
	if ((size_t)written >= buffer_size)
		buffer[buffer_size - 1] = '\0';

	return CP_OK;
}

static int
cp_cat_string_equal(const char *left, const char *right)
{
	unsigned char lch;
	unsigned char rch;

	if (left == NULL || right == NULL)
		return 0;

	while (*left != '\0' && *right != '\0') {
		lch = (unsigned char)*left++;
		rch = (unsigned char)*right++;
		if (tolower(lch) != tolower(rch))
			return 0;
	}

	return *left == '\0' && *right == '\0';
}

static int
cp_cat_text_copy(char *buffer, size_t buffer_size, const char *text)
{
	size_t length;

	if (buffer == NULL || text == NULL || buffer_size == 0)
		return CP_ERR_NULL;

	length = strlen(text);
	if (length >= buffer_size)
		return CP_ERR_RANGE;
	memcpy(buffer, text, length);
	buffer[length] = '\0';

	return CP_OK;
}

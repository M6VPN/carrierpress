/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_operator_state.c */

#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "cp_operator_state.h"

static const char	*cp_operator_text(const char *);
static int		cp_operator_snprintf(char *, size_t,
			    const char *, ...);

void
cp_operator_state_clear(struct cp_operator_state *state)
{
	if (state == NULL)
		return;

	(void)memset(state, 0, sizeof(*state));
}

int
cp_operator_state_format_cue(const struct cp_operator_state *state,
	char *buffer, size_t buffer_size)
{
	if (buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;
	if (state == NULL)
		return cp_operator_snprintf(buffer, buffer_size,
		    "cue=- path=-");

	if (state->playlist_count > 0) {
		return cp_operator_snprintf(buffer, buffer_size,
		    "cue=%zu/%zu path=%s", state->playlist_index + 1,
		    state->playlist_count, cp_operator_text(state->cue_path));
	}

	return cp_operator_snprintf(buffer, buffer_size, "cue=- path=%s",
	    cp_operator_text(state->cue_path));
}

int
cp_operator_state_format_report(const struct cp_operator_state *state,
	char *buffer, size_t buffer_size)
{
	if (buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;
	if (state == NULL)
		return cp_operator_snprintf(buffer, buffer_size,
		    "report=- batch=-");

	return cp_operator_snprintf(buffer, buffer_size,
	    "report=%s batch=%s",
	    state->report_enabled ? cp_operator_text(state->report_path) : "-",
	    state->batch_enabled ? cp_operator_text(state->batch_path) : "-");
}

int
cp_operator_state_format_sources(const struct cp_operator_state *state,
	char *buffer, size_t buffer_size)
{
	if (buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;
	if (state == NULL) {
		return cp_operator_snprintf(buffer, buffer_size,
		    "config=- profile=- name=-");
	}

	return cp_operator_snprintf(buffer, buffer_size,
	    "config=%s profile=%s name=\"%s\"",
	    cp_operator_text(state->config_path),
	    cp_operator_text(state->profile_path),
	    cp_operator_text(state->profile_name));
}

int
cp_operator_state_format_summary(const struct cp_operator_state *state,
	char *buffer, size_t buffer_size)
{
	char cue[128];
	char report[128];
	char sources[192];
	int status;

	if (buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;

	status = cp_operator_state_format_sources(state, sources,
	    sizeof(sources));
	if (status != CP_OK)
		return status;
	status = cp_operator_state_format_cue(state, cue, sizeof(cue));
	if (status != CP_OK)
		return status;
	status = cp_operator_state_format_report(state, report,
	    sizeof(report));
	if (status != CP_OK)
		return status;

	return cp_operator_snprintf(buffer, buffer_size,
	    "Operator: %s | %s | %s", sources, cue, report);
}

static const char *
cp_operator_text(const char *text)
{
	if (text == NULL || text[0] == '\0')
		return "-";

	return text;
}

static int
cp_operator_snprintf(char *buffer, size_t buffer_size, const char *format,
	...)
{
	va_list ap;
	int written;

	if (buffer == NULL || buffer_size == 0 || format == NULL)
		return CP_ERR_NULL;

	va_start(ap, format);
	written = vsnprintf(buffer, buffer_size, format, ap);
	va_end(ap);
	if (written < 0 || (size_t)written >= buffer_size) {
		buffer[buffer_size - 1] = '\0';
		return CP_ERR_RANGE;
	}

	return CP_OK;
}

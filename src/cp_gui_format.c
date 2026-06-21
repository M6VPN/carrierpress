/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_gui_format.c */

#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "cp_agc.h"
#include "cp_gui_format.h"

static const char	*cp_gui_onoff(unsigned int);
static int		cp_gui_snprintf(char *, size_t, const char *, ...);

int
cp_gui_control_command_from_key(int key, enum cp_control_bank bank,
	int next_enabled, struct cp_control_command *command)
{
	int status;

	if (command == NULL)
		return CP_ERR_NULL;

	status = cp_control_command_from_key(key, bank, command);
	if (status != CP_OK)
		return status;
	if (!next_enabled &&
	    command->type == CP_CONTROL_COMMAND_PLAYOUT_NEXT)
		cp_control_command_clear(command);

	return CP_OK;
}

int
cp_gui_format_agc(const struct cp_monitor_snapshot *snapshot,
	char *buffer, size_t buffer_size)
{
	if (snapshot == NULL || buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;

	return cp_gui_snprintf(buffer, buffer_size,
	    "AGC gain %.3f %.2f dB state %s | DSP %s",
	    cp_monitor_level_to_sample(snapshot->agc_gain),
	    cp_monitor_centibel_to_db(snapshot->agc_gain_db_centibel),
	    cp_agc_state_string((enum cp_agc_gate_state)snapshot->agc_state),
	    snapshot->dsp_status == CP_OK ? "ok" : "error");
}

int
cp_gui_format_cat(const struct cp_cat_snapshot *snapshot, char *buffer,
	size_t buffer_size)
{
	if (buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;
	if (snapshot == NULL)
		return cp_gui_snprintf(buffer, buffer_size, "CAT disabled");

	return cp_cat_snapshot_format(snapshot, buffer, buffer_size);
}

int
cp_gui_format_chain(const struct cp_monitor_snapshot *snapshot,
	char *buffer, size_t buffer_size)
{
	if (snapshot == NULL || buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;

	return cp_gui_snprintf(buffer, buffer_size,
	    "Chain: dehummer %s | restoration %s | declipper %s | "
	    "natural dynamics %s | low-level boost %s | MB1 %s | "
	    "bass EQ %s | MB2 %s | AM %s | SSB %s | limiter final",
	    cp_gui_onoff(snapshot->dehummer_enabled),
	    cp_gui_onoff(snapshot->restoration_enabled),
	    cp_gui_onoff(snapshot->declipper_enabled),
	    cp_gui_onoff(snapshot->natural_dynamics_enabled),
	    cp_gui_onoff(snapshot->low_level_boost_enabled),
	    cp_gui_onoff(snapshot->multiband_enabled),
	    cp_gui_onoff(snapshot->bass_eq_enabled),
	    cp_gui_onoff(snapshot->multiband2_enabled),
	    cp_gui_onoff(snapshot->am_enabled),
	    cp_gui_onoff(snapshot->ssb_enabled));
}

int
cp_gui_format_flags(unsigned int flags, char *buffer, size_t buffer_size)
{
	if (buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;
	if (flags == 0)
		return cp_gui_snprintf(buffer, buffer_size, "stream flags: none");

	return cp_gui_snprintf(buffer, buffer_size,
	    "stream flags:%s%s%s%s%s",
	    (flags & CP_MONITOR_INPUT_UNDERFLOW) ? " in_underflow" : "",
	    (flags & CP_MONITOR_INPUT_OVERFLOW) ? " in_overflow" : "",
	    (flags & CP_MONITOR_OUTPUT_UNDERFLOW) ? " out_underflow" : "",
	    (flags & CP_MONITOR_OUTPUT_OVERFLOW) ? " out_overflow" : "",
	    (flags & CP_MONITOR_PRIMING_OUTPUT) ? " priming" : "");
}

int
cp_gui_format_help(enum cp_control_bank bank, int next_enabled,
	char *buffer, size_t buffer_size)
{
	const char *bank_label;
	const char *preset_label;

	if (buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;

	bank_label = bank == CP_CONTROL_BANK_SSB ? "SSB bank" : "AM bank";
	preset_label = bank == CP_CONTROL_BANK_SSB ?
	    "0 SSB off 1-4 SSB presets" : "0 AM off 1-4 AM presets";

	return cp_gui_snprintf(buffer, buffer_size,
	    "Keys: q/Esc stop | %s | a AM s SSB | d hum | m MB1 b MB2 | "
	    "%s | %s",
	    next_enabled ? "n next" : "n next locked", bank_label,
	    preset_label);
}

int
cp_gui_format_meters(const struct cp_monitor_snapshot *snapshot,
	char *buffer, size_t buffer_size)
{
	if (snapshot == NULL || buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;

	return cp_gui_snprintf(buffer, buffer_size,
	    "Meters: in peak %.3f rms %.3f | out peak %.3f rms %.3f",
	    cp_monitor_level_to_sample(snapshot->input_peak),
	    cp_monitor_level_to_sample(snapshot->input_rms),
	    cp_monitor_level_to_sample(snapshot->output_peak),
	    cp_monitor_level_to_sample(snapshot->output_rms));
}

int
cp_gui_format_mode(const struct cp_monitor_snapshot *snapshot,
	char *buffer, size_t buffer_size)
{
	const char *mode;

	if (snapshot == NULL || buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;

	mode = "NEUTRAL";
	if (snapshot->am_enabled)
		mode = "AM";
	else if (snapshot->ssb_enabled)
		mode = "SSB";

	return cp_gui_snprintf(buffer, buffer_size, "Mode: %s", mode);
}

int
cp_gui_format_operator_state(const struct cp_operator_state *state,
	char *buffer, size_t buffer_size)
{
	return cp_operator_state_format_summary(state, buffer, buffer_size);
}

int
cp_gui_format_truncate(const char *text, char *buffer, size_t buffer_size,
	size_t max_chars)
{
	size_t copy;
	size_t length;

	if (text == NULL || buffer == NULL || buffer_size == 0 ||
	    max_chars == 0)
		return CP_ERR_NULL;

	length = strlen(text);
	if (max_chars >= buffer_size)
		max_chars = buffer_size - 1;
	if (length <= max_chars) {
		(void)snprintf(buffer, buffer_size, "%s", text);
		return CP_OK;
	}
	if (max_chars <= 3) {
		copy = max_chars;
		(void)memcpy(buffer, text, copy);
		buffer[copy] = '\0';
		return CP_ERR_RANGE;
	}

	copy = max_chars - 3;
	(void)memcpy(buffer, text, copy);
	(void)memcpy(buffer + copy, "...", 4);

	return CP_ERR_RANGE;
}

int
cp_gui_format_transport(enum cp_gui_mode mode,
	const struct cp_audio_config *config, const char *path,
	size_t playlist_index, size_t playlist_count, int output_device,
	char *buffer, size_t buffer_size)
{
	const char *label;

	if (config == NULL || buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;

	label = "LIVE";
	if (mode == CP_GUI_MODE_PLAYOUT)
		label = playlist_count > 0 ? "PLAYLIST" : "PLAY";
	else if (mode == CP_GUI_MODE_DEMO)
		label = "DEMO";

	if (mode == CP_GUI_MODE_PLAYOUT && playlist_count > 0) {
		return cp_gui_snprintf(buffer, buffer_size,
		    "Transport %s %zu/%zu | %s | rate %.0f Hz | ch %zu | "
		    "out %d",
		    label, playlist_index + 1, playlist_count,
		    path == NULL ? "" : path, config->sample_rate,
		    config->channels, output_device);
	}

	return cp_gui_snprintf(buffer, buffer_size,
	    "Transport %s | %s | rate %.0f Hz | ch %zu | out %d",
	    label, path == NULL ? "" : path, config->sample_rate,
	    config->channels, output_device);
}

static const char *
cp_gui_onoff(unsigned int enabled)
{
	return enabled ? "on" : "off";
}

static int
cp_gui_snprintf(char *buffer, size_t buffer_size, const char *format, ...)
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

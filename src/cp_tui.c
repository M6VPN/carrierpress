/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_tui.c */

#include <sys/types.h>

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <curses.h>

#include "cp_agc.h"
#include "cp_am.h"
#include "cp_auto_eq.h"
#include "cp_bass_eq.h"
#include "cp_declipper.h"
#include "cp_multiband.h"
#include "cp_restoration.h"
#include "cp_ssb.h"
#include "cp_tui.h"

#define CP_TUI_BAR_WIDTH	16
#define CP_TUI_MIN_ROWS		24
#define CP_TUI_MIN_COLS		80
#define CP_TUI_TEXT_SIZE	256

enum cp_tui_processing_mode {
	CP_TUI_PROCESSING_NEUTRAL = 0,
	CP_TUI_PROCESSING_AM,
	CP_TUI_PROCESSING_SSB
};

static enum cp_tui_processing_mode
		cp_tui_processing_mode(
		    const struct cp_monitor_snapshot *);
static int	cp_tui_level_columns(cp_sample_t, cp_sample_t, int);
static int	cp_tui_snprintf(char *, size_t, const char *, ...);
static void	cp_tui_bar(char *, size_t, cp_sample_t, cp_sample_t);
static void	cp_tui_draw_box_line(int, int, const char *, int);
static void	cp_tui_draw_chain(int, int,
		    const struct cp_monitor_snapshot *);
static void	cp_tui_draw_details(int, int,
		    const struct cp_tui_view *);
static void	cp_tui_draw_meters(int, int,
		    const struct cp_monitor_snapshot *);
static void	cp_tui_draw_mode(int, int, const struct cp_tui_view *,
		    enum cp_control_bank);
static void	cp_tui_draw_small_screen(int, int);
static void	cp_tui_draw_transport(int, int,
		    const struct cp_tui_view *, enum cp_control_bank);
static void	cp_tui_draw_text(int, int, int, const char *, ...);
static void	cp_tui_flags_append(char *, size_t, const char *);
static void	cp_tui_format_flags(char *, size_t, unsigned int);
static enum cp_control_bank
		cp_tui_initial_bank(const struct cp_tui_view *);
static const char
		*cp_tui_processing_mode_name(enum cp_tui_processing_mode);

void
cp_tui_close(struct cp_tui *tui)
{
	if (tui == NULL || !tui->active)
		return;

	endwin();
	tui->active = 0;
}

int
cp_tui_init(struct cp_tui *tui)
{
	if (tui == NULL)
		return -1;
	if (initscr() == NULL)
		return -1;

	cbreak();
	noecho();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	curs_set(0);
	tui->active = 1;
	tui->control_bank_set = 0;
	tui->control_bank = CP_CONTROL_BANK_AM;

	return 0;
}

int
cp_tui_update(struct cp_tui *tui, const struct cp_audio_config *config,
	const struct cp_monitor_snapshot *snapshot,
	struct cp_control_command *command)
{
	struct cp_tui_view view;

	memset(&view, 0, sizeof(view));
	view.mode          = CP_TUI_MODE_LIVE;
	view.config        = config;
	view.snapshot      = snapshot;
	view.output_device = config == NULL ? 0 : config->output_device;

	return cp_tui_update_view(tui, &view, command);
}

int
cp_tui_update_view(struct cp_tui *tui, const struct cp_tui_view *view,
	struct cp_control_command *command)
{
	int cols;
	int key;
	int rows;

	if (tui == NULL || view == NULL || view->config == NULL ||
	    view->snapshot == NULL ||
	    command == NULL || !tui->active)
		return 1;
	cp_control_command_clear(command);
	if (!tui->control_bank_set) {
		tui->control_bank = cp_tui_initial_bank(view);
		tui->control_bank_set = 1;
	}

	getmaxyx(stdscr, rows, cols);
	erase();
	if (rows < CP_TUI_MIN_ROWS || cols < CP_TUI_MIN_COLS) {
		cp_tui_draw_small_screen(rows, cols);
	} else {
		cp_tui_draw_transport(rows, cols, view, tui->control_bank);
		cp_tui_draw_mode(rows, cols, view, tui->control_bank);
		cp_tui_draw_meters(rows, cols, view->snapshot);
		cp_tui_draw_chain(rows, cols, view->snapshot);
		cp_tui_draw_details(rows, cols, view);
	}
	refresh();

	key = getch();
	if (key == ERR)
		return 0;
	if (cp_control_command_from_key(key, tui->control_bank, command) !=
	    CP_OK)
		return 0;
	if (command->type == CP_CONTROL_COMMAND_STOP)
		return 1;
	if (command->type == CP_CONTROL_COMMAND_SELECT_AM ||
	    command->type == CP_CONTROL_COMMAND_SELECT_SSB) {
		tui->control_bank = command->bank;
		cp_control_command_clear(command);
		return 0;
	}
	if (command->type == CP_CONTROL_COMMAND_PLAYOUT_NEXT &&
	    !view->next_enabled)
		cp_control_command_clear(command);

	return 0;
}

const char *
cp_tui_active_mode_string(const struct cp_monitor_snapshot *snapshot)
{
	return cp_tui_processing_mode_name(cp_tui_processing_mode(snapshot));
}

int
cp_tui_format_cat_status(const struct cp_cat_snapshot *snapshot,
	char *buffer, size_t buffer_size)
{
	if (buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;
	if (snapshot == NULL)
		return cp_tui_snprintf(buffer, buffer_size, "CAT disabled");

	return cp_cat_snapshot_format(snapshot, buffer, buffer_size);
}

int
cp_tui_format_key_help(const struct cp_tui_view *view,
	enum cp_control_bank bank, char *buffer, size_t buffer_size)
{
	const char *bank_name;
	const char *mode_name;
	const char *next_text;
	const char *preset_text;
	enum cp_tui_processing_mode mode;

	if (view == NULL || view->snapshot == NULL || buffer == NULL ||
	    buffer_size == 0)
		return CP_ERR_NULL;
	if (bank != CP_CONTROL_BANK_AM && bank != CP_CONTROL_BANK_SSB)
		return CP_ERR_RANGE;

	mode = cp_tui_processing_mode(view->snapshot);
	mode_name = cp_tui_processing_mode_name(mode);
	bank_name = bank == CP_CONTROL_BANK_AM ? "AM BANK" : "SSB BANK";
	if (bank == CP_CONTROL_BANK_AM) {
		preset_text = "0 AM off 1 safe 2 short 3 wide 4 voice";
	} else {
		preset_text = "0 SSB off 1 speech 2 narrow 3 wide 4 gentle";
	}
	next_text = view->mode == CP_TUI_MODE_PLAYOUT && view->next_enabled ?
	    " n next" : "";

	return cp_tui_snprintf(buffer, buffer_size,
	    "Keys: q stop%s | a AM bank s SSB bank | d hum m MB1 b MB2 | "
	    "%s | %s | mode %s",
	    next_text, bank_name, preset_text, mode_name);
}

int
cp_tui_format_mode_status(const struct cp_monitor_snapshot *snapshot,
	enum cp_control_bank bank, char *buffer, size_t buffer_size)
{
	const char *am_state;
	const char *ssb_state;
	const char *bank_name;
	enum cp_tui_processing_mode mode;

	if (snapshot == NULL || buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;
	if (bank != CP_CONTROL_BANK_AM && bank != CP_CONTROL_BANK_SSB)
		return CP_ERR_RANGE;

	mode = cp_tui_processing_mode(snapshot);
	bank_name = bank == CP_CONTROL_BANK_AM ? "AM BANK" : "SSB BANK";
	am_state = "AM controls available";
	ssb_state = "SSB controls available";
	if (mode == CP_TUI_PROCESSING_AM) {
		am_state = "AM controls active";
		ssb_state = bank == CP_CONTROL_BANK_SSB ?
		    "SSB bank armed" : "SSB controls locked by AM mode";
	} else if (mode == CP_TUI_PROCESSING_SSB) {
		am_state = bank == CP_CONTROL_BANK_AM ?
		    "AM bank armed" : "AM controls locked by SSB mode";
		ssb_state = "SSB controls active";
	} else if (bank == CP_CONTROL_BANK_AM) {
		ssb_state = "SSB controls locked by AM bank";
	} else {
		am_state = "AM controls locked by SSB bank";
	}

	return cp_tui_snprintf(buffer, buffer_size,
	    "Mode %-7s | Control %-8s | %s | %s",
	    cp_tui_processing_mode_name(mode), bank_name, am_state,
	    ssb_state);
}

static enum cp_tui_processing_mode
cp_tui_processing_mode(const struct cp_monitor_snapshot *snapshot)
{
	if (snapshot == NULL)
		return CP_TUI_PROCESSING_NEUTRAL;
	if (snapshot->am_enabled)
		return CP_TUI_PROCESSING_AM;
	if (snapshot->ssb_enabled)
		return CP_TUI_PROCESSING_SSB;

	return CP_TUI_PROCESSING_NEUTRAL;
}

static int
cp_tui_level_columns(cp_sample_t value, cp_sample_t full_scale, int width)
{
	cp_sample_t ratio;

	if (width <= 0 || !isfinite(value) || value <= 0.0f ||
	    full_scale <= 0.0f)
		return 0;

	ratio = value / full_scale;
	if (ratio >= 1.0f)
		return width;
	if (ratio <= 0.0f)
		return 0;

	return (int)lrintf(ratio * (cp_sample_t)width);
}

static int
cp_tui_snprintf(char *buffer, size_t buffer_size, const char *format, ...)
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

static void
cp_tui_bar(char *buffer, size_t buffer_size, cp_sample_t value,
	cp_sample_t full_scale)
{
	int fill;
	int index;
	size_t offset;

	if (buffer == NULL || buffer_size == 0)
		return;
	fill = cp_tui_level_columns(value, full_scale, CP_TUI_BAR_WIDTH);
	if (buffer_size < (size_t)CP_TUI_BAR_WIDTH + 3u) {
		buffer[0] = '\0';
		return;
	}

	offset = 0;
	buffer[offset++] = '[';
	for (index = 0; index < CP_TUI_BAR_WIDTH; index++)
		buffer[offset++] = index < fill ? '#' : ' ';
	buffer[offset++] = ']';
	buffer[offset] = '\0';
}

static void
cp_tui_draw_box_line(int row, int cols, const char *title, int bottom)
{
	int col;
	int length;
	int start;

	if (row < 0 || cols < 2)
		return;

	mvaddch(row, 0, '+');
	for (col = 1; col < cols - 1; col++)
		mvaddch(row, col, bottom ? '=' : '-');
	mvaddch(row, cols - 1, '+');
	if (title == NULL || title[0] == '\0')
		return;

	length = (int)strlen(title);
	start = 2;
	if (length + start + 1 >= cols)
		length = cols - start - 2;
	if (length > 0)
		mvaddnstr(row, start, title, length);
}

static void
cp_tui_draw_chain(int rows, int cols,
	const struct cp_monitor_snapshot *snapshot)
{
	char mb1[CP_TUI_TEXT_SIZE];
	char mb2[CP_TUI_TEXT_SIZE];

	(void)rows;
	cp_tui_draw_box_line(10, cols, " Processing Chain ", 0);
	cp_tui_draw_text(11, 2, cols, "DC blocker -> dehummer %s -> "
	    "restoration %s -> declipper %s -> Auto EQ %s",
	    snapshot->dehummer_enabled ? "on" : "off",
	    snapshot->restoration_enabled ? "on" : "off",
	    snapshot->declipper_enabled ? "on" : "off",
	    snapshot->auto_eq_enabled ? "on" : "off");
	cp_tui_draw_text(12, 2, cols, "natural dynamics %s -> "
	    "low-level boost %s -> AGC %s -> multiband 1 %s",
	    snapshot->natural_dynamics_enabled ? "on" : "off",
	    snapshot->low_level_boost_enabled ? "on" : "off",
	    cp_agc_state_string(
	    (enum cp_agc_gate_state)snapshot->agc_state),
	    snapshot->multiband_enabled ? "on" : "off");
	cp_tui_draw_text(13, 2, cols, "bass EQ %s -> multiband 2 %s -> "
	    "AM %s -> SSB %s -> limiter -> output meter",
	    snapshot->bass_eq_enabled ? "on" : "off",
	    snapshot->multiband2_enabled ? "on" : "off",
	    snapshot->am_enabled ? "on" : "off",
	    snapshot->ssb_enabled ? "on" : "off");

	cp_tui_snprintf(mb1, sizeof(mb1), "MB1 bands %zu preset %s",
	    snapshot->band_count,
	    cp_multiband_preset_string(
	    (enum cp_multiband_preset)snapshot->multiband_preset));
	cp_tui_snprintf(mb2, sizeof(mb2), "MB2 bands %zu preset %s",
	    snapshot->band2_count,
	    cp_multiband_preset_string(
	    (enum cp_multiband_preset)snapshot->multiband2_preset));
	cp_tui_draw_text(14, 2, cols, "%s | %s", mb1, mb2);
}

static void
cp_tui_draw_details(int rows, int cols, const struct cp_tui_view *view)
{
	const struct cp_monitor_snapshot *snapshot;
	const char *am_preset;
	const char *bass_preset;
	const char *ssb_preset;
	char cat_text[CP_TUI_TEXT_SIZE];
	size_t band;

	(void)rows;
	snapshot = view->snapshot;
	am_preset = cp_am_preset_string((enum cp_am_preset)
	    snapshot->am_preset);
	ssb_preset = cp_ssb_preset_string((enum cp_ssb_preset)
	    snapshot->ssb_preset);
	bass_preset = cp_bass_eq_preset_string((enum cp_bass_eq_preset)
	    snapshot->bass_eq_preset);
	if (am_preset == NULL)
		am_preset = "unknown";
	if (ssb_preset == NULL)
		ssb_preset = "unknown";
	if (bass_preset == NULL)
		bass_preset = "unknown";

	cp_tui_draw_box_line(15, cols, " Detailed Status ", 0);
	cp_tui_draw_text(16, 2, cols, "AM %s preset %s HP %u LP %u pos %.2f "
	    "neg %.2f asym %s %.2f",
	    snapshot->am_enabled ? "on" : "off", am_preset,
	    snapshot->am_highpass_hz, snapshot->am_lowpass_hz,
	    cp_monitor_level_to_sample(snapshot->am_positive_peak),
	    cp_monitor_level_to_sample(snapshot->am_negative_peak),
	    snapshot->am_asymmetry_enabled ? "on" : "off",
	    cp_monitor_level_to_sample(snapshot->am_asymmetry_ratio));
	cp_tui_draw_text(17, 2, cols, "SSB %s preset %s HP %u LP %u peak %.2f "
	    "phase %s | Bass EQ %s preset %s",
	    snapshot->ssb_enabled ? "on" : "off", ssb_preset,
	    snapshot->ssb_highpass_hz, snapshot->ssb_lowpass_hz,
	    cp_monitor_level_to_sample(snapshot->ssb_peak_limit),
	    snapshot->ssb_phase_rotator_enabled ? "on" : "off",
	    snapshot->bass_eq_enabled ? "on" : "off", bass_preset);
	cp_tui_draw_text(18, 2, cols, "Analysis %s profile %s flags 0x%08x "
	    "clip %.3f HF %.3f | Declipper %s repairs %u",
	    snapshot->restoration_enabled ? "on" : "off",
	    cp_restoration_source_profile_string(
	    (enum cp_restoration_source_profile)
	    snapshot->restoration_source_profile),
	    snapshot->restoration_reason_flags,
	    cp_monitor_level_to_sample(snapshot->restoration_clipped_ratio),
	    cp_monitor_level_to_sample(snapshot->restoration_hf_ratio),
	    snapshot->declipper_enabled ? "on" : "off",
	    snapshot->declipper_repaired_samples);
	cp_tui_draw_text(19, 2, cols, "Auto EQ %s rms %.4f tilt %.2f dB | "
	    "Bass recommend %s confidence %.3f",
	    snapshot->auto_eq_enabled ? "on" : "off",
	    cp_monitor_level_to_sample(snapshot->auto_eq_total_rms),
	    cp_monitor_centibel_to_db(
	    snapshot->auto_eq_spectral_tilt_db_centibel),
	    snapshot->bass_eq_recommend_valid ? "valid" : "off",
	    cp_monitor_level_to_sample(
	    snapshot->bass_eq_recommend_confidence));

	for (band = 0; band < snapshot->band_count &&
	    band < CP_MONITOR_MAX_BANDS && band < 4; band++) {
		cp_tui_draw_text(20 + (int)(band / 2), 2 + (int)(band % 2) *
		    (cols / 2), cols, "MB1 B%zu rms %.3f gr %.2f dB",
		    band + 1,
		    cp_monitor_level_to_sample(snapshot->band_rms[band]),
		    cp_monitor_centibel_to_db(
		    snapshot->band_gr_db_centibel[band]));
	}

	if (cp_tui_format_cat_status(view->cat_snapshot, cat_text,
	    sizeof(cat_text)) != CP_OK)
		cat_text[0] = '\0';
	cp_tui_draw_text(22, 2, cols, "%s", cat_text);
}

static void
cp_tui_draw_meters(int rows, int cols,
	const struct cp_monitor_snapshot *snapshot)
{
	char flags[CP_TUI_TEXT_SIZE];
	char in_peak[CP_TUI_BAR_WIDTH + 3];
	char in_rms[CP_TUI_BAR_WIDTH + 3];
	char out_peak[CP_TUI_BAR_WIDTH + 3];
	char out_rms[CP_TUI_BAR_WIDTH + 3];

	(void)rows;
	cp_tui_bar(in_peak, sizeof(in_peak),
	    cp_monitor_level_to_sample(snapshot->input_peak), 1.0f);
	cp_tui_bar(in_rms, sizeof(in_rms),
	    cp_monitor_level_to_sample(snapshot->input_rms), 1.0f);
	cp_tui_bar(out_peak, sizeof(out_peak),
	    cp_monitor_level_to_sample(snapshot->output_peak), 1.0f);
	cp_tui_bar(out_rms, sizeof(out_rms),
	    cp_monitor_level_to_sample(snapshot->output_rms), 1.0f);
	cp_tui_format_flags(flags, sizeof(flags), snapshot->stream_flags);

	cp_tui_draw_box_line(6, cols, " Meters ", 0);
	cp_tui_draw_text(7, 2, cols, "Input  peak %s %.3f  RMS %s %.3f",
	    in_peak, cp_monitor_level_to_sample(snapshot->input_peak),
	    in_rms, cp_monitor_level_to_sample(snapshot->input_rms));
	cp_tui_draw_text(8, 2, cols, "Output peak %s %.3f  RMS %s %.3f",
	    out_peak, cp_monitor_level_to_sample(snapshot->output_peak),
	    out_rms, cp_monitor_level_to_sample(snapshot->output_rms));
	cp_tui_draw_text(9, 2, cols, "AGC gain %.3f %.2f dB state %s | "
	    "Limiter final clamp | Flags %s",
	    cp_monitor_level_to_sample(snapshot->agc_gain),
	    cp_monitor_centibel_to_db(snapshot->agc_gain_db_centibel),
	    cp_agc_state_string(
	    (enum cp_agc_gate_state)snapshot->agc_state),
	    flags);
}

static void
cp_tui_draw_mode(int rows, int cols, const struct cp_tui_view *view,
	enum cp_control_bank bank)
{
	char mode_text[CP_TUI_TEXT_SIZE];

	(void)rows;
	cp_tui_draw_box_line(3, cols, " Mode / Controls ", 0);
	if (cp_tui_format_mode_status(view->snapshot, bank, mode_text,
	    sizeof(mode_text)) != CP_OK)
		mode_text[0] = '\0';
	cp_tui_draw_text(4, 2, cols, "%s", mode_text);
	cp_tui_draw_text(5, 2, cols, "Preset commands only. Audio-chain "
	    "mode display is baseband audio processing, not RF generation.");
}

static void
cp_tui_draw_small_screen(int rows, int cols)
{
	if (rows <= 0 || cols <= 0)
		return;
	cp_tui_draw_text(0, 0, cols, "CarrierPress TUI");
	if (rows > 1)
		cp_tui_draw_text(1, 0, cols, "Terminal too small: need at "
		    "least %dx%d, current %dx%d", CP_TUI_MIN_COLS,
		    CP_TUI_MIN_ROWS, cols, rows);
	if (rows > 2)
		cp_tui_draw_text(2, 0, cols, "Resize terminal or run without "
		    "--tui. Press q to stop.");
}

static void
cp_tui_draw_transport(int rows, int cols, const struct cp_tui_view *view,
	enum cp_control_bank bank)
{
	char keys[CP_TUI_TEXT_SIZE];
	const struct cp_audio_config *config;

	config = view->config;
	cp_tui_draw_box_line(0, cols, " CarrierPress Operator Panel ", 1);
	if (view->mode == CP_TUI_MODE_PLAYOUT) {
		if (view->playlist_count > 0) {
			cp_tui_draw_text(1, 2, cols, "Transport PLAYLIST "
			    "track %zu/%zu | rate %.0f Hz | channels %zu | "
			    "block %zu | output %d",
			    view->playlist_index + 1, view->playlist_count,
			    config->sample_rate, config->channels,
			    config->block_size, view->output_device);
		} else {
			cp_tui_draw_text(1, 2, cols, "Transport PLAY file | "
			    "rate %.0f Hz | channels %zu | block %zu | "
			    "output %d",
			    config->sample_rate, config->channels,
			    config->block_size, view->output_device);
		}
		cp_tui_draw_text(2, 2, cols, "Source %s",
		    view->path == NULL ? "" : view->path);
	} else {
		cp_tui_draw_text(1, 2, cols, "Transport LIVE | rate %.0f Hz | "
		    "channels %zu | block %zu | input %d | output %d",
		    config->sample_rate, config->channels, config->block_size,
		    config->input_device, config->output_device);
		cp_tui_draw_text(2, 2, cols, "Backend %s | device %s",
		    cp_audio_backend_string(config->backend),
		    config->device_name == NULL ||
		    config->device_name[0] == '\0' ? "auto" :
		    config->device_name);
	}

	if (cp_tui_format_key_help(view, bank, keys,
	    sizeof(keys)) == CP_OK && rows > 0)
		cp_tui_draw_text(rows - 1, 1, cols, "%s", keys);
}

static void
cp_tui_draw_text(int row, int col, int cols, const char *format, ...)
{
	char buffer[CP_TUI_TEXT_SIZE];
	va_list ap;
	int available;
	int written;

	if (row < 0 || col < 0 || cols <= col || format == NULL)
		return;

	available = cols - col;
	if (available <= 0)
		return;

	va_start(ap, format);
	written = vsnprintf(buffer, sizeof(buffer), format, ap);
	va_end(ap);
	if (written < 0)
		return;
	if ((size_t)written >= sizeof(buffer))
		buffer[sizeof(buffer) - 1] = '\0';

	mvaddnstr(row, col, buffer, available);
}

static void
cp_tui_flags_append(char *buffer, size_t buffer_size, const char *text)
{
	size_t length;
	size_t used;

	if (buffer == NULL || buffer_size == 0 || text == NULL)
		return;
	used = strlen(buffer);
	length = strlen(text);
	if (used >= buffer_size - 1u || length >= buffer_size - used)
		return;
	(void)strncat(buffer, text, buffer_size - used - 1u);
}

static void
cp_tui_format_flags(char *buffer, size_t buffer_size, unsigned int flags)
{
	if (buffer == NULL || buffer_size == 0)
		return;
	if (flags == 0u) {
		cp_tui_snprintf(buffer, buffer_size, "none");
		return;
	}

	buffer[0] = '\0';
	if ((flags & CP_MONITOR_INPUT_UNDERFLOW) != 0)
		cp_tui_flags_append(buffer, buffer_size, "in_underflow ");
	if ((flags & CP_MONITOR_INPUT_OVERFLOW) != 0)
		cp_tui_flags_append(buffer, buffer_size, "in_overflow ");
	if ((flags & CP_MONITOR_OUTPUT_UNDERFLOW) != 0)
		cp_tui_flags_append(buffer, buffer_size, "out_underflow ");
	if ((flags & CP_MONITOR_OUTPUT_OVERFLOW) != 0)
		cp_tui_flags_append(buffer, buffer_size, "out_overflow ");
	if ((flags & CP_MONITOR_PRIMING_OUTPUT) != 0)
		cp_tui_flags_append(buffer, buffer_size, "priming ");
}

static enum cp_control_bank
cp_tui_initial_bank(const struct cp_tui_view *view)
{
	if (view->snapshot->ssb_enabled || view->config->ssb_config.enabled)
		return CP_CONTROL_BANK_SSB;

	return CP_CONTROL_BANK_AM;
}

static const char *
cp_tui_processing_mode_name(enum cp_tui_processing_mode mode)
{
	switch (mode) {
	case CP_TUI_PROCESSING_AM:
		return "AM";
	case CP_TUI_PROCESSING_SSB:
		return "SSB";
	case CP_TUI_PROCESSING_NEUTRAL:
	default:
		return "NEUTRAL";
	}
}

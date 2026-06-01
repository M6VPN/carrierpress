/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_tui.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>

#include <curses.h>

#include "cp_agc.h"
#include "cp_multiband.h"
#include "cp_tui.h"

#define CP_TUI_BAR_WIDTH	40
#define CP_TUI_QUIT_KEY		'q'

static void	cp_tui_draw_bar(int, int, const char *, cp_sample_t,
		    cp_sample_t);
static void	cp_tui_draw_flags(int, unsigned int);
static void	cp_tui_draw_header(const struct cp_audio_config *);
static void	cp_tui_draw_multiband(int, const struct cp_monitor_snapshot *);
static int	cp_tui_level_columns(cp_sample_t, cp_sample_t);

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

	return 0;
}

int
cp_tui_update(struct cp_tui *tui, const struct cp_audio_config *config,
	const struct cp_monitor_snapshot *snapshot)
{
	cp_sample_t agc_gain;
	int key;

	if (tui == NULL || config == NULL || snapshot == NULL || !tui->active)
		return 1;

	erase();
	cp_tui_draw_header(config);
	cp_tui_draw_bar(4, 2, "Input peak ",
	    cp_monitor_level_to_sample(snapshot->input_peak), 1.0f);
	cp_tui_draw_bar(5, 2, "Input RMS  ",
	    cp_monitor_level_to_sample(snapshot->input_rms), 1.0f);
	cp_tui_draw_bar(7, 2, "Output peak",
	    cp_monitor_level_to_sample(snapshot->output_peak), 1.0f);
	cp_tui_draw_bar(8, 2, "Output RMS ",
	    cp_monitor_level_to_sample(snapshot->output_rms), 1.0f);

	agc_gain = cp_monitor_level_to_sample(snapshot->agc_gain);
	mvprintw(10, 2, "AGC gain %0.3f  gain_db %0.2f  state %s",
	    agc_gain,
	    cp_monitor_centibel_to_db(snapshot->agc_gain_db_centibel),
	    cp_agc_state_string((enum cp_agc_gate_state)snapshot->agc_state));

	cp_tui_draw_multiband(12, snapshot);

	mvprintw(18, 2, "AM %s preset %s HP %0.1f Hz LP %0.1f Hz "
	    "pos %0.2f neg %0.2f asym %s %0.2f",
	    config->am_config.enabled ? "on" : "off",
	    config->am_config.preset_name,
	    config->am_config.highpass_hz,
	    config->am_config.lowpass_hz,
	    config->am_config.positive_peak_limit,
	    config->am_config.negative_peak_limit,
	    config->am_config.asymmetry_enabled ? "on" : "off",
	    config->am_config.asymmetry_ratio);

	cp_tui_draw_flags(20, snapshot->stream_flags);
	mvprintw(22, 2, "DSP status %d", snapshot->dsp_status);
	mvprintw(24, 2, "Press q to stop.");
	refresh();

	key = getch();
	if (key == CP_TUI_QUIT_KEY || key == 'Q')
		return 1;

	return 0;
}

static void
cp_tui_draw_bar(int row, int col, const char *label, cp_sample_t value,
	cp_sample_t full_scale)
{
	int fill;
	int index;

	fill = cp_tui_level_columns(value, full_scale);
	mvprintw(row, col, "%s [", label);
	for (index = 0; index < CP_TUI_BAR_WIDTH; index++)
		addch(index < fill ? '#' : ' ');
	printw("] %0.3f", value);
}

static void
cp_tui_draw_flags(int row, unsigned int flags)
{
	mvprintw(row, 2, "Stream flags:");
	if (flags == 0u) {
		printw(" none");
		return;
	}
	if ((flags & CP_MONITOR_INPUT_UNDERFLOW) != 0)
		printw(" input_underflow");
	if ((flags & CP_MONITOR_INPUT_OVERFLOW) != 0)
		printw(" input_overflow");
	if ((flags & CP_MONITOR_OUTPUT_UNDERFLOW) != 0)
		printw(" output_underflow");
	if ((flags & CP_MONITOR_OUTPUT_OVERFLOW) != 0)
		printw(" output_overflow");
	if ((flags & CP_MONITOR_PRIMING_OUTPUT) != 0)
		printw(" priming_output");
}

static void
cp_tui_draw_header(const struct cp_audio_config *config)
{
	mvprintw(1, 2, "CarrierPress live monitor");
	mvprintw(2, 2, "rate %0.0f Hz  channels %zu  block %zu  "
	    "input_device %d  output_device %d",
	    config->sample_rate, config->channels, config->block_size,
	    config->input_device, config->output_device);
}

static void
cp_tui_draw_multiband(int row, const struct cp_monitor_snapshot *snapshot)
{
	size_t band;

	mvprintw(row, 2, "Multiband bands %zu", snapshot->band_count);
	for (band = 0; band < snapshot->band_count &&
	    band < CP_MONITOR_MAX_BANDS; band++) {
		mvprintw(row + 1 + (int)band, 2,
		    "Band %zu RMS %0.3f  GR %0.2f dB", band + 1,
		    cp_monitor_level_to_sample(snapshot->band_rms[band]),
		    cp_monitor_centibel_to_db(snapshot->band_gr_db_centibel[band]));
	}
}

static int
cp_tui_level_columns(cp_sample_t value, cp_sample_t full_scale)
{
	cp_sample_t ratio;

	if (!isfinite(value) || value <= 0.0f || full_scale <= 0.0f)
		return 0;

	ratio = value / full_scale;
	if (ratio >= 1.0f)
		return CP_TUI_BAR_WIDTH;
	if (ratio <= 0.0f)
		return 0;

	return (int)lrintf(ratio * (cp_sample_t)CP_TUI_BAR_WIDTH);
}

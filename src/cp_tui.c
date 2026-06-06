/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_tui.c */

#include <sys/types.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <curses.h>

#include "cp_agc.h"
#include "cp_auto_eq.h"
#include "cp_bass_eq.h"
#include "cp_declipper.h"
#include "cp_multiband.h"
#include "cp_restoration.h"
#include "cp_ssb.h"
#include "cp_tui.h"

#define CP_TUI_BAR_WIDTH	40
#define CP_TUI_QUIT_KEY		'q'

static void	cp_tui_draw_bar(int, int, const char *, cp_sample_t,
		    cp_sample_t);
static void	cp_tui_draw_am(int, const struct cp_monitor_snapshot *);
static void	cp_tui_draw_auto_eq(int,
		    const struct cp_monitor_snapshot *);
static void	cp_tui_draw_bass_eq(int,
		    const struct cp_monitor_snapshot *);
static void	cp_tui_draw_dehummer(int,
		    const struct cp_monitor_snapshot *);
static void	cp_tui_draw_declipper(int,
		    const struct cp_monitor_snapshot *);
static void	cp_tui_draw_flags(int, unsigned int);
static void	cp_tui_draw_header(const struct cp_tui_view *);
static void	cp_tui_draw_multiband(int, const char *, unsigned int,
		    int, size_t, const unsigned int *, const int *);
static void	cp_tui_draw_natural(int,
		    const struct cp_monitor_snapshot *);
static void	cp_tui_draw_restoration(int,
		    const struct cp_monitor_snapshot *);
static void	cp_tui_draw_ssb(int, const struct cp_monitor_snapshot *);
static void	cp_tui_draw_keys(const struct cp_tui_view *,
		    enum cp_control_bank);
static enum cp_control_bank
		cp_tui_initial_bank(const struct cp_tui_view *);
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
	cp_sample_t agc_gain;
	const struct cp_monitor_snapshot *snapshot;
	int key;

	if (tui == NULL || view == NULL || view->config == NULL ||
	    view->snapshot == NULL ||
	    command == NULL || !tui->active)
		return 1;
	cp_control_command_clear(command);
	snapshot = view->snapshot;
	if (!tui->control_bank_set) {
		tui->control_bank = cp_tui_initial_bank(view);
		tui->control_bank_set = 1;
	}

	erase();
	cp_tui_draw_header(view);
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

	cp_tui_draw_dehummer(11, snapshot);
	cp_tui_draw_natural(12, snapshot);
	cp_tui_draw_multiband(13, "MB1", snapshot->multiband_enabled,
	    snapshot->multiband_preset, snapshot->band_count,
	    snapshot->band_rms, snapshot->band_gr_db_centibel);
	cp_tui_draw_multiband(14, "MB2", snapshot->multiband2_enabled,
	    snapshot->multiband2_preset, snapshot->band2_count,
	    snapshot->band2_rms, snapshot->band2_gr_db_centibel);
	cp_tui_draw_bass_eq(15, snapshot);

	cp_tui_draw_am(16, snapshot);
	cp_tui_draw_ssb(17, snapshot);
	cp_tui_draw_auto_eq(18, snapshot);
	cp_tui_draw_restoration(19, snapshot);
	cp_tui_draw_declipper(20, snapshot);
	cp_tui_draw_flags(21, snapshot->stream_flags);
	cp_tui_draw_keys(view, tui->control_bank);
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

static void
cp_tui_draw_am(int row, const struct cp_monitor_snapshot *snapshot)
{
	const char *preset_name;

	preset_name = cp_am_preset_string(
	    (enum cp_am_preset)snapshot->am_preset);
	if (preset_name == NULL)
		preset_name = "unknown";

	mvprintw(row, 2, "AM %s preset %s HP %u Hz LP %u Hz pos %0.2f "
	    "neg %0.2f asym %s %0.2f",
	    snapshot->am_enabled ? "on" : "off",
	    preset_name,
	    snapshot->am_highpass_hz,
	    snapshot->am_lowpass_hz,
	    cp_monitor_level_to_sample(snapshot->am_positive_peak),
	    cp_monitor_level_to_sample(snapshot->am_negative_peak),
	    snapshot->am_asymmetry_enabled ? "on" : "off",
	    cp_monitor_level_to_sample(snapshot->am_asymmetry_ratio));
}

static void
cp_tui_draw_auto_eq(int row, const struct cp_monitor_snapshot *snapshot)
{
	if (!snapshot->auto_eq_enabled) {
		mvprintw(row, 2, "Auto EQ analysis off");
		return;
	}

	mvprintw(row, 2, "Auto EQ %s rms %0.4f tilt %0.2f dB low %0.3f "
	    "presence %0.3f high %0.3f finite %s",
	    cp_auto_eq_source_hint_string(
	    (enum cp_auto_eq_source_hint)snapshot->auto_eq_source_hint),
	    cp_monitor_level_to_sample(snapshot->auto_eq_total_rms),
	    cp_monitor_centibel_to_db(
	    snapshot->auto_eq_spectral_tilt_db_centibel),
	    cp_monitor_level_to_sample(snapshot->auto_eq_low_weight),
	    cp_monitor_level_to_sample(snapshot->auto_eq_presence_weight),
	    cp_monitor_level_to_sample(snapshot->auto_eq_high_weight),
	    snapshot->auto_eq_finite ? "yes" : "no");
}

static void
cp_tui_draw_bass_eq(int row, const struct cp_monitor_snapshot *snapshot)
{
	const char *preset_name;

	preset_name = cp_bass_eq_preset_string(
	    (enum cp_bass_eq_preset)snapshot->bass_eq_preset);
	if (preset_name == NULL)
		preset_name = "unknown";

	mvprintw(row, 2, "Bass EQ %s preset %s bass %u Hz %0.2f dB "
	    "presence %u Hz %0.2f dB",
	    snapshot->bass_eq_enabled ? "on" : "off",
	    preset_name,
	    snapshot->bass_eq_low_hz,
	    cp_monitor_centibel_to_db(
	    snapshot->bass_eq_low_gain_db_centibel),
	    snapshot->bass_eq_high_hz,
	    cp_monitor_centibel_to_db(
	    snapshot->bass_eq_high_gain_db_centibel));
}

static void
cp_tui_draw_ssb(int row, const struct cp_monitor_snapshot *snapshot)
{
	const char *preset_name;

	preset_name = cp_ssb_preset_string(
	    (enum cp_ssb_preset)snapshot->ssb_preset);
	if (preset_name == NULL)
		preset_name = "unknown";

	mvprintw(row, 2, "SSB %s preset %s HP %u Hz LP %u Hz peak %0.2f "
	    "phase %s",
	    snapshot->ssb_enabled ? "on" : "off",
	    preset_name,
	    snapshot->ssb_highpass_hz,
	    snapshot->ssb_lowpass_hz,
	    cp_monitor_level_to_sample(snapshot->ssb_peak_limit),
	    snapshot->ssb_phase_rotator_enabled ? "on" : "off");
}

static void
cp_tui_draw_restoration(int row, const struct cp_monitor_snapshot *snapshot)
{
	if (!snapshot->restoration_enabled) {
		mvprintw(row, 2, "Analysis off");
		return;
	}

	mvprintw(row, 2, "Analysis %s flags 0x%08x clip %0.4f HF %0.4f "
	    "clip_conf %0.3f lowceil %0.3f transient %0.3f "
	    "loss_conf %0.3f peak %0.3f crest %0.3f flat %u repeat %u",
	    cp_restoration_source_profile_string(
	    (enum cp_restoration_source_profile)
	    snapshot->restoration_source_profile),
	    snapshot->restoration_reason_flags,
	    cp_monitor_level_to_sample(snapshot->restoration_clipped_ratio),
	    cp_monitor_level_to_sample(snapshot->restoration_hf_ratio),
	    cp_monitor_level_to_sample(
	    snapshot->restoration_clipping_confidence),
	    cp_monitor_level_to_sample(
	    snapshot->restoration_low_ceiling_confidence),
	    cp_monitor_level_to_sample(
	    snapshot->restoration_transient_confidence),
	    cp_monitor_level_to_sample(snapshot->restoration_lossy_confidence),
	    cp_monitor_level_to_sample(snapshot->restoration_observed_peak),
	    cp_monitor_level_to_sample(snapshot->restoration_crest_factor),
	    snapshot->restoration_flat_runs,
	    snapshot->restoration_peak_repeats);
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
cp_tui_draw_dehummer(int row, const struct cp_monitor_snapshot *snapshot)
{
	mvprintw(row, 2, "Dehummer %s base %u Hz harmonics %u",
	    snapshot->dehummer_enabled ? "on" : "off",
	    snapshot->dehummer_base_hz,
	    snapshot->dehummer_harmonic_count);
}

static void
cp_tui_draw_declipper(int row, const struct cp_monitor_snapshot *snapshot)
{
	mvprintw(row, 2, "Declipper %s repaired %u runs %u delta %0.4f "
	    "bypass %s finite %s",
	    snapshot->declipper_enabled ? "on" : "off",
	    snapshot->declipper_repaired_samples,
	    snapshot->declipper_repaired_runs,
	    cp_monitor_level_to_sample(snapshot->declipper_max_delta),
	    cp_declipper_bypass_reason_string(
	    (enum cp_declipper_bypass_reason)
	    snapshot->declipper_bypass_reason),
	    snapshot->declipper_finite ? "yes" : "no");
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
cp_tui_draw_header(const struct cp_tui_view *view)
{
	const struct cp_audio_config *config;

	config = view->config;
	if (view->mode == CP_TUI_MODE_PLAYOUT) {
		mvprintw(1, 2, "CarrierPress playout monitor");
		if (view->playlist_count > 0) {
			mvprintw(2, 2, "track %zu/%zu  rate %0.0f Hz  "
			    "channels %zu  block %zu  output_device %d",
			    view->playlist_index + 1, view->playlist_count,
			    config->sample_rate, config->channels,
			    config->block_size,
			    view->output_device);
		} else {
			mvprintw(2, 2, "single file  rate %0.0f Hz  "
			    "channels %zu  block %zu  output_device %d",
			    config->sample_rate, config->channels,
			    config->block_size,
			    view->output_device);
		}
		mvprintw(3, 2, "file %s", view->path == NULL ? "" :
		    view->path);
		return;
	}

	mvprintw(1, 2, "CarrierPress live monitor");
	mvprintw(2, 2, "rate %0.0f Hz  channels %zu  block %zu  "
	    "input_device %d  output_device %d",
	    config->sample_rate, config->channels, config->block_size,
	    config->input_device, config->output_device);
}

static void
cp_tui_draw_multiband(int row, const char *label, unsigned int enabled,
	int preset, size_t band_count, const unsigned int *band_rms,
	const int *band_gr_db_centibel)
{
	const char *preset_name;
	size_t band;

	preset_name = cp_multiband_preset_string(
	    (enum cp_multiband_preset)preset);
	if (preset_name == NULL)
		preset_name = "unknown";

	mvprintw(row, 2, "%s %s bands %zu preset %s", label,
	    enabled ? "on" : "off", band_count, preset_name);
	for (band = 0; band < band_count && band < CP_MONITOR_MAX_BANDS;
	    band++) {
		printw("  B%zu %0.3f/%0.2f", band + 1,
		    cp_monitor_level_to_sample(band_rms[band]),
		    cp_monitor_centibel_to_db(band_gr_db_centibel[band]));
	}
}

static void
cp_tui_draw_natural(int row, const struct cp_monitor_snapshot *snapshot)
{
	mvprintw(row, 2, "Natural %s GR %0.2f dB  Low boost %s gain %0.3f "
	    "%0.2f dB %s",
	    snapshot->natural_dynamics_enabled ? "on" : "off",
	    cp_monitor_centibel_to_db(
	    snapshot->natural_dynamics_gr_db_centibel),
	    snapshot->low_level_boost_enabled ? "on" : "off",
	    cp_monitor_level_to_sample(snapshot->low_level_boost_gain),
	    cp_monitor_centibel_to_db(
	    snapshot->low_level_boost_gain_db_centibel),
	    cp_agc_state_string((enum cp_agc_gate_state)
	    snapshot->low_level_boost_state));
}

static void
cp_tui_draw_keys(const struct cp_tui_view *view, enum cp_control_bank bank)
{
	const char *preset_keys;

	if (bank == CP_CONTROL_BANK_SSB)
		preset_keys = "0 SSB off  1 speech  2 narrow  3 wide  "
		    "4 gentle";
	else
		preset_keys = "0 AM off  1 safe  2 shortwave  3 wide  "
		    "4 voice";

	if (view->mode == CP_TUI_MODE_PLAYOUT && view->next_enabled) {
		mvprintw(24, 2,
		    "Keys: a AM  s SSB  d hum  m mb1  b mb2  %s  n next  q stop",
		    preset_keys);
		return;
	}
	mvprintw(24, 2, "Keys: a AM  s SSB  d hum  m mb1  b mb2  %s  q stop",
	    preset_keys);
}

static enum cp_control_bank
cp_tui_initial_bank(const struct cp_tui_view *view)
{
	if (view->snapshot->ssb_enabled || view->config->ssb_config.enabled)
		return CP_CONTROL_BANK_SSB;

	return CP_CONTROL_BANK_AM;
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

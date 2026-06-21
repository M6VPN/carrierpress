/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_gui.h */

#ifndef CP_GUI_H
#define CP_GUI_H

#include <sys/types.h>

#include "cp_gui_format.h"
#ifdef CP_WITH_FFTW
#include "cp_spectrum.h"
#endif
#include "cp_waveform.h"

struct cp_gui {
	int active;
	int should_stop;
	int control_bank_set;
	int pending_command_set;
	int pending_workflow_set;
	enum cp_control_bank control_bank;
	struct cp_control_command pending_command;
	struct cp_gui_workflow_request pending_workflow;
	void *window;
	void *renderer;
};

struct cp_gui_view {
	enum cp_gui_mode mode;
	const struct cp_audio_config *config;
	const struct cp_monitor_snapshot *snapshot;
	const struct cp_cat_snapshot *cat_snapshot;
	const struct cp_operator_state *operator_state;
	const struct cp_gui_workflow_request *workflow_request;
	const struct cp_waveform_snapshot *waveform;
#ifdef CP_WITH_FFTW
	const struct cp_spectrum_snapshot *spectrum;
#endif
	const char *cue_wav_path;
	const char *cue_playlist_path;
	const char *path;
	size_t playlist_index;
	size_t playlist_count;
	int output_device;
};

void	cp_gui_close(struct cp_gui *);
void	cp_gui_delay_ms(unsigned int);
int	cp_gui_init(struct cp_gui *);
int	cp_gui_save_bmp(struct cp_gui *, const char *);
int	cp_gui_should_stop(const struct cp_gui *);
int	cp_gui_take_control_command(struct cp_gui *,
	    struct cp_control_command *);
int	cp_gui_take_workflow_request(struct cp_gui *,
	    struct cp_gui_workflow_request *);
int	cp_gui_update(struct cp_gui *, const struct cp_gui_view *);

#endif

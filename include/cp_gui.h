/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_gui.h */

#ifndef CP_GUI_H
#define CP_GUI_H

#include <sys/types.h>

#include "cp_gui_format.h"
#include "cp_waveform.h"

struct cp_gui {
	int active;
	int should_stop;
	void *window;
	void *renderer;
};

struct cp_gui_view {
	enum cp_gui_mode mode;
	const struct cp_audio_config *config;
	const struct cp_monitor_snapshot *snapshot;
	const struct cp_cat_snapshot *cat_snapshot;
	const struct cp_waveform_snapshot *waveform;
	const char *path;
	size_t playlist_index;
	size_t playlist_count;
	int output_device;
};

void	cp_gui_close(struct cp_gui *);
void	cp_gui_delay_ms(unsigned int);
int	cp_gui_init(struct cp_gui *);
int	cp_gui_should_stop(const struct cp_gui *);
int	cp_gui_update(struct cp_gui *, const struct cp_gui_view *);

#endif

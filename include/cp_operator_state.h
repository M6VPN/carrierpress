/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_operator_state.h */

#ifndef CP_OPERATOR_STATE_H
#define CP_OPERATOR_STATE_H

#include <sys/types.h>

#include "cp_types.h"

struct cp_operator_state {
	const char *config_path;
	const char *profile_path;
	const char *profile_name;
	const char *report_path;
	const char *batch_path;
	const char *cue_path;
	size_t playlist_index;
	size_t playlist_count;
	int report_enabled;
	int batch_enabled;
};

void	cp_operator_state_clear(struct cp_operator_state *);
int	cp_operator_state_format_cue(const struct cp_operator_state *,
	    char *, size_t);
int	cp_operator_state_format_report(const struct cp_operator_state *,
	    char *, size_t);
int	cp_operator_state_format_sources(const struct cp_operator_state *,
	    char *, size_t);
int	cp_operator_state_format_summary(const struct cp_operator_state *,
	    char *, size_t);

#endif

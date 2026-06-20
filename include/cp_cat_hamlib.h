/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_cat_hamlib.h */

#ifndef CP_CAT_HAMLIB_H
#define CP_CAT_HAMLIB_H

#include <sys/types.h>

#include <stdint.h>

#include "cp_cat.h"

int	cp_cat_hamlib_mode_to_text(uint64_t, char *, size_t);
int	cp_cat_hamlib_ptt_to_state(int, enum cp_cat_ptt_state *);
int	cp_cat_hamlib_snapshot_update(const struct cp_cat_config *,
	    struct cp_cat_snapshot *);

#endif

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_cat_flrig.h */

#ifndef CP_CAT_FLRIG_H
#define CP_CAT_FLRIG_H

#include <sys/types.h>

#include <stdint.h>

#include "cp_cat.h"

#define CP_CAT_FLRIG_RESPONSE_BYTES	8192

int	cp_cat_flrig_parse_frequency(const char *, uint64_t *);
int	cp_cat_flrig_parse_mode(const char *, char *, size_t);
int	cp_cat_flrig_parse_ptt(const char *, enum cp_cat_ptt_state *);
int	cp_cat_flrig_snapshot_update(const struct cp_cat_config *,
	    struct cp_cat_snapshot *);

#endif

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_batch_wav.h */

#ifndef CP_BATCH_WAV_H
#define CP_BATCH_WAV_H

#include <sys/types.h>

#include <stdio.h>

#include "cp_batch.h"
#include "cp_block.h"

struct cp_batch_wav_result {
	size_t processed;
	size_t failed;
	int last_status;
};

void	cp_batch_wav_result_init(struct cp_batch_wav_result *);
int	cp_batch_wav_process_plan(const struct cp_batch_plan *,
	    const struct cp_block_config *, size_t,
	    struct cp_batch_wav_result *, FILE *);

#endif

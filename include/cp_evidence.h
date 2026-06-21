/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_evidence.h */

#ifndef CP_EVIDENCE_H
#define CP_EVIDENCE_H

#include <sys/types.h>

#include "cp_types.h"

#define CP_EVIDENCE_PATH_SIZE	512

struct cp_evidence_metadata {
	const char *version;
	const char *screenshot_path;
	const char *mode;
	const char *config_path;
	const char *profile_path;
	const char *profile_name;
	const char *report_path;
};

int	cp_evidence_metadata_path(const char *, char *, size_t);
int	cp_evidence_write_screenshot_metadata(const char *,
	    const struct cp_evidence_metadata *);

#endif

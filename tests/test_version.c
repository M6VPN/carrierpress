/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_version.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_version.h"

int
main(void)
{
	if (CP_VERSION_MAJOR != 0 || CP_VERSION_MINOR != 4 ||
	    CP_VERSION_PATCH != 1) {
		printf("test_version: numeric version mismatch\n");
		return 1;
	}
	if (strcmp(CP_VERSION_STRING, "0.4.1") != 0) {
		printf("test_version: string version mismatch\n");
		return 1;
	}

	return 0;
}

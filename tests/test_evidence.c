/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_evidence.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_evidence.h"

#define TEST_EVIDENCE_PATH	"build/tests/gui-demo.bmp"
#define TEST_EVIDENCE_META	"build/tests/gui-demo.bmp.txt"
#define TEST_EVIDENCE_BUFSIZ	2048

static int	read_file(const char *, char *, size_t);
static int	test_metadata_path(void);
static int	test_write_metadata(void);

int
main(void)
{
	if (!test_metadata_path())
		return 1;
	if (!test_write_metadata())
		return 1;

	return 0;
}

static int
read_file(const char *path, char *buffer, size_t buffer_size)
{
	FILE *file;
	size_t count;

	if (path == NULL || buffer == NULL || buffer_size == 0)
		return 0;

	file = fopen(path, "r");
	if (file == NULL)
		return 0;
	count = fread(buffer, 1, buffer_size - 1, file);
	buffer[count] = '\0';
	if (ferror(file)) {
		fclose(file);
		return 0;
	}
	fclose(file);

	return 1;
}

static int
test_metadata_path(void)
{
	char buffer[CP_EVIDENCE_PATH_SIZE];

	if (cp_evidence_metadata_path(TEST_EVIDENCE_PATH, buffer,
	    sizeof(buffer)) != CP_OK ||
	    strcmp(buffer, TEST_EVIDENCE_META) != 0) {
		printf("test_evidence: metadata path failed: %s\n", buffer);
		return 0;
	}
	if (cp_evidence_metadata_path(NULL, buffer, sizeof(buffer)) !=
	    CP_ERR_NULL) {
		printf("test_evidence: null path failed\n");
		return 0;
	}

	return 1;
}

static int
test_write_metadata(void)
{
	char buffer[TEST_EVIDENCE_BUFSIZ];
	struct cp_evidence_metadata metadata;

	(void)memset(&metadata, 0, sizeof(metadata));
	metadata.version = "0.2.0";
	metadata.mode = "demo";
	metadata.config_path = "configs/default.conf";
	metadata.profile_path = "profiles/am-safe.profile";
	metadata.profile_name = "AM Safe";
	metadata.report_path = "build/report.json";

	if (cp_evidence_write_screenshot_metadata(TEST_EVIDENCE_PATH,
	    &metadata) != CP_OK) {
		printf("test_evidence: metadata write failed\n");
		return 0;
	}
	if (!read_file(TEST_EVIDENCE_META, buffer, sizeof(buffer))) {
		printf("test_evidence: metadata read failed\n");
		return 0;
	}
	if (strstr(buffer, "carrierpress_evidence=screenshot") == NULL ||
	    strstr(buffer, "version=0.2.0") == NULL ||
	    strstr(buffer, "config_path=configs/default.conf") == NULL ||
	    strstr(buffer,
	    "profile_path=profiles/am-safe.profile") == NULL ||
	    strstr(buffer, "profile_name=AM Safe") == NULL ||
	    strstr(buffer, "report_path=build/report.json") == NULL) {
		printf("test_evidence: metadata content failed\n");
		return 0;
	}
	if (strstr(buffer, "compliance") != NULL ||
	    strstr(buffer, "proof") != NULL) {
		printf("test_evidence: metadata wording failed\n");
		return 0;
	}

	return 1;
}

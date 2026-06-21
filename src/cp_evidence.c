/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_evidence.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_evidence.h"

static const char	*cp_evidence_text(const char *);

int
cp_evidence_metadata_path(const char *screenshot_path, char *buffer,
	size_t buffer_size)
{
	int written;

	if (screenshot_path == NULL || screenshot_path[0] == '\0' ||
	    buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;

	written = snprintf(buffer, buffer_size, "%s.txt", screenshot_path);
	if (written < 0 || (size_t)written >= buffer_size) {
		buffer[buffer_size - 1] = '\0';
		return CP_ERR_RANGE;
	}

	return CP_OK;
}

int
cp_evidence_write_screenshot_metadata(const char *screenshot_path,
	const struct cp_evidence_metadata *metadata)
{
	char metadata_path[CP_EVIDENCE_PATH_SIZE];
	FILE *file;

	if (screenshot_path == NULL || metadata == NULL)
		return CP_ERR_NULL;
	if (cp_evidence_metadata_path(screenshot_path, metadata_path,
	    sizeof(metadata_path)) != CP_OK)
		return CP_ERR_RANGE;

	file = fopen(metadata_path, "w");
	if (file == NULL)
		return CP_ERR_RANGE;

	fprintf(file, "carrierpress_evidence=screenshot\n");
	fprintf(file, "version=%s\n", cp_evidence_text(metadata->version));
	fprintf(file, "screenshot_path=%s\n",
	    cp_evidence_text(screenshot_path));
	fprintf(file, "mode=%s\n", cp_evidence_text(metadata->mode));
	fprintf(file, "config_path=%s\n",
	    cp_evidence_text(metadata->config_path));
	fprintf(file, "profile_path=%s\n",
	    cp_evidence_text(metadata->profile_path));
	fprintf(file, "profile_name=%s\n",
	    cp_evidence_text(metadata->profile_name));
	fprintf(file, "report_path=%s\n",
	    cp_evidence_text(metadata->report_path));
	fprintf(file, "note=engineering operator evidence only; "
	    "not legal or transmitter validation\n");
	if (ferror(file)) {
		fclose(file);
		return CP_ERR_RANGE;
	}
	if (fclose(file) != 0)
		return CP_ERR_RANGE;

	return CP_OK;
}

static const char *
cp_evidence_text(const char *text)
{
	if (text == NULL || text[0] == '\0')
		return "-";

	return text;
}

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_playlist_check.c */

#include <sys/types.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "cp_playlist_check.h"

static int	cp_playlist_check_read_line(FILE *, char *, size_t, int *);
static char	*cp_playlist_check_trim(char *);

void
cp_playlist_check_result_init(struct cp_playlist_check_result *result)
{
	if (result == NULL)
		return;

	memset(result, 0, sizeof(*result));
}

int
cp_playlist_check_file(const char *path,
	struct cp_playlist_check_result *result, FILE *report)
{
	FILE *file;
	char line[CP_PLAYLIST_CHECK_MAX_LINE];
	char *entry;
	size_t line_number;
	int truncated;

	if (path == NULL || result == NULL)
		return CP_PLAYLIST_CHECK_ERR_NULL;

	cp_playlist_check_result_init(result);
	if (report != NULL)
		fprintf(report, "playlist: %s\n", path);

	file = fopen(path, "r");
	if (file == NULL) {
		result->errors++;
		if (report != NULL)
			fprintf(report, "error: could not open playlist\n");
		return CP_PLAYLIST_CHECK_ERR_OPEN;
	}

	line_number = 0;
	while (cp_playlist_check_read_line(file, line, sizeof(line),
	    &truncated)) {
		line_number++;
		result->total_lines++;
		if (truncated) {
			result->too_long_lines++;
			result->errors++;
			if (report != NULL)
				fprintf(report, "line %zu: error: playlist "
				    "line is too long\n", line_number);
			continue;
		}

		entry = cp_playlist_check_trim(line);
		if (entry[0] == '\0' || entry[0] == '#') {
			result->skipped_lines++;
			if (report != NULL)
				fprintf(report, "line %zu: skipped: "
				    "blank/comment\n", line_number);
			continue;
		}
		if (!cp_playlist_check_path_is_wav(entry)) {
			result->unsupported_entries++;
			result->errors++;
			if (report != NULL)
				fprintf(report, "line %zu: error: "
				    "unsupported format: convert to WAV "
				    "first: %s\n", line_number, entry);
			continue;
		}

		result->playable_entries++;
		if (report != NULL)
			fprintf(report, "line %zu: ok: %s\n",
			    line_number, entry);
	}

	fclose(file);
	if (report != NULL)
		fprintf(report, "summary: %zu playable, %zu skipped, "
		    "%zu error\n", result->playable_entries,
		    result->skipped_lines, result->errors);

	if (result->errors > 0)
		return CP_PLAYLIST_CHECK_ERR_BAD;

	return CP_PLAYLIST_CHECK_OK;
}

int
cp_playlist_check_path_is_wav(const char *path)
{
	size_t length;

	if (path == NULL)
		return 0;

	length = strlen(path);
	if (length < 4)
		return 0;

	path += length - 4;
	if (tolower((unsigned char)path[0]) != '.')
		return 0;
	if (tolower((unsigned char)path[1]) != 'w')
		return 0;
	if (tolower((unsigned char)path[2]) != 'a')
		return 0;
	if (tolower((unsigned char)path[3]) != 'v')
		return 0;

	return 1;
}

const char *
cp_playlist_check_status_string(int status)
{
	switch (status) {
	case CP_PLAYLIST_CHECK_OK:
		return "ok";
	case CP_PLAYLIST_CHECK_ERR_NULL:
		return "missing playlist path or result";
	case CP_PLAYLIST_CHECK_ERR_OPEN:
		return "could not open playlist";
	case CP_PLAYLIST_CHECK_ERR_BAD:
		return "playlist has errors";
	default:
		return "unknown playlist check error";
	}
}

static int
cp_playlist_check_read_line(FILE *file, char *line, size_t line_size,
	int *truncated)
{
	size_t length;
	int ch;

	if (truncated != NULL)
		*truncated = 0;
	if (file == NULL || line == NULL || line_size == 0)
		return 0;
	if (fgets(line, (int)line_size, file) == NULL)
		return 0;

	length = strlen(line);
	if (length > 0 && line[length - 1] == '\n')
		return 1;
	if (feof(file))
		return 1;

	if (truncated != NULL)
		*truncated = 1;
	while ((ch = fgetc(file)) != EOF && ch != '\n')
		;

	return 1;
}

static char *
cp_playlist_check_trim(char *line)
{
	char *end;

	while (*line != '\0' && isspace((unsigned char)*line))
		line++;

	end = line + strlen(line);
	while (end > line && isspace((unsigned char)end[-1]))
		*--end = '\0';

	return line;
}

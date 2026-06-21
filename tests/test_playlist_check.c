/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_playlist_check.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_playlist_check.h"

static int	write_long_playlist(const char *);
static int	write_text_file(const char *, const char *);
static int	test_missing_file(void);
static int	test_path_filter(void);
static int	test_too_long_line(void);
static int	test_valid_playlist(void);

int
main(void)
{
	if (!test_path_filter())
		return 1;
	if (!test_valid_playlist())
		return 1;
	if (!test_too_long_line())
		return 1;
	if (!test_missing_file())
		return 1;

	return 0;
}

static int
write_long_playlist(const char *path)
{
	FILE *file;
	size_t index;

	file = fopen(path, "w");
	if (file == NULL)
		return 0;
	for (index = 0; index < CP_PLAYLIST_CHECK_MAX_LINE + 8; index++) {
		if (fputc('a', file) == EOF) {
			fclose(file);
			return 0;
		}
	}
	if (fputc('\n', file) == EOF) {
		fclose(file);
		return 0;
	}
	fclose(file);

	return 1;
}

static int
write_text_file(const char *path, const char *text)
{
	FILE *file;

	file = fopen(path, "w");
	if (file == NULL)
		return 0;
	if (fputs(text, file) == EOF) {
		fclose(file);
		return 0;
	}
	fclose(file);

	return 1;
}

static int
test_missing_file(void)
{
	struct cp_playlist_check_result result;

	if (cp_playlist_check_file("build/tests/missing-playlist.txt",
	    &result, NULL) != CP_PLAYLIST_CHECK_ERR_OPEN ||
	    result.errors != 1) {
		printf("test_playlist_check: missing file accepted\n");
		return 0;
	}

	return 1;
}

static int
test_path_filter(void)
{
	if (!cp_playlist_check_path_is_wav("track.wav"))
		return 0;
	if (!cp_playlist_check_path_is_wav("TRACK.WAV"))
		return 0;
	if (cp_playlist_check_path_is_wav("track.mp3"))
		return 0;
	if (cp_playlist_check_path_is_wav("wav"))
		return 0;
	if (strcmp(cp_playlist_check_status_string(CP_PLAYLIST_CHECK_ERR_BAD),
	    "playlist has errors") != 0)
		return 0;

	return 1;
}

static int
test_too_long_line(void)
{
	struct cp_playlist_check_result result;
	const char *path;

	path = "build/tests/playlist_too_long.txt";
	if (!write_long_playlist(path)) {
		printf("test_playlist_check: long fixture write failed\n");
		return 0;
	}
	if (cp_playlist_check_file(path, &result, NULL) !=
	    CP_PLAYLIST_CHECK_ERR_BAD ||
	    result.total_lines != 1 ||
	    result.too_long_lines != 1 ||
	    result.errors != 1) {
		printf("test_playlist_check: long line not reported\n");
		return 0;
	}

	return 1;
}

static int
test_valid_playlist(void)
{
	struct cp_playlist_check_result result;
	const char *path;

	path = "build/tests/playlist_check.txt";
	if (!write_text_file(path, "\n# comment\nintro.wav\n"
	    "TWO.WAV\nmusic.mp3\nmix.flac\nvoice.ogg\n")) {
		printf("test_playlist_check: fixture write failed\n");
		return 0;
	}
	if (cp_playlist_check_file(path, &result, NULL) !=
	    CP_PLAYLIST_CHECK_ERR_BAD ||
	    result.total_lines != 7 ||
	    result.playable_entries != 2 ||
	    result.skipped_lines != 2 ||
	    result.unsupported_entries != 3 ||
	    result.errors != 3) {
		printf("test_playlist_check: summary mismatch\n");
		return 0;
	}

	return 1;
}

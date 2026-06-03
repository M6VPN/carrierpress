/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_playout.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_playout.h"

static int	write_text_file(const char *, const char *);
static int	test_path_filter(void);
static int	test_playlist_load(void);
static int	test_playlist_rejects_mp3(void);

int
main(void)
{
	if (!test_path_filter())
		return 1;
	if (!test_playlist_load())
		return 1;
	if (!test_playlist_rejects_mp3())
		return 1;

	return 0;
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
test_path_filter(void)
{
	if (!cp_playout_path_is_wav("track.wav"))
		return 0;
	if (!cp_playout_path_is_wav("TRACK.WAV"))
		return 0;
	if (cp_playout_path_is_wav("track.mp3"))
		return 0;
	if (cp_playout_path_is_wav("wav"))
		return 0;

	return 1;
}

static int
test_playlist_load(void)
{
	struct cp_playlist playlist;
	const char *path;
	int status;

	path = "build/tests/playout_good.txt";
	if (!write_text_file(path, "\n# comment\n one.wav \nTWO.WAV\n")) {
		printf("test_playout: fixture write failed\n");
		return 0;
	}

	status = cp_playlist_load(path, &playlist);
	if (status != CP_PLAYOUT_OK) {
		printf("test_playout: load failed: %s\n",
		    cp_playout_status_string(status));
		return 0;
	}
	if (cp_playlist_count(&playlist) != 2) {
		cp_playlist_free(&playlist);
		printf("test_playout: count mismatch\n");
		return 0;
	}
	if (strcmp(cp_playlist_get(&playlist, 0), "one.wav") != 0 ||
	    strcmp(cp_playlist_get(&playlist, 1), "TWO.WAV") != 0) {
		cp_playlist_free(&playlist);
		printf("test_playout: path mismatch\n");
		return 0;
	}

	cp_playlist_free(&playlist);
	return 1;
}

static int
test_playlist_rejects_mp3(void)
{
	struct cp_playlist playlist;
	const char *path;
	int status;

	path = "build/tests/playout_bad.txt";
	if (!write_text_file(path, "good.wav\nbad.mp3\n")) {
		printf("test_playout: fixture write failed\n");
		return 0;
	}

	status = cp_playlist_load(path, &playlist);
	if (status != CP_PLAYOUT_ERR_UNSUPPORTED) {
		printf("test_playout: unsupported file accepted\n");
		return 0;
	}

	return 1;
}

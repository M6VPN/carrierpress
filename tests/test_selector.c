/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_selector.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_audio.h"
#include "cp_selector.h"

static int	test_disabled_items(void);
static int	test_empty_selector(void);
static int	test_forbidden_text(void);
static int	test_format_kinds(void);
static int	test_audio_file_selector(void);
static int	test_long_text_bounds(void);
static int	test_next_prev(void);
static int	test_output_device_selector(void);

int
main(void)
{
	if (!test_empty_selector())
		return 1;
	if (!test_next_prev())
		return 1;
	if (!test_disabled_items())
		return 1;
	if (!test_long_text_bounds())
		return 1;
	if (!test_format_kinds())
		return 1;
	if (!test_audio_file_selector())
		return 1;
	if (!test_output_device_selector())
		return 1;
	if (!test_forbidden_text())
		return 1;

	return 0;
}

static int
test_disabled_items(void)
{
	struct cp_selector selector;

	cp_selector_init(&selector, CP_SELECTOR_OUTPUT_DEVICE);
	if (cp_selector_add(&selector, "disabled", "0", 0) != CP_OK)
		return 0;
	if (cp_selector_add(&selector, "enabled", "1", 1) != CP_OK)
		return 0;
	if (selector.selected != 1)
		return 0;
	if (cp_selector_select(&selector, 0) != CP_ERR_RANGE)
		return 0;
	if (cp_selector_select(&selector, 1) != CP_OK)
		return 0;
	if (cp_selector_next(&selector) != CP_OK || selector.selected != 1)
		return 0;

	return 1;
}

static int
test_empty_selector(void)
{
	struct cp_selector selector;
	char buffer[96];

	cp_selector_init(&selector, CP_SELECTOR_AUDIO_FILE);
	if (selector.kind != CP_SELECTOR_AUDIO_FILE || selector.count != 0 ||
	    selector.selected != 0)
		return 0;
	if (cp_selector_current(&selector) != NULL)
		return 0;
	if (cp_selector_next(&selector) != CP_ERR_RANGE ||
	    cp_selector_prev(&selector) != CP_ERR_RANGE)
		return 0;
	if (cp_selector_format_line(&selector, buffer, sizeof(buffer)) !=
	    CP_OK ||
	    strcmp(buffer,
	    "selector=audio_file selected=0/0 label=- value=-") != 0) {
		printf("test_selector: empty format mismatch: %s\n", buffer);
		return 0;
	}

	return 1;
}

static int
test_output_device_selector(void)
{
	struct cp_audio_device_candidate choices[5];
	struct cp_selector selector;
	char buffer[256];
	char menu[128];
	char long_name[180];
	size_t index;

	memset(choices, 0, sizeof(choices));
	if (cp_selector_load_output_devices(&selector, NULL, 0, 2, 0, 0) !=
	    CP_OK || selector.kind != CP_SELECTOR_OUTPUT_DEVICE ||
	    selector.count != 0)
		return 0;
	if (cp_selector_load_output_devices(NULL, choices, 0, 2, 0, 0) !=
	    CP_ERR_NULL)
		return 0;

	choices[0].index = 0;
	choices[0].name = "input only";
	choices[0].max_input_channels = 2;
	choices[1].index = 1;
	choices[1].name = "Built-in Audio";
	choices[1].max_output_channels = 2;
	choices[1].default_output = 1;
	choices[2].index = 2;
	choices[2].name = "USB Audio";
	choices[2].max_output_channels = 2;
	choices[3].index = 4;
	choices[3].name = "Headphones";
	choices[3].max_output_channels = 2;
	if (cp_selector_load_output_devices(&selector, choices, 4, 2, 1,
	    4) != CP_OK)
		return 0;
	if (selector.count != 3 || selector.selected != 2)
		return 0;
	if (strstr(selector.items[0].label, "default") == NULL ||
	    strstr(selector.items[1].label, "current") == NULL ||
	    strstr(selector.items[2].label, "requested") == NULL)
		return 0;
	if (strcmp(selector.items[1].value, "2") != 0 ||
	    strcmp(selector.items[2].value, "4") != 0)
		return 0;
	if (cp_selector_format_line(&selector, buffer, sizeof(buffer)) !=
	    CP_OK ||
	    strstr(buffer, "selector=output_device") == NULL ||
	    strstr(buffer, "selected=3/3") == NULL ||
	    strstr(buffer, "Headphones") == NULL)
		return 0;
	if (cp_selector_format_menu_item(&selector, 2, menu,
	    sizeof(menu)) != CP_OK ||
	    strstr(menu, "> 4 Headphones") == NULL ||
	    strstr(menu, "requested") == NULL)
		return 0;
	if (cp_selector_format_menu_item(&selector, 8, menu,
	    sizeof(menu)) != CP_ERR_RANGE)
		return 0;

	for (index = 0; index < sizeof(long_name) - 1; index++)
		long_name[index] = 'x';
	long_name[sizeof(long_name) - 1] = '\0';
	choices[4].index = 9;
	choices[4].name = long_name;
	choices[4].max_output_channels = 2;
	if (cp_selector_load_output_devices(&selector, choices + 4, 1, 9,
	    0, 0) != CP_OK ||
	    strlen(selector.items[0].label) >= CP_SELECTOR_LABEL_MAX ||
	    strstr(selector.items[0].label, "...") == NULL)
		return 0;

	return 1;
}

static int
test_audio_file_selector(void)
{
	struct cp_selector selector;
	const char *paths[] = {
		"audio/intro.wav",
		"audio/music.mp3",
		"audio/news-bed.WAV",
		"audio/archive.flac",
		"audio/readme.txt",
		"",
		NULL,
		"audio/intro.wav"
	};
	char buffer[256];
	char menu[128];
	char long_path[320];
	size_t index;

	if (cp_selector_load_audio_files(NULL, paths, 1, NULL, NULL) !=
	    CP_ERR_NULL)
		return 0;
	if (cp_selector_load_audio_files(&selector, NULL, 1, NULL, NULL) !=
	    CP_ERR_NULL)
		return 0;
	if (cp_selector_add_audio_file(&selector, NULL, NULL, NULL, NULL) !=
	    CP_ERR_NULL)
		return 0;
	if (cp_selector_add_audio_file(&selector, NULL, "", NULL, NULL) !=
	    CP_ERR_RANGE)
		return 0;

	if (cp_selector_load_audio_files(&selector, paths, 8,
	    "audio/intro.wav", "audio/news-bed.WAV") != CP_OK)
		return 0;
	if (selector.kind != CP_SELECTOR_AUDIO_FILE || selector.count != 5 ||
	    selector.selected != 2)
		return 0;
	if (!selector.items[0].enabled || selector.items[1].enabled ||
	    !selector.items[2].enabled || selector.items[3].enabled ||
	    selector.items[4].enabled)
		return 0;
	if (strstr(selector.items[0].label, "current") == NULL ||
	    strstr(selector.items[2].label, "requested") == NULL ||
	    strstr(selector.items[1].label, "convert externally") == NULL ||
	    strstr(selector.items[3].label, "convert externally") == NULL ||
	    strstr(selector.items[4].label, "unsupported") == NULL)
		return 0;
	if (strcmp(selector.items[2].value, "audio/news-bed.WAV") != 0)
		return 0;
	if (cp_selector_format_line(&selector, buffer, sizeof(buffer)) !=
	    CP_OK ||
	    strstr(buffer, "selector=audio_file") == NULL ||
	    strstr(buffer, "selected=3/5") == NULL ||
	    strstr(buffer, "news-bed.WAV") == NULL)
		return 0;
	if (cp_selector_format_menu_item(&selector, 1, menu,
	    sizeof(menu)) != CP_OK ||
	    strstr(menu, "music.mp3") == NULL ||
	    strstr(menu, "disabled") == NULL)
		return 0;
	if (cp_selector_select(&selector, 1) != CP_ERR_RANGE)
		return 0;
	if (cp_selector_next(&selector) != CP_OK || selector.selected != 0)
		return 0;
	if (cp_selector_prev(&selector) != CP_OK || selector.selected != 2)
		return 0;

	memset(long_path, 'p', sizeof(long_path) - 5);
	(void)snprintf(long_path + sizeof(long_path) - 5, 5, ".wav");
	if (cp_selector_load_audio_files(&selector,
	    (const char *const *)&paths[0], 0, NULL, NULL) != CP_OK)
		return 0;
	if (cp_selector_add_audio_file(&selector, NULL, long_path, NULL,
	    NULL) != CP_OK)
		return 0;
	if (strlen(selector.items[0].label) >= CP_SELECTOR_LABEL_MAX ||
	    strlen(selector.items[0].value) >= CP_SELECTOR_VALUE_MAX)
		return 0;

	for (index = 0; index < selector.count; index++) {
		if (strstr(selector.items[index].label, "ptt") != NULL ||
		    strstr(selector.items[index].label, "transmit") != NULL ||
		    strstr(selector.items[index].label, "cat_ptt") != NULL ||
		    strstr(selector.items[index].label, "rig_frequency") !=
		    NULL ||
		    strstr(selector.items[index].label, "rig_mode") != NULL ||
		    strstr(selector.items[index].label, "hamlib") != NULL ||
		    strstr(selector.items[index].label, "flrig") != NULL)
			return 0;
	}

	return 1;
}

static int
test_forbidden_text(void)
{
	struct cp_selector selector;
	char buffer[256];

	cp_selector_init(&selector, CP_SELECTOR_PLAYLIST);
	if (cp_selector_add(&selector, "show playlist",
	    "playlists/show.txt", 1) != CP_OK)
		return 0;
	if (cp_selector_format_line(&selector, buffer, sizeof(buffer)) !=
	    CP_OK)
		return 0;
	if (strstr(buffer, "ptt") != NULL ||
	    strstr(buffer, "transmit") != NULL ||
	    strstr(buffer, "cat_ptt") != NULL ||
	    strstr(buffer, "rig_frequency") != NULL ||
	    strstr(buffer, "rig_mode") != NULL ||
	    strstr(buffer, "hamlib") != NULL ||
	    strstr(buffer, "flrig") != NULL) {
		printf("test_selector: forbidden text: %s\n", buffer);
		return 0;
	}

	return 1;
}

static int
test_format_kinds(void)
{
	struct cp_selector selector;
	char buffer[256];

	if (strcmp(cp_selector_kind_string(CP_SELECTOR_OUTPUT_DEVICE),
	    "output_device") != 0)
		return 0;
	if (strcmp(cp_selector_kind_string(CP_SELECTOR_AUDIO_FILE),
	    "audio_file") != 0)
		return 0;
	if (strcmp(cp_selector_kind_string(CP_SELECTOR_PLAYLIST),
	    "playlist") != 0)
		return 0;

	cp_selector_init(&selector, CP_SELECTOR_OUTPUT_DEVICE);
	if (cp_selector_add(&selector, "USB Audio", "2", 1) != CP_OK ||
	    cp_selector_format_line(&selector, buffer, sizeof(buffer)) !=
	    CP_OK ||
	    strstr(buffer, "selector=output_device") == NULL ||
	    strstr(buffer, "label=\"USB Audio\"") == NULL)
		return 0;

	cp_selector_init(&selector, CP_SELECTOR_AUDIO_FILE);
	if (cp_selector_add(&selector, "program.wav",
	    "audio/program.wav", 1) != CP_OK ||
	    cp_selector_format_line(&selector, buffer, sizeof(buffer)) !=
	    CP_OK ||
	    strstr(buffer, "selector=audio_file") == NULL)
		return 0;

	cp_selector_init(&selector, CP_SELECTOR_PLAYLIST);
	if (cp_selector_add(&selector, "show list",
	    "playlists/show.txt", 1) != CP_OK ||
	    cp_selector_format_line(&selector, buffer, sizeof(buffer)) !=
	    CP_OK ||
	    strstr(buffer, "selector=playlist") == NULL)
		return 0;

	return 1;
}

static int
test_long_text_bounds(void)
{
	struct cp_selector selector;
	const struct cp_selector_item *item;
	char buffer[80];
	char long_text[512];
	size_t index;

	for (index = 0; index < sizeof(long_text) - 1; index++)
		long_text[index] = 'a';
	long_text[sizeof(long_text) - 1] = '\0';

	cp_selector_init(&selector, CP_SELECTOR_AUDIO_FILE);
	if (cp_selector_add(&selector, long_text, long_text, 1) != CP_OK)
		return 0;
	item = cp_selector_current(&selector);
	if (item == NULL)
		return 0;
	if (item->label[sizeof(item->label) - 1] != '\0' ||
	    item->value[sizeof(item->value) - 1] != '\0')
		return 0;
	if (strlen(item->label) != CP_SELECTOR_LABEL_MAX - 1 ||
	    strlen(item->value) != CP_SELECTOR_VALUE_MAX - 1)
		return 0;
	if (cp_selector_format_line(&selector, buffer, sizeof(buffer)) !=
	    CP_OK)
		return 0;
	if (buffer[sizeof(buffer) - 1] != '\0')
		return 0;

	return 1;
}

static int
test_next_prev(void)
{
	struct cp_selector selector;

	cp_selector_init(&selector, CP_SELECTOR_OUTPUT_DEVICE);
	if (cp_selector_add(NULL, "x", "x", 1) != CP_ERR_NULL)
		return 0;
	if (cp_selector_add(&selector, "default", "-1", 1) != CP_OK ||
	    cp_selector_add(&selector, "disabled", "0", 0) != CP_OK ||
	    cp_selector_add(&selector, "USB", "1", 1) != CP_OK)
		return 0;
	if (cp_selector_current(&selector) == NULL ||
	    strcmp(cp_selector_current(&selector)->label, "default") != 0)
		return 0;
	if (cp_selector_next(&selector) != CP_OK || selector.selected != 2)
		return 0;
	if (cp_selector_next(&selector) != CP_OK || selector.selected != 0)
		return 0;
	if (cp_selector_prev(&selector) != CP_OK || selector.selected != 2)
		return 0;
	if (cp_selector_select(&selector, 2) != CP_OK ||
	    selector.selected != 2)
		return 0;
	if (cp_selector_select(&selector, 9) != CP_ERR_RANGE)
		return 0;

	return 1;
}

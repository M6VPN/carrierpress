/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/tests/test_selector.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_selector.h"

static int	test_disabled_items(void);
static int	test_empty_selector(void);
static int	test_forbidden_text(void);
static int	test_format_kinds(void);
static int	test_long_text_bounds(void);
static int	test_next_prev(void);

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

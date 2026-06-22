/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_selector.c */

#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "cp_selector.h"

static int	cp_selector_copy_text(char *, size_t, const char *);
static int	cp_selector_has_enabled(const struct cp_selector *);
static int	cp_selector_move(struct cp_selector *, int);
static int	cp_selector_snprintf(char *, size_t, const char *, ...);

int
cp_selector_add(struct cp_selector *selector, const char *label,
	const char *value, int enabled)
{
	struct cp_selector_item *item;
	int had_enabled;

	if (selector == NULL || label == NULL || value == NULL)
		return CP_ERR_NULL;
	if (selector->count >= CP_SELECTOR_ITEMS_MAX)
		return CP_ERR_RANGE;

	had_enabled = cp_selector_has_enabled(selector);
	item = &selector->items[selector->count];
	(void)memset(item, 0, sizeof(*item));
	(void)cp_selector_copy_text(item->label, sizeof(item->label), label);
	(void)cp_selector_copy_text(item->value, sizeof(item->value), value);
	item->enabled = enabled ? 1 : 0;
	selector->count++;
	if (selector->count == 1 || (!had_enabled && item->enabled))
		selector->selected = selector->count - 1;

	return CP_OK;
}

const struct cp_selector_item *
cp_selector_current(const struct cp_selector *selector)
{
	if (selector == NULL || selector->count == 0 ||
	    selector->selected >= selector->count)
		return NULL;

	return &selector->items[selector->selected];
}

int
cp_selector_format_line(const struct cp_selector *selector, char *buffer,
	size_t buffer_size)
{
	const struct cp_selector_item *item;
	size_t selected_number;

	if (buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;
	if (selector == NULL)
		return CP_ERR_NULL;

	item = cp_selector_current(selector);
	if (item == NULL) {
		return cp_selector_snprintf(buffer, buffer_size,
		    "selector=%s selected=0/0 label=- value=-",
		    cp_selector_kind_string(selector->kind));
	}

	selected_number = selector->selected + 1;
	return cp_selector_snprintf(buffer, buffer_size,
	    "selector=%s selected=%zu/%zu label=\"%s\" value=\"%s\"%s",
	    cp_selector_kind_string(selector->kind), selected_number,
	    selector->count, item->label, item->value,
	    item->enabled ? "" : " disabled");
}

void
cp_selector_init(struct cp_selector *selector, enum cp_selector_kind kind)
{
	if (selector == NULL)
		return;

	(void)memset(selector, 0, sizeof(*selector));
	selector->kind = kind;
	selector->selected = 0;
}

const char *
cp_selector_kind_string(enum cp_selector_kind kind)
{
	switch (kind) {
	case CP_SELECTOR_OUTPUT_DEVICE:
		return "output_device";
	case CP_SELECTOR_AUDIO_FILE:
		return "audio_file";
	case CP_SELECTOR_PLAYLIST:
		return "playlist";
	default:
		return "unknown";
	}
}

int
cp_selector_next(struct cp_selector *selector)
{
	return cp_selector_move(selector, 1);
}

int
cp_selector_prev(struct cp_selector *selector)
{
	return cp_selector_move(selector, -1);
}

int
cp_selector_select(struct cp_selector *selector, size_t index)
{
	if (selector == NULL)
		return CP_ERR_NULL;
	if (index >= selector->count || !selector->items[index].enabled)
		return CP_ERR_RANGE;

	selector->selected = index;
	return CP_OK;
}

static int
cp_selector_copy_text(char *dst, size_t dst_size, const char *src)
{
	if (dst == NULL || src == NULL || dst_size == 0)
		return CP_ERR_NULL;

	(void)snprintf(dst, dst_size, "%s", src);
	return CP_OK;
}

static int
cp_selector_has_enabled(const struct cp_selector *selector)
{
	size_t index;

	if (selector == NULL)
		return 0;
	for (index = 0; index < selector->count; index++) {
		if (selector->items[index].enabled)
			return 1;
	}

	return 0;
}

static int
cp_selector_move(struct cp_selector *selector, int direction)
{
	size_t offset;
	size_t index;

	if (selector == NULL)
		return CP_ERR_NULL;
	if (selector->count == 0 || !cp_selector_has_enabled(selector))
		return CP_ERR_RANGE;

	for (offset = 1; offset <= selector->count; offset++) {
		if (direction > 0) {
			index = (selector->selected + offset) %
			    selector->count;
		} else {
			index = (selector->selected + selector->count -
			    (offset % selector->count)) % selector->count;
		}
		if (selector->items[index].enabled) {
			selector->selected = index;
			return CP_OK;
		}
	}

	return CP_ERR_RANGE;
}

static int
cp_selector_snprintf(char *buffer, size_t buffer_size, const char *format, ...)
{
	va_list ap;
	int written;

	if (buffer == NULL || buffer_size == 0 || format == NULL)
		return CP_ERR_NULL;

	va_start(ap, format);
	written = vsnprintf(buffer, buffer_size, format, ap);
	va_end(ap);
	if (written < 0)
		return CP_ERR_BUFFER;
	buffer[buffer_size - 1] = '\0';

	return CP_OK;
}

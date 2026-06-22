/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_selector.c */

#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "cp_audio.h"
#include "cp_selector.h"

static int	cp_selector_copy_text(char *, size_t, const char *);
static int	cp_selector_extension_matches(const char *, const char *);
static int	cp_selector_format_audio_label(const char *, const char *,
		    const char *, const char *, char *, size_t, int *);
static int	cp_selector_format_output_label(
		    const struct cp_audio_device_candidate *, int, int, int,
		    char *, size_t);
static int	cp_selector_format_output_value(
		    const struct cp_audio_device_candidate *, char *, size_t);
static int	cp_selector_format_playlist_label(const char *, const char *,
		    const char *, const char *, char *, size_t, int *);
static int	cp_selector_has_enabled(const struct cp_selector *);
static int	cp_selector_path_is_compressed(const char *);
static int	cp_selector_path_is_playlist(const char *);
static int	cp_selector_path_is_wav(const char *);
static int	cp_selector_marker_append(char *, size_t, const char *);
static int	cp_selector_move(struct cp_selector *, int);
static const char
		*cp_selector_path_label(const char *);
static int	cp_selector_snprintf(char *, size_t, const char *, ...);
static void	cp_selector_truncate_text(char *, size_t, const char *,
		    size_t);
static int	cp_selector_value_exists(const struct cp_selector *,
		    const char *);

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

int
cp_selector_add_audio_file(struct cp_selector *selector, const char *label,
	const char *path, const char *current_path, const char *requested_path)
{
	char item_label[CP_SELECTOR_LABEL_MAX];
	int enabled;
	int status;

	if (selector == NULL || path == NULL)
		return CP_ERR_NULL;
	if (path[0] == '\0')
		return CP_ERR_RANGE;

	status = cp_selector_format_audio_label(label, path, current_path,
	    requested_path, item_label, sizeof(item_label), &enabled);
	if (status != CP_OK)
		return status;

	return cp_selector_add(selector, item_label, path, enabled);
}

int
cp_selector_add_playlist(struct cp_selector *selector, const char *label,
	const char *path, const char *current_path, const char *requested_path)
{
	char item_label[CP_SELECTOR_LABEL_MAX];
	int enabled;
	int status;

	if (selector == NULL || path == NULL)
		return CP_ERR_NULL;
	if (path[0] == '\0')
		return CP_ERR_RANGE;

	status = cp_selector_format_playlist_label(label, path, current_path,
	    requested_path, item_label, sizeof(item_label), &enabled);
	if (status != CP_OK)
		return status;

	return cp_selector_add(selector, item_label, path, enabled);
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

int
cp_selector_format_menu_item(const struct cp_selector *selector, size_t index,
	char *buffer, size_t buffer_size)
{
	const struct cp_selector_item *item;

	if (selector == NULL || buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;
	if (index >= selector->count)
		return CP_ERR_RANGE;

	item = &selector->items[index];
	return cp_selector_snprintf(buffer, buffer_size, "%c %s %s%s",
	    index == selector->selected ? '>' : ' ', item->value,
	    item->label, item->enabled ? "" : " disabled");
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
cp_selector_load_output_devices(struct cp_selector *selector,
	const struct cp_audio_device_candidate *candidates, size_t count,
	int current_device, int requested_set, int requested_device)
{
	char label[CP_SELECTOR_LABEL_MAX];
	char value[CP_SELECTOR_VALUE_MAX];
	size_t default_index;
	size_t index;
	size_t loaded_index;
	size_t selected_current;
	size_t selected_requested;
	int status;

	if (selector == NULL)
		return CP_ERR_NULL;
	if (candidates == NULL && count > 0)
		return CP_ERR_NULL;

	cp_selector_init(selector, CP_SELECTOR_OUTPUT_DEVICE);
	default_index = (size_t)-1;
	selected_current = (size_t)-1;
	selected_requested = (size_t)-1;

	for (index = 0; index < count; index++) {
		if (candidates[index].max_output_channels <= 0)
			continue;
		if (selector->count >= CP_SELECTOR_ITEMS_MAX)
			break;
		status = cp_selector_format_output_label(&candidates[index],
		    current_device, requested_set, requested_device, label,
		    sizeof(label));
		if (status != CP_OK)
			return status;
		status = cp_selector_format_output_value(&candidates[index],
		    value, sizeof(value));
		if (status != CP_OK)
			return status;
		status = cp_selector_add(selector, label, value, 1);
		if (status != CP_OK)
			return status;

		loaded_index = selector->count - 1;
		if (candidates[index].default_output)
			default_index = loaded_index;
		if (candidates[index].index == current_device)
			selected_current = loaded_index;
		if (requested_set &&
		    candidates[index].index == requested_device)
			selected_requested = loaded_index;
	}

	if (selector->count == 0)
		return CP_OK;
	if (requested_set && requested_device == CP_AUDIO_DEFAULT_DEVICE &&
	    default_index != (size_t)-1)
		selected_requested = default_index;
	if (current_device == CP_AUDIO_DEFAULT_DEVICE &&
	    default_index != (size_t)-1)
		selected_current = default_index;
	if (selected_requested != (size_t)-1)
		selector->selected = selected_requested;
	else if (selected_current != (size_t)-1)
		selector->selected = selected_current;
	else if (default_index != (size_t)-1)
		selector->selected = default_index;

	return CP_OK;
}

int
cp_selector_load_audio_files(struct cp_selector *selector,
	const char *const *paths, size_t count, const char *current_path,
	const char *requested_path)
{
	size_t index;
	size_t selected_current;
	size_t selected_requested;
	size_t loaded_index;
	int status;

	if (selector == NULL)
		return CP_ERR_NULL;
	if (paths == NULL && count > 0)
		return CP_ERR_NULL;

	cp_selector_init(selector, CP_SELECTOR_AUDIO_FILE);
	selected_current = (size_t)-1;
	selected_requested = (size_t)-1;
	for (index = 0; index < count; index++) {
		if (paths[index] == NULL || paths[index][0] == '\0')
			continue;
		if (cp_selector_value_exists(selector, paths[index]))
			continue;
		if (selector->count >= CP_SELECTOR_ITEMS_MAX)
			break;
		status = cp_selector_add_audio_file(selector, NULL,
		    paths[index], current_path, requested_path);
		if (status != CP_OK)
			return status;
		loaded_index = selector->count - 1;
		if (selector->items[loaded_index].enabled &&
		    current_path != NULL &&
		    strcmp(paths[index], current_path) == 0)
			selected_current = loaded_index;
		if (selector->items[loaded_index].enabled &&
		    requested_path != NULL &&
		    strcmp(paths[index], requested_path) == 0)
			selected_requested = loaded_index;
	}
	if (selected_requested != (size_t)-1)
		selector->selected = selected_requested;
	else if (selected_current != (size_t)-1)
		selector->selected = selected_current;

	return CP_OK;
}

int
cp_selector_load_playlists(struct cp_selector *selector,
	const char *const *paths, size_t count, const char *current_path,
	const char *requested_path)
{
	size_t index;
	size_t loaded_index;
	size_t selected_current;
	size_t selected_requested;
	int status;

	if (selector == NULL)
		return CP_ERR_NULL;
	if (paths == NULL && count > 0)
		return CP_ERR_NULL;

	cp_selector_init(selector, CP_SELECTOR_PLAYLIST);
	selected_current = (size_t)-1;
	selected_requested = (size_t)-1;
	for (index = 0; index < count; index++) {
		if (paths[index] == NULL || paths[index][0] == '\0')
			continue;
		if (cp_selector_value_exists(selector, paths[index]))
			continue;
		if (selector->count >= CP_SELECTOR_ITEMS_MAX)
			break;
		status = cp_selector_add_playlist(selector, NULL,
		    paths[index], current_path, requested_path);
		if (status != CP_OK)
			return status;
		loaded_index = selector->count - 1;
		if (selector->items[loaded_index].enabled &&
		    current_path != NULL &&
		    strcmp(paths[index], current_path) == 0)
			selected_current = loaded_index;
		if (selector->items[loaded_index].enabled &&
		    requested_path != NULL &&
		    strcmp(paths[index], requested_path) == 0)
			selected_requested = loaded_index;
	}
	if (selected_requested != (size_t)-1)
		selector->selected = selected_requested;
	else if (selected_current != (size_t)-1)
		selector->selected = selected_current;

	return CP_OK;
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
cp_selector_format_output_label(
	const struct cp_audio_device_candidate *candidate, int current_device,
	int requested_set, int requested_device, char *buffer,
	size_t buffer_size)
{
	char markers[64];
	char name[56];

	if (candidate == NULL || buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;

	cp_selector_truncate_text(name, sizeof(name),
	    candidate->name == NULL || candidate->name[0] == '\0' ?
	    "unnamed" : candidate->name, sizeof(name) - 1);
	markers[0] = '\0';
	if (candidate->index == current_device ||
	    (current_device == CP_AUDIO_DEFAULT_DEVICE &&
	    candidate->default_output))
		cp_selector_marker_append(markers, sizeof(markers),
		    "current");
	if (requested_set && (candidate->index == requested_device ||
	    (requested_device == CP_AUDIO_DEFAULT_DEVICE &&
	    candidate->default_output)))
		cp_selector_marker_append(markers, sizeof(markers),
		    "requested");
	if (candidate->default_output)
		cp_selector_marker_append(markers, sizeof(markers),
		    "default");

	if (markers[0] != '\0') {
		return cp_selector_snprintf(buffer, buffer_size, "%s [%s]",
		    name, markers);
	}

	return cp_selector_snprintf(buffer, buffer_size, "%s (%d out)",
	    name, candidate->max_output_channels);
}

static int
cp_selector_format_audio_label(const char *label, const char *path,
	const char *current_path, const char *requested_path, char *buffer,
	size_t buffer_size, int *enabled)
{
	char markers[96];
	char name[56];
	const char *source_label;

	if (path == NULL || buffer == NULL || buffer_size == 0 ||
	    enabled == NULL)
		return CP_ERR_NULL;
	if (path[0] == '\0')
		return CP_ERR_RANGE;

	*enabled = cp_selector_path_is_wav(path);
	source_label = label != NULL && label[0] != '\0' ? label :
	    cp_selector_path_label(path);
	cp_selector_truncate_text(name, sizeof(name), source_label,
	    sizeof(name) - 1);
	markers[0] = '\0';
	if (current_path != NULL && strcmp(path, current_path) == 0)
		cp_selector_marker_append(markers, sizeof(markers),
		    "current");
	if (requested_path != NULL && strcmp(path, requested_path) == 0)
		cp_selector_marker_append(markers, sizeof(markers),
		    "requested");
	if (!*enabled) {
		cp_selector_marker_append(markers, sizeof(markers),
		    cp_selector_path_is_compressed(path) ?
		    "convert externally" : "unsupported");
	}
	if (markers[0] != '\0') {
		return cp_selector_snprintf(buffer, buffer_size, "%s [%s]",
		    name, markers);
	}

	return cp_selector_snprintf(buffer, buffer_size, "%s", name);
}

static int
cp_selector_format_output_value(
	const struct cp_audio_device_candidate *candidate, char *buffer,
	size_t buffer_size)
{
	if (candidate == NULL || buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;

	return cp_selector_snprintf(buffer, buffer_size, "%d",
	    candidate->index);
}

static int
cp_selector_format_playlist_label(const char *label, const char *path,
	const char *current_path, const char *requested_path, char *buffer,
	size_t buffer_size, int *enabled)
{
	char markers[96];
	char name[56];
	const char *source_label;

	if (path == NULL || buffer == NULL || buffer_size == 0 ||
	    enabled == NULL)
		return CP_ERR_NULL;
	if (path[0] == '\0')
		return CP_ERR_RANGE;

	*enabled = cp_selector_path_is_playlist(path);
	source_label = label != NULL && label[0] != '\0' ? label :
	    cp_selector_path_label(path);
	cp_selector_truncate_text(name, sizeof(name), source_label,
	    sizeof(name) - 1);
	markers[0] = '\0';
	if (current_path != NULL && strcmp(path, current_path) == 0)
		cp_selector_marker_append(markers, sizeof(markers),
		    "current");
	if (requested_path != NULL && strcmp(path, requested_path) == 0)
		cp_selector_marker_append(markers, sizeof(markers),
		    "requested");
	if (!*enabled) {
		cp_selector_marker_append(markers, sizeof(markers),
		    cp_selector_path_is_wav(path) ||
		    cp_selector_path_is_compressed(path) ?
		    "not a playlist" : "unsupported");
	}
	if (markers[0] != '\0') {
		return cp_selector_snprintf(buffer, buffer_size, "%s [%s]",
		    name, markers);
	}

	return cp_selector_snprintf(buffer, buffer_size, "%s", name);
}

static int
cp_selector_extension_matches(const char *path, const char *extension)
{
	size_t extension_length;
	size_t index;
	size_t path_length;
	char a;
	char b;

	if (path == NULL || extension == NULL)
		return 0;

	path_length = strlen(path);
	extension_length = strlen(extension);
	if (path_length < extension_length)
		return 0;

	path += path_length - extension_length;
	for (index = 0; index < extension_length; index++) {
		a = path[index];
		b = extension[index];
		if (a >= 'A' && a <= 'Z')
			a = (char)(a - 'A' + 'a');
		if (b >= 'A' && b <= 'Z')
			b = (char)(b - 'A' + 'a');
		if (a != b)
			return 0;
	}

	return 1;
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
cp_selector_path_is_compressed(const char *path)
{
	return cp_selector_extension_matches(path, ".mp3") ||
	    cp_selector_extension_matches(path, ".flac") ||
	    cp_selector_extension_matches(path, ".ogg") ||
	    cp_selector_extension_matches(path, ".opus") ||
	    cp_selector_extension_matches(path, ".m4a") ||
	    cp_selector_extension_matches(path, ".aac");
}

static int
cp_selector_path_is_playlist(const char *path)
{
	return cp_selector_extension_matches(path, ".txt") ||
	    cp_selector_extension_matches(path, ".playlist");
}

static int
cp_selector_path_is_wav(const char *path)
{
	return cp_selector_extension_matches(path, ".wav");
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

static const char *
cp_selector_path_label(const char *path)
{
	const char *label;
	const char *scan;

	if (path == NULL)
		return "";

	label = path;
	for (scan = path; *scan != '\0'; scan++) {
		if (*scan == '/' || *scan == '\\')
			label = scan + 1;
	}

	return label;
}

static int
cp_selector_marker_append(char *buffer, size_t buffer_size, const char *marker)
{
	size_t length;

	if (buffer == NULL || marker == NULL || buffer_size == 0)
		return CP_ERR_NULL;

	length = strlen(buffer);
	if (length > 0 && length + 1 < buffer_size) {
		buffer[length] = ' ';
		buffer[length + 1] = '\0';
		length++;
	}
	if (length < buffer_size - 1)
		(void)snprintf(buffer + length, buffer_size - length, "%s",
		    marker);

	return CP_OK;
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

static void
cp_selector_truncate_text(char *buffer, size_t buffer_size, const char *text,
	size_t max_chars)
{
	size_t copy;
	size_t length;

	if (buffer == NULL || buffer_size == 0)
		return;
	if (text == NULL)
		text = "";
	if (max_chars >= buffer_size)
		max_chars = buffer_size - 1;

	length = strlen(text);
	if (length <= max_chars) {
		(void)snprintf(buffer, buffer_size, "%s", text);
		return;
	}
	if (max_chars <= 3) {
		copy = max_chars;
		(void)memcpy(buffer, text, copy);
		buffer[copy] = '\0';
		return;
	}

	copy = max_chars - 3;
	(void)memcpy(buffer, text, copy);
	(void)memcpy(buffer + copy, "...", 4);
}

static int
cp_selector_value_exists(const struct cp_selector *selector, const char *value)
{
	size_t index;

	if (selector == NULL || value == NULL)
		return 0;

	for (index = 0; index < selector->count; index++) {
		if (strcmp(selector->items[index].value, value) == 0)
			return 1;
	}

	return 0;
}

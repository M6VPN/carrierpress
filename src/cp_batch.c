/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_batch.c */

#include <sys/types.h>
#include <sys/stat.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "cp_batch.h"

static int	cp_batch_add_item(struct cp_batch_plan *, size_t,
		    enum cp_batch_item_status, const char *, const char *);
static void	cp_batch_clear_error(struct cp_batch_error *);
static int	cp_batch_copy_string(char *, size_t, const char *);
static int	cp_batch_file_exists(const char *);
static const char	*cp_batch_path_basename(const char *);
static int	cp_batch_path_has_ext(const char *, const char *);
static int	cp_batch_plan_item_paths(struct cp_batch_item *, const char *);
static void	cp_batch_plan_recount(struct cp_batch_plan *);
static void	cp_batch_set_error(struct cp_batch_error *, size_t,
		    const char *, const char *);
static int	cp_batch_trim_line(char *);

void
cp_batch_plan_init(struct cp_batch_plan *plan)
{
	if (plan == NULL)
		return;
	(void)memset(plan, 0, sizeof(*plan));
}

void
cp_batch_plan_free(struct cp_batch_plan *plan)
{
	(void)plan;
}

int
cp_batch_plan_check_overwrites(struct cp_batch_plan *plan,
	int allow_overwrite)
{
	struct cp_batch_item *item;
	size_t index;

	if (plan == NULL)
		return CP_BATCH_ERR_NULL;

	for (index = 0; index < plan->item_count; index++) {
		item = &plan->items[index];
		if (item->status != CP_BATCH_ITEM_OK)
			continue;
		if (cp_batch_file_exists(item->output_path)) {
			item->status = allow_overwrite ?
			    CP_BATCH_ITEM_WARNING : CP_BATCH_ITEM_ERROR;
			(void)cp_batch_copy_string(item->reason,
			    sizeof(item->reason), "output already exists");
			continue;
		}
		if (cp_batch_file_exists(item->report_path)) {
			item->status = allow_overwrite ?
			    CP_BATCH_ITEM_WARNING : CP_BATCH_ITEM_ERROR;
			(void)cp_batch_copy_string(item->reason,
			    sizeof(item->reason), "report already exists");
		}
	}
	cp_batch_plan_recount(plan);
	if (plan->errors > 0)
		return CP_BATCH_ERR_BAD;

	return CP_BATCH_OK;
}

int
cp_batch_plan_load(const char *list_path, const char *output_dir,
	struct cp_batch_plan *plan, struct cp_batch_error *error)
{
	struct cp_batch_item *item;
	FILE *file;
	char line[CP_BATCH_MAX_LINE + 2];
	size_t index;
	size_t other;
	size_t line_number;
	size_t len;
	int too_long;

	if (plan == NULL)
		return CP_BATCH_ERR_NULL;
	cp_batch_plan_init(plan);
	cp_batch_clear_error(error);
	if (list_path == NULL || output_dir == NULL ||
	    list_path[0] == '\0' || output_dir[0] == '\0') {
		cp_batch_set_error(error, 0, list_path, "missing batch path or "
		    "output directory");
		plan->errors = 1;
		return CP_BATCH_ERR_BAD;
	}
	if (!cp_batch_copy_string(plan->list_path, sizeof(plan->list_path),
	    list_path) ||
	    !cp_batch_copy_string(plan->output_dir, sizeof(plan->output_dir),
	    output_dir)) {
		cp_batch_set_error(error, 0, list_path, "path is too long");
		plan->errors = 1;
		return CP_BATCH_ERR_RANGE;
	}
	file = fopen(list_path, "r");
	if (file == NULL) {
		cp_batch_set_error(error, 0, list_path, "could not open batch "
		    "list");
		plan->errors = 1;
		return CP_BATCH_ERR_OPEN;
	}

	line_number = 0;
	while (fgets(line, sizeof(line), file) != NULL) {
		line_number++;
		plan->total_lines = line_number;
		len = strlen(line);
		too_long = len > 0 && line[len - 1] != '\n' &&
		    !feof(file);
		if (too_long) {
			while (fgets(line, sizeof(line), file) != NULL) {
				len = strlen(line);
				if (len > 0 && line[len - 1] == '\n')
					break;
			}
			if (!cp_batch_add_item(plan, line_number,
			    CP_BATCH_ITEM_ERROR, "", "batch line is too long")) {
				fclose(file);
				cp_batch_set_error(error, line_number, "",
				    "too many batch entries");
				return CP_BATCH_ERR_RANGE;
			}
			continue;
		}
		if (!cp_batch_trim_line(line)) {
			if (!cp_batch_add_item(plan, line_number,
			    CP_BATCH_ITEM_SKIPPED, "", "blank/comment")) {
				fclose(file);
				cp_batch_set_error(error, line_number, "",
				    "too many batch entries");
				return CP_BATCH_ERR_RANGE;
			}
			continue;
		}
		if (!cp_batch_path_is_supported_input(line)) {
			if (!cp_batch_add_item(plan, line_number,
			    CP_BATCH_ITEM_ERROR, line,
			    "unsupported format: convert to WAV first")) {
				fclose(file);
				cp_batch_set_error(error, line_number, line,
				    "too many batch entries");
				return CP_BATCH_ERR_RANGE;
			}
			continue;
		}
		if (!cp_batch_add_item(plan, line_number, CP_BATCH_ITEM_OK,
		    line, "")) {
			fclose(file);
			cp_batch_set_error(error, line_number, line,
			    "too many batch entries");
			return CP_BATCH_ERR_RANGE;
		}
		item = &plan->items[plan->item_count - 1];
		if (!cp_batch_plan_item_paths(item, output_dir)) {
			item->status = CP_BATCH_ITEM_ERROR;
			(void)cp_batch_copy_string(item->reason,
			    sizeof(item->reason), "planned path is too long");
		}
	}
	fclose(file);

	for (index = 0; index < plan->item_count; index++) {
		item = &plan->items[index];
		if (item->status != CP_BATCH_ITEM_OK)
			continue;
		for (other = 0; other < index; other++) {
			if (plan->items[other].status != CP_BATCH_ITEM_OK &&
			    plan->items[other].status !=
			    CP_BATCH_ITEM_WARNING)
				continue;
			if (strcmp(item->output_path,
			    plan->items[other].output_path) == 0) {
				item->status = CP_BATCH_ITEM_ERROR;
				(void)cp_batch_copy_string(item->reason,
				    sizeof(item->reason),
				    "duplicate output path");
				break;
			}
			if (strcmp(item->report_path,
			    plan->items[other].report_path) == 0) {
				item->status = CP_BATCH_ITEM_ERROR;
				(void)cp_batch_copy_string(item->reason,
				    sizeof(item->reason),
				    "duplicate report path");
				break;
			}
		}
	}

	cp_batch_plan_recount(plan);
	if (plan->errors > 0)
		return CP_BATCH_ERR_BAD;

	return CP_BATCH_OK;
}

void
cp_batch_plan_print(const struct cp_batch_plan *plan, FILE *file)
{
	const struct cp_batch_item *item;
	size_t index;

	if (plan == NULL || file == NULL)
		return;

	fprintf(file, "batch: %s\n", plan->list_path);
	fprintf(file, "output_dir: %s\n", plan->output_dir);
	for (index = 0; index < plan->item_count; index++) {
		item = &plan->items[index];
		if (item->status == CP_BATCH_ITEM_SKIPPED) {
			fprintf(file, "line %zu: skipped: %s\n",
			    item->line_number, item->reason);
		} else if (item->status == CP_BATCH_ITEM_OK) {
			fprintf(file, "line %zu: ok input=%s output=%s "
			    "report=%s\n", item->line_number,
			    item->input_path, item->output_path,
			    item->report_path);
		} else if (item->status == CP_BATCH_ITEM_WARNING) {
			fprintf(file, "line %zu: warning input=%s output=%s "
			    "report=%s reason=%s\n", item->line_number,
			    item->input_path, item->output_path,
			    item->report_path, item->reason);
		} else {
			fprintf(file, "line %zu: error input=%s reason=%s\n",
			    item->line_number, item->input_path,
			    item->reason);
		}
	}
	fprintf(file, "summary: items=%zu warnings=%zu errors=%zu "
	    "status=%s\n", plan->planned_items, plan->warnings,
	    plan->errors, plan->errors > 0 ? "fail" : "ok");
}

int
cp_batch_path_is_supported_input(const char *path)
{
	if (path == NULL || path[0] == '\0')
		return 0;

	return cp_batch_path_has_ext(path, ".wav");
}

const char *
cp_batch_status_string(int status)
{
	switch (status) {
	case CP_BATCH_OK:
		return "ok";
	case CP_BATCH_ERR_NULL:
		return "null argument";
	case CP_BATCH_ERR_OPEN:
		return "could not open batch list";
	case CP_BATCH_ERR_BAD:
		return "batch has errors";
	case CP_BATCH_ERR_RANGE:
		return "batch value out of range";
	default:
		return "unknown batch error";
	}
}

static int
cp_batch_add_item(struct cp_batch_plan *plan, size_t line_number,
	enum cp_batch_item_status status, const char *path, const char *reason)
{
	struct cp_batch_item *item;

	if (plan == NULL || plan->item_count >= CP_BATCH_MAX_ITEMS)
		return 0;
	item = &plan->items[plan->item_count++];
	(void)memset(item, 0, sizeof(*item));
	item->line_number = line_number;
	item->status = status;
	if (path != NULL &&
	    !cp_batch_copy_string(item->input_path, sizeof(item->input_path),
	    path))
		return 0;
	if (reason != NULL &&
	    !cp_batch_copy_string(item->reason, sizeof(item->reason), reason))
		return 0;

	return 1;
}

static void
cp_batch_clear_error(struct cp_batch_error *error)
{
	if (error != NULL)
		(void)memset(error, 0, sizeof(*error));
}

static int
cp_batch_copy_string(char *dst, size_t dstlen, const char *src)
{
	int written;

	if (dst == NULL || dstlen == 0 || src == NULL)
		return 0;
	written = snprintf(dst, dstlen, "%s", src);
	if (written < 0 || (size_t)written >= dstlen)
		return 0;

	return 1;
}

static int
cp_batch_file_exists(const char *path)
{
	struct stat st;

	if (path == NULL || path[0] == '\0')
		return 0;

	return stat(path, &st) == 0;
}

static const char *
cp_batch_path_basename(const char *path)
{
	const char *base;
	const char *slash;
	const char *backslash;

	if (path == NULL)
		return NULL;
	slash = strrchr(path, '/');
	backslash = strrchr(path, '\\');
	base = path;
	if (slash != NULL && slash + 1 > base)
		base = slash + 1;
	if (backslash != NULL && backslash + 1 > base)
		base = backslash + 1;

	return base;
}

static int
cp_batch_path_has_ext(const char *path, const char *extension)
{
	const char *dot;
	size_t path_len;
	size_t ext_len;
	size_t index;

	if (path == NULL || extension == NULL)
		return 0;
	path_len = strlen(path);
	ext_len = strlen(extension);
	if (path_len <= ext_len)
		return 0;
	dot = path + path_len - ext_len;
	for (index = 0; index < ext_len; index++) {
		if (tolower((unsigned char)dot[index]) !=
		    tolower((unsigned char)extension[index]))
			return 0;
	}

	return 1;
}

static int
cp_batch_plan_item_paths(struct cp_batch_item *item, const char *output_dir)
{
	const char *base;
	char report_base[CP_BATCH_MAX_PATH];
	size_t base_len;
	size_t ext_len;
	size_t out_len;
	int written;

	if (item == NULL || output_dir == NULL)
		return 0;
	base = cp_batch_path_basename(item->input_path);
	if (base == NULL || base[0] == '\0' || strcmp(base, ".") == 0 ||
	    strcmp(base, "..") == 0) {
		(void)cp_batch_copy_string(item->reason, sizeof(item->reason),
		    "invalid input basename");
		return 0;
	}
	out_len = strlen(output_dir);
	written = snprintf(item->output_path, sizeof(item->output_path),
	    "%s%s%s", output_dir,
	    out_len > 0 && output_dir[out_len - 1] == '/' ? "" : "/", base);
	if (written < 0 || (size_t)written >= sizeof(item->output_path))
		return 0;

	base_len = strlen(base);
	ext_len = strlen(".wav");
	if (base_len <= ext_len)
		return 0;
	if (base_len - ext_len >= sizeof(report_base))
		return 0;
	(void)memcpy(report_base, base, base_len - ext_len);
	report_base[base_len - ext_len] = '\0';
	written = snprintf(item->report_path, sizeof(item->report_path),
	    "%s%s%s.report.json", output_dir,
	    out_len > 0 && output_dir[out_len - 1] == '/' ? "" : "/",
	    report_base);
	if (written < 0 || (size_t)written >= sizeof(item->report_path))
		return 0;

	return 1;
}

static void
cp_batch_plan_recount(struct cp_batch_plan *plan)
{
	size_t index;

	if (plan == NULL)
		return;
	plan->planned_items = 0;
	plan->skipped_items = 0;
	plan->warnings = 0;
	plan->errors = 0;
	for (index = 0; index < plan->item_count; index++) {
		switch (plan->items[index].status) {
		case CP_BATCH_ITEM_SKIPPED:
			plan->skipped_items++;
			break;
		case CP_BATCH_ITEM_OK:
			plan->planned_items++;
			break;
		case CP_BATCH_ITEM_WARNING:
			plan->planned_items++;
			plan->warnings++;
			break;
		case CP_BATCH_ITEM_ERROR:
		default:
			plan->errors++;
			break;
		}
	}
}

static void
cp_batch_set_error(struct cp_batch_error *error, size_t line_number,
	const char *path, const char *reason)
{
	if (error == NULL)
		return;
	error->line_number = line_number;
	if (path != NULL)
		(void)cp_batch_copy_string(error->path, sizeof(error->path),
		    path);
	if (reason != NULL)
		(void)cp_batch_copy_string(error->reason,
		    sizeof(error->reason), reason);
}

static int
cp_batch_trim_line(char *line)
{
	char *start;
	char *end;
	size_t len;

	if (line == NULL)
		return 0;
	len = strlen(line);
	while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r' ||
	    isspace((unsigned char)line[len - 1]))) {
		line[--len] = '\0';
	}
	start = line;
	while (*start != '\0' && isspace((unsigned char)*start))
		start++;
	if (*start == '\0' || *start == '#') {
		line[0] = '\0';
		return 0;
	}
	if (start != line)
		(void)memmove(line, start, strlen(start) + 1);
	end = line + strlen(line);
	while (end > line && isspace((unsigned char)end[-1]))
		*--end = '\0';

	return line[0] != '\0';
}

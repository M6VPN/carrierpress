/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_batch_wav.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_batch_wav.h"
#include "cp_report.h"
#include "cp_version.h"
#include "cp_wav.h"

static int	cp_batch_wav_item_processable(const struct cp_batch_item *);
static void	cp_batch_wav_json_string(FILE *, const char *);
static void	cp_batch_wav_summary_item(FILE *,
		    const struct cp_batch_item *, const char *);

void
cp_batch_wav_result_init(struct cp_batch_wav_result *result)
{
	if (result == NULL)
		return;
	(void)memset(result, 0, sizeof(*result));
	result->last_status = CP_WAV_OK;
}

int
cp_batch_wav_process_plan(const struct cp_batch_plan *plan,
	const struct cp_block_config *config, size_t block_frames,
	struct cp_batch_wav_result *result, FILE *out)
{
	const struct cp_batch_item *item;
	struct cp_wav_report report;
	size_t index;
	size_t ordinal;
	int status;

	if (plan == NULL || result == NULL || block_frames == 0)
		return CP_BATCH_ERR_NULL;
	cp_batch_wav_result_init(result);
	if (out == NULL)
		out = stdout;
	if (plan->errors > 0)
		return CP_BATCH_ERR_BAD;

	result->planned = plan->planned_items;
	result->skipped = plan->skipped_items;
	fprintf(out, "batch_process: start items=%zu\n",
	    plan->planned_items);
	ordinal = 0;
	for (index = 0; index < plan->item_count; index++) {
		item = &plan->items[index];
		if (!cp_batch_wav_item_processable(item))
			continue;
		ordinal++;
		fprintf(out, "batch_process: %zu/%zu input=%s output=%s "
		    "report=%s\n", ordinal, plan->planned_items,
		    item->input_path, item->output_path, item->report_path);
		status = cp_wav_process_file_config_full_sidecar_report(
		    item->input_path, item->output_path, block_frames, config,
		    &report, item->report_path);
		if (status != CP_WAV_OK) {
			result->failed++;
			result->last_status = status;
			fprintf(out, "batch_process: error input=%s reason=%s\n",
			    item->input_path, cp_wav_status_string(status));
			break;
		}
		result->processed++;
		fprintf(out, "batch_process: ok input=%s\n",
		    item->input_path);
	}

	fprintf(out, "batch_process: done processed=%zu failed=%zu status=%s\n",
	    result->processed, result->failed,
	    result->failed == 0 ? "ok" : "fail");

	return result->failed == 0 ? CP_BATCH_OK : CP_BATCH_ERR_BAD;
}

int
cp_batch_wav_write_summary_json(const char *path,
	const struct cp_batch_plan *plan,
	const struct cp_batch_wav_result *result,
	const char *profile_path, const char *profile_name)
{
	const struct cp_batch_item *item;
	FILE *file;
	size_t index;
	size_t written;
	const char *status;
	int close_status;
	int write_error;

	if (path == NULL || plan == NULL || result == NULL)
		return CP_BATCH_ERR_NULL;
	file = fopen(path, "w");
	if (file == NULL)
		return CP_BATCH_ERR_OPEN;

	fprintf(file, "{\n");
	fprintf(file, "  \"carrierpress_report\": \"batch_summary\",\n");
	fprintf(file, "  \"schema_version\": %d,\n",
	    CP_REPORT_SCHEMA_VERSION);
	fprintf(file, "  \"version\": ");
	cp_batch_wav_json_string(file, CP_VERSION_STRING);
	fprintf(file, ",\n");
	fprintf(file, "  \"status\": ");
	cp_batch_wav_json_string(file,
	    result->failed == 0 ? "ok" : "fail");
	fprintf(file, ",\n");
	fprintf(file, "  \"batch_list\": ");
	cp_batch_wav_json_string(file, plan->list_path);
	fprintf(file, ",\n");
	fprintf(file, "  \"output_dir\": ");
	cp_batch_wav_json_string(file, plan->output_dir);
	fprintf(file, ",\n");
	fprintf(file, "  \"profile\": {\n");
	fprintf(file, "    \"path\": ");
	cp_batch_wav_json_string(file, profile_path);
	fprintf(file, ",\n");
	fprintf(file, "    \"name\": ");
	cp_batch_wav_json_string(file, profile_name);
	fprintf(file, "\n");
	fprintf(file, "  },\n");
	fprintf(file, "  \"planned\": %zu,\n", result->planned);
	fprintf(file, "  \"processed\": %zu,\n", result->processed);
	fprintf(file, "  \"failed\": %zu,\n", result->failed);
	fprintf(file, "  \"skipped\": %zu,\n", result->skipped);
	fprintf(file, "  \"last_status\": %d,\n", result->last_status);
	fprintf(file, "  \"items\": [\n");
	written = 0;
	for (index = 0; index < plan->item_count; index++) {
		item = &plan->items[index];
		if (item->status == CP_BATCH_ITEM_SKIPPED)
			continue;
		if (written > 0)
			fprintf(file, ",\n");
		if (written < result->processed)
			status = "ok";
		else if (result->failed > 0 && written == result->processed)
			status = "fail";
		else if (item->status == CP_BATCH_ITEM_ERROR)
			status = "error";
		else
			status = "pending";
		cp_batch_wav_summary_item(file, item, status);
		written++;
	}
	fprintf(file, "\n");
	fprintf(file, "  ]\n");
	fprintf(file, "}\n");

	write_error = ferror(file);
	close_status = fclose(file);
	if (write_error || close_status != 0)
		return CP_BATCH_ERR_BAD;

	return CP_BATCH_OK;
}

static int
cp_batch_wav_item_processable(const struct cp_batch_item *item)
{
	if (item == NULL)
		return 0;

	return item->status == CP_BATCH_ITEM_OK ||
	    item->status == CP_BATCH_ITEM_WARNING;
}

static void
cp_batch_wav_json_string(FILE *file, const char *text)
{
	const unsigned char *cursor;

	if (text == NULL) {
		fprintf(file, "null");
		return;
	}

	fputc('"', file);
	for (cursor = (const unsigned char *)text; *cursor != '\0'; cursor++) {
		switch (*cursor) {
		case '"':
			fputs("\\\"", file);
			break;
		case '\\':
			fputs("\\\\", file);
			break;
		case '\b':
			fputs("\\b", file);
			break;
		case '\f':
			fputs("\\f", file);
			break;
		case '\n':
			fputs("\\n", file);
			break;
		case '\r':
			fputs("\\r", file);
			break;
		case '\t':
			fputs("\\t", file);
			break;
		default:
			if (*cursor < 0x20)
				fprintf(file, "\\u%04x",
				    (unsigned int)*cursor);
			else
				fputc((int)*cursor, file);
			break;
		}
	}
	fputc('"', file);
}

static void
cp_batch_wav_summary_item(FILE *file, const struct cp_batch_item *item,
	const char *status)
{
	fprintf(file, "    {\n");
	fprintf(file, "      \"line\": %zu,\n", item->line_number);
	fprintf(file, "      \"input\": ");
	cp_batch_wav_json_string(file, item->input_path);
	fprintf(file, ",\n");
	fprintf(file, "      \"output\": ");
	cp_batch_wav_json_string(file, item->output_path);
	fprintf(file, ",\n");
	fprintf(file, "      \"report\": ");
	cp_batch_wav_json_string(file, item->report_path);
	fprintf(file, ",\n");
	fprintf(file, "      \"status\": ");
	cp_batch_wav_json_string(file, status);
	fprintf(file, ",\n");
	fprintf(file, "      \"reason\": ");
	cp_batch_wav_json_string(file, item->reason[0] == '\0' ? NULL :
	    item->reason);
	fprintf(file, "\n");
	fprintf(file, "    }");
}

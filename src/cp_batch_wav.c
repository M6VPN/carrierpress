/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_batch_wav.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "cp_batch_wav.h"
#include "cp_wav.h"

static int	cp_batch_wav_item_processable(const struct cp_batch_item *);

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

static int
cp_batch_wav_item_processable(const struct cp_batch_item *item)
{
	if (item == NULL)
		return 0;

	return item->status == CP_BATCH_ITEM_OK ||
	    item->status == CP_BATCH_ITEM_WARNING;
}

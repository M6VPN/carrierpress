/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_batch.h */

#ifndef CP_BATCH_H
#define CP_BATCH_H

#include <sys/types.h>

#include <stdio.h>

#define CP_BATCH_MAX_ITEMS	256
#define CP_BATCH_MAX_PATH	512
#define CP_BATCH_MAX_LINE	4096
#define CP_BATCH_REASON_SIZE	128

enum cp_batch_status {
	CP_BATCH_OK          = 0,
	CP_BATCH_ERR_NULL    = -1000,
	CP_BATCH_ERR_OPEN    = -1001,
	CP_BATCH_ERR_BAD     = -1002,
	CP_BATCH_ERR_RANGE   = -1003
};

enum cp_batch_item_status {
	CP_BATCH_ITEM_SKIPPED = 0,
	CP_BATCH_ITEM_OK,
	CP_BATCH_ITEM_WARNING,
	CP_BATCH_ITEM_ERROR
};

struct cp_batch_item {
	size_t line_number;
	enum cp_batch_item_status status;
	char input_path[CP_BATCH_MAX_PATH];
	char output_path[CP_BATCH_MAX_PATH];
	char report_path[CP_BATCH_MAX_PATH];
	char reason[CP_BATCH_REASON_SIZE];
};

struct cp_batch_plan {
	char list_path[CP_BATCH_MAX_PATH];
	char output_dir[CP_BATCH_MAX_PATH];
	struct cp_batch_item items[CP_BATCH_MAX_ITEMS];
	size_t item_count;
	size_t total_lines;
	size_t planned_items;
	size_t skipped_items;
	size_t warnings;
	size_t errors;
};

struct cp_batch_error {
	size_t line_number;
	char path[CP_BATCH_MAX_PATH];
	char reason[CP_BATCH_REASON_SIZE];
};

void	cp_batch_plan_init(struct cp_batch_plan *);
void	cp_batch_plan_free(struct cp_batch_plan *);
int	cp_batch_plan_check_overwrites(struct cp_batch_plan *, int);
int	cp_batch_plan_load(const char *, const char *,
	    struct cp_batch_plan *, struct cp_batch_error *);
void	cp_batch_plan_print(const struct cp_batch_plan *, FILE *);
int	cp_batch_path_is_supported_input(const char *);
const char	*cp_batch_status_string(int);

#endif

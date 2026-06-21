/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_report.h */

#ifndef CP_REPORT_H
#define CP_REPORT_H

#include <stddef.h>
#include <stdint.h>

#include "cp_auto_eq.h"
#include "cp_block.h"
#include "cp_restoration.h"
#include "cp_types.h"

#define CP_REPORT_SCHEMA_VERSION		1
#define CP_REPORT_SCHEMA_VERSION_STRING		"1"

enum cp_report_status {
	CP_REPORT_OK        = 0,
	CP_REPORT_ERR_NULL  = -300,
	CP_REPORT_ERR_RANGE = -301,
	CP_REPORT_ERR_OPEN  = -302,
	CP_REPORT_ERR_WRITE = -303
};

struct cp_report_metrics {
	double input_square_sum;
	double output_square_sum;
	double input_sum;
	double output_sum;
	cp_sample_t input_peak;
	cp_sample_t output_peak;
	cp_sample_t output_min;
	cp_sample_t output_max;
	uint64_t frames;
	uint64_t samples;
	size_t channels;
	int finite;
	int have_samples;
};

struct cp_report_processed_file {
	const char *input_path;
	const char *output_path;
	const char *profile_path;
	const char *profile_name;
	size_t sample_rate_hz;
	size_t channels;
	uint64_t frames;
	const struct cp_report_metrics *metrics;
	const struct cp_block_config *block_config;
	const struct cp_restoration_metrics *restoration_metrics;
	const struct cp_auto_eq_metrics *auto_eq_metrics;
};

void		cp_report_metrics_init(struct cp_report_metrics *);
int		cp_report_metrics_update(struct cp_report_metrics *,
		    const cp_sample_t *, const cp_sample_t *, size_t, size_t);
const char	*cp_report_status_string(int);
int		cp_report_write_processed_file_json(const char *,
		    const struct cp_report_processed_file *);

#endif

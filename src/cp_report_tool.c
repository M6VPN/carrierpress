/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_report_tool.c */

#include <sys/types.h>

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cp_report.h"
#include "cp_report_tool.h"

#define CP_REPORT_TOOL_MAX_BYTES	(4u * 1024u * 1024u)
#define CP_REPORT_TOOL_TYPE_SIZE	32
#define CP_REPORT_TOOL_VALUE_SIZE	256

enum cp_report_tool_type {
	CP_REPORT_TOOL_TYPE_UNKNOWN = 0,
	CP_REPORT_TOOL_TYPE_QUALITY,
	CP_REPORT_TOOL_TYPE_PROCESSED_FILE,
	CP_REPORT_TOOL_TYPE_BATCH_SUMMARY
};

struct cp_report_tool_doc {
	char *text;
	size_t length;
	enum cp_report_tool_type type;
	int schema_version;
	char source_path[CP_REPORT_TOOL_VALUE_SIZE];
	char version[CP_REPORT_TOOL_VALUE_SIZE];
	char status[CP_REPORT_TOOL_VALUE_SIZE];
	char input[CP_REPORT_TOOL_VALUE_SIZE];
	char output[CP_REPORT_TOOL_VALUE_SIZE];
	char batch_list[CP_REPORT_TOOL_VALUE_SIZE];
	char output_dir[CP_REPORT_TOOL_VALUE_SIZE];
	long sample_rate_hz;
	long frames;
	long channels;
	long planned;
	long processed;
	long failed;
	long skipped;
	size_t cases;
	size_t failed_cases;
};

struct cp_report_tool_compare {
	double max_abs_delta;
	size_t changed_metrics;
};

static double	cp_report_tool_abs(double);
static int	cp_report_tool_compare_metric(const struct cp_report_tool_doc *,
		    const struct cp_report_tool_doc *, const char *, double,
		    struct cp_report_tool_compare *);
static int	cp_report_tool_compare_metric_occurrences(const char *,
		    const char *, const char *, double,
		    struct cp_report_tool_compare *);
static int	cp_report_tool_compare_processed(
		    const struct cp_report_tool_doc *,
		    const struct cp_report_tool_doc *, double, FILE *);
static int	cp_report_tool_compare_quality(
		    const struct cp_report_tool_doc *,
		    const struct cp_report_tool_doc *, double, FILE *);
static size_t	cp_report_tool_count_string_field_after(const char *,
		    const char *, const char *, const char *);
static void	cp_report_tool_doc_clear(struct cp_report_tool_doc *);
static void	cp_report_tool_doc_free(struct cp_report_tool_doc *);
static int	cp_report_tool_doc_load(const char *,
		    struct cp_report_tool_doc *);
static const char *cp_report_tool_find_colon(const char *);
static const char *cp_report_tool_find_key(const char *, const char *);
static int	cp_report_tool_get_long(const char *, const char *, long *);
static int	cp_report_tool_get_number_at(const char *, const char *,
		    double *);
static int	cp_report_tool_get_number_field(const char *, const char *,
		    double *);
static int	cp_report_tool_get_string(const char *, const char *, char *,
		    size_t);
static int	cp_report_tool_parse_doc(struct cp_report_tool_doc *);
static int	cp_report_tool_print_processed_summary(
		    const struct cp_report_tool_doc *, FILE *);
static int	cp_report_tool_print_quality_summary(
		    const struct cp_report_tool_doc *, FILE *);
static int	cp_report_tool_print_batch_summary(
		    const struct cp_report_tool_doc *, FILE *);
static int	cp_report_tool_read_file(const char *, char **, size_t *);
static const char *cp_report_tool_skip_ws(const char *);
static int	cp_report_tool_type_from_string(const char *,
		    enum cp_report_tool_type *);

static const char *processed_metrics[] = {
	"input_rms",
	"output_rms",
	"input_peak",
	"output_peak",
	"input_crest",
	"output_crest",
	"input_dc",
	"output_dc",
	"output_min",
	"output_max",
	"sample_count"
};

static const char *quality_metrics[] = {
	"input_rms",
	"output_rms",
	"input_peak",
	"output_peak",
	"output_min",
	"output_max",
	"input_crest",
	"output_crest",
	"input_dc",
	"output_dc",
	"input_hum50",
	"output_hum50",
	"input_hum60",
	"output_hum60",
	"output_left_rms",
	"output_right_rms",
	"analysis_clip_ratio",
	"analysis_hf_ratio",
	"analysis_clip_confidence",
	"analysis_low_ceiling_confidence",
	"analysis_transient_confidence",
	"analysis_lossy_confidence",
	"analysis_flat_ratio",
	"analysis_peak_repeat_ratio",
	"analysis_peak",
	"analysis_crest",
	"analysis_reason_flags",
	"declipper_samples",
	"declipper_runs",
	"declipper_delta",
	"natural_gr_db",
	"low_boost_gain_db",
	"auto_eq_rms",
	"auto_eq_tilt_db",
	"auto_eq_low",
	"auto_eq_presence",
	"auto_eq_high",
	"bass_eq_recommend_low_db",
	"bass_eq_recommend_high_db",
	"bass_eq_recommend_output_db",
	"bass_eq_recommend_confidence"
};

const char *
cp_report_tool_status_string(int status)
{
	switch (status) {
	case CP_REPORT_TOOL_OK:
		return "ok";
	case CP_REPORT_TOOL_ERR_NULL:
		return "missing report tool argument";
	case CP_REPORT_TOOL_ERR_OPEN:
		return "could not open report file";
	case CP_REPORT_TOOL_ERR_READ:
		return "could not read report file";
	case CP_REPORT_TOOL_ERR_PARSE:
		return "could not parse CarrierPress report";
	case CP_REPORT_TOOL_ERR_SCHEMA:
		return "unsupported or missing report schema version";
	case CP_REPORT_TOOL_ERR_TYPE:
		return "unsupported or mismatched report type";
	case CP_REPORT_TOOL_ERR_COMPARE:
		return "report comparison exceeded tolerance";
	default:
		return "unknown report tool error";
	}
}

int
cp_report_tool_summary_file(const char *path, FILE *out)
{
	struct cp_report_tool_doc doc;
	int status;

	if (path == NULL || out == NULL)
		return CP_REPORT_TOOL_ERR_NULL;

	status = cp_report_tool_doc_load(path, &doc);
	if (status != CP_REPORT_TOOL_OK)
		return status;

	if (doc.type == CP_REPORT_TOOL_TYPE_QUALITY)
		status = cp_report_tool_print_quality_summary(&doc, out);
	else if (doc.type == CP_REPORT_TOOL_TYPE_PROCESSED_FILE)
		status = cp_report_tool_print_processed_summary(&doc, out);
	else if (doc.type == CP_REPORT_TOOL_TYPE_BATCH_SUMMARY)
		status = cp_report_tool_print_batch_summary(&doc, out);
	else
		status = CP_REPORT_TOOL_ERR_TYPE;

	cp_report_tool_doc_free(&doc);

	return status;
}

int
cp_report_tool_compare_files(const char *base_path, const char *new_path,
	double tolerance, FILE *out)
{
	struct cp_report_tool_doc base;
	struct cp_report_tool_doc new_doc;
	int status;

	if (base_path == NULL || new_path == NULL || out == NULL)
		return CP_REPORT_TOOL_ERR_NULL;
	if (!isfinite(tolerance) || tolerance < 0.0)
		return CP_REPORT_TOOL_ERR_COMPARE;

	status = cp_report_tool_doc_load(base_path, &base);
	if (status != CP_REPORT_TOOL_OK)
		return status;
	status = cp_report_tool_doc_load(new_path, &new_doc);
	if (status != CP_REPORT_TOOL_OK) {
		cp_report_tool_doc_free(&base);
		return status;
	}

	if (base.type != new_doc.type) {
		status = CP_REPORT_TOOL_ERR_TYPE;
		goto done;
	}

	if (base.type == CP_REPORT_TOOL_TYPE_QUALITY)
		status = cp_report_tool_compare_quality(&base, &new_doc,
		    tolerance, out);
	else if (base.type == CP_REPORT_TOOL_TYPE_PROCESSED_FILE)
		status = cp_report_tool_compare_processed(&base, &new_doc,
		    tolerance, out);
	else
		status = CP_REPORT_TOOL_ERR_TYPE;

done:
	cp_report_tool_doc_free(&new_doc);
	cp_report_tool_doc_free(&base);

	return status;
}

static double
cp_report_tool_abs(double value)
{
	return value < 0.0 ? -value : value;
}

static int
cp_report_tool_compare_metric(const struct cp_report_tool_doc *base,
	const struct cp_report_tool_doc *new_doc, const char *key,
	double tolerance, struct cp_report_tool_compare *compare)
{
	double base_value;
	double delta;
	double new_value;
	int base_status;
	int new_status;

	base_status = cp_report_tool_get_number_field(base->text, key,
	    &base_value);
	new_status = cp_report_tool_get_number_field(new_doc->text, key,
	    &new_value);
	if (base_status != CP_REPORT_TOOL_OK ||
	    new_status != CP_REPORT_TOOL_OK)
		return CP_REPORT_TOOL_ERR_COMPARE;

	delta = cp_report_tool_abs(new_value - base_value);
	if (delta > compare->max_abs_delta)
		compare->max_abs_delta = delta;
	if (delta > tolerance)
		compare->changed_metrics++;

	return CP_REPORT_TOOL_OK;
}

static int
cp_report_tool_compare_metric_occurrences(const char *base_text,
	const char *new_text, const char *key, double tolerance,
	struct cp_report_tool_compare *compare)
{
	const char *base_cursor;
	const char *new_cursor;
	double base_value;
	double delta;
	double new_value;
	size_t count;

	base_cursor = base_text;
	new_cursor = new_text;
	count = 0;
	for (;;) {
		base_cursor = cp_report_tool_find_key(base_cursor, key);
		new_cursor = cp_report_tool_find_key(new_cursor, key);
		if (base_cursor == NULL && new_cursor == NULL)
			break;
		if (base_cursor == NULL || new_cursor == NULL)
			return CP_REPORT_TOOL_ERR_COMPARE;
		if (cp_report_tool_get_number_at(base_cursor, key,
		    &base_value) != CP_REPORT_TOOL_OK ||
		    cp_report_tool_get_number_at(new_cursor, key,
		    &new_value) != CP_REPORT_TOOL_OK)
			return CP_REPORT_TOOL_ERR_COMPARE;
		delta = cp_report_tool_abs(new_value - base_value);
		if (delta > compare->max_abs_delta)
			compare->max_abs_delta = delta;
		if (delta > tolerance)
			compare->changed_metrics++;
		base_cursor++;
		new_cursor++;
		count++;
	}

	return count == 0 ? CP_REPORT_TOOL_ERR_COMPARE : CP_REPORT_TOOL_OK;
}

static int
cp_report_tool_compare_processed(const struct cp_report_tool_doc *base,
	const struct cp_report_tool_doc *new_doc, double tolerance, FILE *out)
{
	struct cp_report_tool_compare compare;
	double delta;
	size_t index;
	int status;

	(void)memset(&compare, 0, sizeof(compare));
	if (strcmp(base->status, new_doc->status) != 0)
		compare.changed_metrics++;
	if (base->sample_rate_hz != new_doc->sample_rate_hz ||
	    base->channels != new_doc->channels ||
	    base->frames != new_doc->frames)
		compare.changed_metrics++;

	for (index = 0; index < sizeof(processed_metrics) /
	    sizeof(processed_metrics[0]); index++) {
		status = cp_report_tool_compare_metric(base, new_doc,
		    processed_metrics[index], tolerance, &compare);
		if (status != CP_REPORT_TOOL_OK)
			return status;
	}

	fprintf(out, "compare=processed_file\n");
	fprintf(out, "schema_version=%d\n", CP_REPORT_SCHEMA_VERSION);
	fprintf(out, "base=%s\n", base->source_path);
	fprintf(out, "new=%s\n", new_doc->source_path);
	fprintf(out, "status=%s\n",
	    compare.changed_metrics == 0 ? "pass" : "fail");
	if (cp_report_tool_get_number_field(base->text, "input_rms",
	    &delta) == CP_REPORT_TOOL_OK) {
		double new_value;
		(void)cp_report_tool_get_number_field(new_doc->text,
		    "input_rms", &new_value);
		fprintf(out, "input_rms_delta=%.9g\n", new_value - delta);
	}
	if (cp_report_tool_get_number_field(base->text, "output_rms",
	    &delta) == CP_REPORT_TOOL_OK) {
		double new_value;
		(void)cp_report_tool_get_number_field(new_doc->text,
		    "output_rms", &new_value);
		fprintf(out, "output_rms_delta=%.9g\n", new_value - delta);
	}
	if (cp_report_tool_get_number_field(base->text, "input_peak",
	    &delta) == CP_REPORT_TOOL_OK) {
		double new_value;
		(void)cp_report_tool_get_number_field(new_doc->text,
		    "input_peak", &new_value);
		fprintf(out, "input_peak_delta=%.9g\n", new_value - delta);
	}
	if (cp_report_tool_get_number_field(base->text, "output_peak",
	    &delta) == CP_REPORT_TOOL_OK) {
		double new_value;
		(void)cp_report_tool_get_number_field(new_doc->text,
		    "output_peak", &new_value);
		fprintf(out, "output_peak_delta=%.9g\n", new_value - delta);
	}
	fprintf(out, "max_abs_delta=%.9g\n", compare.max_abs_delta);
	fprintf(out, "changed_metrics=%zu\n", compare.changed_metrics);

	return compare.changed_metrics == 0 ? CP_REPORT_TOOL_OK :
	    CP_REPORT_TOOL_ERR_COMPARE;
}

static int
cp_report_tool_compare_quality(const struct cp_report_tool_doc *base,
	const struct cp_report_tool_doc *new_doc, double tolerance, FILE *out)
{
	struct cp_report_tool_compare compare;
	size_t index;
	int status;

	(void)memset(&compare, 0, sizeof(compare));
	if (strcmp(base->status, new_doc->status) != 0 ||
	    base->cases != new_doc->cases ||
	    base->failed_cases != new_doc->failed_cases ||
	    base->sample_rate_hz != new_doc->sample_rate_hz ||
	    base->frames != new_doc->frames ||
	    base->channels != new_doc->channels)
		compare.changed_metrics++;

	for (index = 0; index < sizeof(quality_metrics) /
	    sizeof(quality_metrics[0]); index++) {
		status = cp_report_tool_compare_metric_occurrences(
		    base->text, new_doc->text, quality_metrics[index],
		    tolerance, &compare);
		if (status != CP_REPORT_TOOL_OK)
			return status;
	}

	fprintf(out, "compare=quality\n");
	fprintf(out, "schema_version=%d\n", CP_REPORT_SCHEMA_VERSION);
	fprintf(out, "base=%s\n", base->source_path);
	fprintf(out, "new=%s\n", new_doc->source_path);
	fprintf(out, "cases_base=%zu\n", base->cases);
	fprintf(out, "cases_new=%zu\n", new_doc->cases);
	fprintf(out, "status=%s\n",
	    compare.changed_metrics == 0 ? "pass" : "fail");
	fprintf(out, "max_abs_delta=%.9g\n", compare.max_abs_delta);
	fprintf(out, "changed_metrics=%zu\n", compare.changed_metrics);

	return compare.changed_metrics == 0 ? CP_REPORT_TOOL_OK :
	    CP_REPORT_TOOL_ERR_COMPARE;
}

static size_t
cp_report_tool_count_string_field_after(const char *text, const char *after,
	const char *key, const char *value)
{
	const char *cursor;
	char buffer[CP_REPORT_TOOL_VALUE_SIZE];
	size_t count;

	if (text == NULL || key == NULL)
		return 0;

	cursor = after == NULL ? text : strstr(text, after);
	if (cursor == NULL)
		cursor = text;

	count = 0;
	for (;;) {
		cursor = cp_report_tool_find_key(cursor, key);
		if (cursor == NULL)
			break;
		if (cp_report_tool_get_string(cursor, key, buffer,
		    sizeof(buffer)) == CP_REPORT_TOOL_OK &&
		    (value == NULL || strcmp(buffer, value) == 0))
			count++;
		cursor++;
	}

	return count;
}

static void
cp_report_tool_doc_clear(struct cp_report_tool_doc *doc)
{
	if (doc == NULL)
		return;

	(void)memset(doc, 0, sizeof(*doc));
}

static void
cp_report_tool_doc_free(struct cp_report_tool_doc *doc)
{
	if (doc == NULL)
		return;

	free(doc->text);
	doc->text = NULL;
	doc->length = 0;
}

static int
cp_report_tool_doc_load(const char *path, struct cp_report_tool_doc *doc)
{
	int written;
	int status;

	if (path == NULL || doc == NULL)
		return CP_REPORT_TOOL_ERR_NULL;

	cp_report_tool_doc_clear(doc);
	written = snprintf(doc->source_path, sizeof(doc->source_path), "%s",
	    path);
	if (written < 0 || (size_t)written >= sizeof(doc->source_path))
		return CP_REPORT_TOOL_ERR_PARSE;
	status = cp_report_tool_read_file(path, &doc->text, &doc->length);
	if (status != CP_REPORT_TOOL_OK)
		return status;
	status = cp_report_tool_parse_doc(doc);
	if (status != CP_REPORT_TOOL_OK)
		cp_report_tool_doc_free(doc);

	return status;
}

static const char *
cp_report_tool_find_colon(const char *cursor)
{
	if (cursor == NULL)
		return NULL;

	while (*cursor != '\0' && *cursor != ':') {
		if (*cursor == '\n' || *cursor == '\r')
			return NULL;
		cursor++;
	}
	if (*cursor != ':')
		return NULL;

	return cp_report_tool_skip_ws(cursor + 1);
}

static const char *
cp_report_tool_find_key(const char *text, const char *key)
{
	char pattern[CP_REPORT_TOOL_VALUE_SIZE];
	const char *cursor;
	const char *match;
	int written;

	if (text == NULL || key == NULL)
		return NULL;

	written = snprintf(pattern, sizeof(pattern), "\"%s\"", key);
	if (written < 0 || (size_t)written >= sizeof(pattern))
		return NULL;

	cursor = text;
	while ((match = strstr(cursor, pattern)) != NULL) {
		cursor = cp_report_tool_skip_ws(match + (size_t)written);
		if (*cursor == ':')
			return match;
		cursor = match + 1;
	}

	return NULL;
}

static int
cp_report_tool_get_long(const char *text, const char *key, long *value)
{
	double parsed;

	if (value == NULL)
		return CP_REPORT_TOOL_ERR_NULL;
	if (cp_report_tool_get_number_field(text, key, &parsed) !=
	    CP_REPORT_TOOL_OK)
		return CP_REPORT_TOOL_ERR_PARSE;
	if (parsed < 0.0 || parsed > (double)LONG_MAX)
		return CP_REPORT_TOOL_ERR_PARSE;

	*value = (long)parsed;
	return CP_REPORT_TOOL_OK;
}

static int
cp_report_tool_get_number_at(const char *cursor, const char *key,
	double *value)
{
	char *end;
	double parsed;

	if (cursor == NULL || key == NULL || value == NULL)
		return CP_REPORT_TOOL_ERR_NULL;
	(void)key;

	cursor = cp_report_tool_find_colon(cursor);
	if (cursor == NULL)
		return CP_REPORT_TOOL_ERR_PARSE;
	if (strncmp(cursor, "null", 4) == 0)
		return CP_REPORT_TOOL_ERR_PARSE;

	errno = 0;
	parsed = strtod(cursor, &end);
	if (errno != 0 || end == cursor || !isfinite(parsed))
		return CP_REPORT_TOOL_ERR_PARSE;

	*value = parsed;
	return CP_REPORT_TOOL_OK;
}

static int
cp_report_tool_get_number_field(const char *text, const char *key,
	double *value)
{
	const char *cursor;

	cursor = cp_report_tool_find_key(text, key);
	if (cursor == NULL)
		return CP_REPORT_TOOL_ERR_PARSE;

	return cp_report_tool_get_number_at(cursor, key, value);
}

static int
cp_report_tool_get_string(const char *text, const char *key, char *buffer,
	size_t buffer_size)
{
	const char *cursor;
	size_t offset;

	if (text == NULL || key == NULL || buffer == NULL || buffer_size == 0)
		return CP_REPORT_TOOL_ERR_NULL;

	cursor = cp_report_tool_find_key(text, key);
	if (cursor == NULL)
		return CP_REPORT_TOOL_ERR_PARSE;
	cursor = cp_report_tool_find_colon(cursor);
	if (cursor == NULL || *cursor != '"')
		return CP_REPORT_TOOL_ERR_PARSE;
	cursor++;

	offset = 0;
	while (*cursor != '\0' && *cursor != '"') {
		if (offset + 1 >= buffer_size)
			return CP_REPORT_TOOL_ERR_PARSE;
		if (*cursor == '\\') {
			cursor++;
			switch (*cursor) {
			case '"':
			case '\\':
			case '/':
				buffer[offset++] = *cursor;
				break;
			case 'b':
				buffer[offset++] = '\b';
				break;
			case 'f':
				buffer[offset++] = '\f';
				break;
			case 'n':
				buffer[offset++] = '\n';
				break;
			case 'r':
				buffer[offset++] = '\r';
				break;
			case 't':
				buffer[offset++] = '\t';
				break;
			default:
				return CP_REPORT_TOOL_ERR_PARSE;
			}
		} else {
			buffer[offset++] = *cursor;
		}
		cursor++;
	}
	if (*cursor != '"')
		return CP_REPORT_TOOL_ERR_PARSE;
	buffer[offset] = '\0';

	return CP_REPORT_TOOL_OK;
}

static int
cp_report_tool_parse_doc(struct cp_report_tool_doc *doc)
{
	char type[CP_REPORT_TOOL_TYPE_SIZE];
	long schema;

	if (doc == NULL || doc->text == NULL)
		return CP_REPORT_TOOL_ERR_NULL;
	if (cp_report_tool_get_string(doc->text, "carrierpress_report", type,
	    sizeof(type)) != CP_REPORT_TOOL_OK)
		return CP_REPORT_TOOL_ERR_PARSE;
	if (cp_report_tool_type_from_string(type, &doc->type) !=
	    CP_REPORT_TOOL_OK)
		return CP_REPORT_TOOL_ERR_TYPE;
	if (cp_report_tool_get_long(doc->text, "schema_version", &schema) !=
	    CP_REPORT_TOOL_OK)
		return CP_REPORT_TOOL_ERR_SCHEMA;
	if (schema != CP_REPORT_SCHEMA_VERSION)
		return CP_REPORT_TOOL_ERR_SCHEMA;

	doc->schema_version = (int)schema;
	if (cp_report_tool_get_string(doc->text, "version", doc->version,
	    sizeof(doc->version)) != CP_REPORT_TOOL_OK)
		return CP_REPORT_TOOL_ERR_PARSE;
	if (cp_report_tool_get_string(doc->text, "status", doc->status,
	    sizeof(doc->status)) != CP_REPORT_TOOL_OK)
		return CP_REPORT_TOOL_ERR_PARSE;
	if (doc->type == CP_REPORT_TOOL_TYPE_QUALITY) {
		if (cp_report_tool_get_long(doc->text, "sample_rate_hz",
		    &doc->sample_rate_hz) != CP_REPORT_TOOL_OK ||
		    cp_report_tool_get_long(doc->text, "frames",
		    &doc->frames) != CP_REPORT_TOOL_OK ||
		    cp_report_tool_get_long(doc->text, "channels",
		    &doc->channels) != CP_REPORT_TOOL_OK)
			return CP_REPORT_TOOL_ERR_PARSE;
		doc->cases = cp_report_tool_count_string_field_after(
		    doc->text, "\"cases\"", "fixture", NULL);
		doc->failed_cases = cp_report_tool_count_string_field_after(
		    doc->text, "\"cases\"", "status", "fail");
		if (doc->cases == 0)
			return CP_REPORT_TOOL_ERR_PARSE;
	} else if (doc->type == CP_REPORT_TOOL_TYPE_PROCESSED_FILE) {
		if (cp_report_tool_get_long(doc->text, "sample_rate_hz",
		    &doc->sample_rate_hz) != CP_REPORT_TOOL_OK ||
		    cp_report_tool_get_long(doc->text, "frames",
		    &doc->frames) != CP_REPORT_TOOL_OK ||
		    cp_report_tool_get_long(doc->text, "channels",
		    &doc->channels) != CP_REPORT_TOOL_OK)
			return CP_REPORT_TOOL_ERR_PARSE;
		if (cp_report_tool_get_string(doc->text, "input", doc->input,
		    sizeof(doc->input)) != CP_REPORT_TOOL_OK ||
		    cp_report_tool_get_string(doc->text, "output", doc->output,
		    sizeof(doc->output)) != CP_REPORT_TOOL_OK)
			return CP_REPORT_TOOL_ERR_PARSE;
	} else if (doc->type == CP_REPORT_TOOL_TYPE_BATCH_SUMMARY) {
		if (cp_report_tool_get_string(doc->text, "batch_list",
		    doc->batch_list, sizeof(doc->batch_list)) !=
		    CP_REPORT_TOOL_OK ||
		    cp_report_tool_get_string(doc->text, "output_dir",
		    doc->output_dir, sizeof(doc->output_dir)) !=
		    CP_REPORT_TOOL_OK ||
		    cp_report_tool_get_long(doc->text, "planned",
		    &doc->planned) != CP_REPORT_TOOL_OK ||
		    cp_report_tool_get_long(doc->text, "processed",
		    &doc->processed) != CP_REPORT_TOOL_OK ||
		    cp_report_tool_get_long(doc->text, "failed",
		    &doc->failed) != CP_REPORT_TOOL_OK ||
		    cp_report_tool_get_long(doc->text, "skipped",
		    &doc->skipped) != CP_REPORT_TOOL_OK)
			return CP_REPORT_TOOL_ERR_PARSE;
	}

	return CP_REPORT_TOOL_OK;
}

static int
cp_report_tool_print_batch_summary(const struct cp_report_tool_doc *doc,
	FILE *out)
{
	if (doc == NULL || out == NULL)
		return CP_REPORT_TOOL_ERR_NULL;

	fprintf(out, "report=batch_summary\n");
	fprintf(out, "schema_version=%d\n", doc->schema_version);
	fprintf(out, "version=%s\n", doc->version);
	fprintf(out, "status=%s\n", doc->status);
	fprintf(out, "batch_list=%s\n", doc->batch_list);
	fprintf(out, "output_dir=%s\n", doc->output_dir);
	fprintf(out, "planned=%ld\n", doc->planned);
	fprintf(out, "processed=%ld\n", doc->processed);
	fprintf(out, "failed=%ld\n", doc->failed);
	fprintf(out, "skipped=%ld\n", doc->skipped);

	return CP_REPORT_TOOL_OK;
}

static int
cp_report_tool_print_processed_summary(const struct cp_report_tool_doc *doc,
	FILE *out)
{
	double input_peak;
	double input_rms;
	double output_peak;
	double output_rms;

	if (doc == NULL || out == NULL)
		return CP_REPORT_TOOL_ERR_NULL;
	if (cp_report_tool_get_number_field(doc->text, "input_rms",
	    &input_rms) != CP_REPORT_TOOL_OK ||
	    cp_report_tool_get_number_field(doc->text, "output_rms",
	    &output_rms) != CP_REPORT_TOOL_OK ||
	    cp_report_tool_get_number_field(doc->text, "input_peak",
	    &input_peak) != CP_REPORT_TOOL_OK ||
	    cp_report_tool_get_number_field(doc->text, "output_peak",
	    &output_peak) != CP_REPORT_TOOL_OK)
		return CP_REPORT_TOOL_ERR_PARSE;

	fprintf(out, "report=processed_file\n");
	fprintf(out, "schema_version=%d\n", doc->schema_version);
	fprintf(out, "version=%s\n", doc->version);
	fprintf(out, "status=%s\n", doc->status);
	fprintf(out, "input=%s\n", doc->input);
	fprintf(out, "output=%s\n", doc->output);
	fprintf(out, "sample_rate_hz=%ld\n", doc->sample_rate_hz);
	fprintf(out, "frames=%ld\n", doc->frames);
	fprintf(out, "channels=%ld\n", doc->channels);
	fprintf(out, "input_rms=%.9g\n", input_rms);
	fprintf(out, "output_rms=%.9g\n", output_rms);
	fprintf(out, "input_peak=%.9g\n", input_peak);
	fprintf(out, "output_peak=%.9g\n", output_peak);

	return CP_REPORT_TOOL_OK;
}

static int
cp_report_tool_print_quality_summary(const struct cp_report_tool_doc *doc,
	FILE *out)
{
	if (doc == NULL || out == NULL)
		return CP_REPORT_TOOL_ERR_NULL;

	fprintf(out, "report=quality\n");
	fprintf(out, "schema_version=%d\n", doc->schema_version);
	fprintf(out, "version=%s\n", doc->version);
	fprintf(out, "status=%s\n", doc->status);
	fprintf(out, "cases=%zu\n", doc->cases);
	fprintf(out, "failed_cases=%zu\n", doc->failed_cases);
	fprintf(out, "sample_rate_hz=%ld\n", doc->sample_rate_hz);
	fprintf(out, "frames=%ld\n", doc->frames);
	fprintf(out, "channels=%ld\n", doc->channels);

	return CP_REPORT_TOOL_OK;
}

static int
cp_report_tool_read_file(const char *path, char **text, size_t *length)
{
	FILE *file;
	char *buffer;
	long file_size;
	size_t count;

	if (path == NULL || text == NULL || length == NULL)
		return CP_REPORT_TOOL_ERR_NULL;

	file = fopen(path, "rb");
	if (file == NULL)
		return CP_REPORT_TOOL_ERR_OPEN;
	if (fseek(file, 0, SEEK_END) != 0) {
		fclose(file);
		return CP_REPORT_TOOL_ERR_READ;
	}
	file_size = ftell(file);
	if (file_size < 0 ||
	    (unsigned long)file_size > CP_REPORT_TOOL_MAX_BYTES) {
		fclose(file);
		return CP_REPORT_TOOL_ERR_READ;
	}
	if (fseek(file, 0, SEEK_SET) != 0) {
		fclose(file);
		return CP_REPORT_TOOL_ERR_READ;
	}
	buffer = malloc((size_t)file_size + 1u);
	if (buffer == NULL) {
		fclose(file);
		return CP_REPORT_TOOL_ERR_READ;
	}
	count = fread(buffer, 1, (size_t)file_size, file);
	if (count != (size_t)file_size || ferror(file)) {
		free(buffer);
		fclose(file);
		return CP_REPORT_TOOL_ERR_READ;
	}
	buffer[count] = '\0';
	fclose(file);

	*text = buffer;
	*length = count;
	return CP_REPORT_TOOL_OK;
}

static const char *
cp_report_tool_skip_ws(const char *cursor)
{
	while (cursor != NULL && (*cursor == ' ' || *cursor == '\t' ||
	    *cursor == '\n' || *cursor == '\r'))
		cursor++;

	return cursor;
}

static int
cp_report_tool_type_from_string(const char *text,
	enum cp_report_tool_type *type)
{
	if (text == NULL || type == NULL)
		return CP_REPORT_TOOL_ERR_NULL;
	if (strcmp(text, "quality") == 0) {
		*type = CP_REPORT_TOOL_TYPE_QUALITY;
		return CP_REPORT_TOOL_OK;
	}
	if (strcmp(text, "processed_file") == 0) {
		*type = CP_REPORT_TOOL_TYPE_PROCESSED_FILE;
		return CP_REPORT_TOOL_OK;
	}
	if (strcmp(text, "batch_summary") == 0) {
		*type = CP_REPORT_TOOL_TYPE_BATCH_SUMMARY;
		return CP_REPORT_TOOL_OK;
	}

	*type = CP_REPORT_TOOL_TYPE_UNKNOWN;
	return CP_REPORT_TOOL_ERR_TYPE;
}

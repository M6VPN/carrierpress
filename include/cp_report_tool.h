/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_report_tool.h */

#ifndef CP_REPORT_TOOL_H
#define CP_REPORT_TOOL_H

#include <stdio.h>

#define CP_REPORT_TOOL_DEFAULT_TOLERANCE	0.000001

enum cp_report_tool_status {
	CP_REPORT_TOOL_OK          = 0,
	CP_REPORT_TOOL_ERR_NULL    = -400,
	CP_REPORT_TOOL_ERR_OPEN    = -401,
	CP_REPORT_TOOL_ERR_READ    = -402,
	CP_REPORT_TOOL_ERR_PARSE   = -403,
	CP_REPORT_TOOL_ERR_SCHEMA  = -404,
	CP_REPORT_TOOL_ERR_TYPE    = -405,
	CP_REPORT_TOOL_ERR_COMPARE = -406
};

const char	*cp_report_tool_status_string(int);
int		cp_report_tool_summary_file(const char *, FILE *);
int		cp_report_tool_compare_files(const char *, const char *,
		    double, FILE *);

#endif

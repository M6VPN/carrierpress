/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_dashboard.h */

#ifndef CP_DASHBOARD_H
#define CP_DASHBOARD_H

enum cp_dashboard_section {
	CP_DASHBOARD_PROCESSING = 0,
	CP_DASHBOARD_METERS,
	CP_DASHBOARD_PLAYOUT,
	CP_DASHBOARD_SELECTORS,
	CP_DASHBOARD_DEVICE,
	CP_DASHBOARD_WORKFLOW,
	CP_DASHBOARD_SAFETY
};

const char	*cp_dashboard_section_title(enum cp_dashboard_section);

#endif

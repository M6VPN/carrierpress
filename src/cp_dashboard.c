/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_dashboard.c */

#include <sys/types.h>

#include "cp_dashboard.h"

const char *
cp_dashboard_section_title(enum cp_dashboard_section section)
{
	switch (section) {
	case CP_DASHBOARD_PROCESSING:
		return "Processing";
	case CP_DASHBOARD_METERS:
		return "Meters";
	case CP_DASHBOARD_PLAYOUT:
		return "Playout";
	case CP_DASHBOARD_SELECTORS:
		return "Selectors";
	case CP_DASHBOARD_DEVICE:
		return "Device";
	case CP_DASHBOARD_WORKFLOW:
		return "Workflow";
	case CP_DASHBOARD_SAFETY:
		return "Safety";
	default:
		return "Status";
	}
}

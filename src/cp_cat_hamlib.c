/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_cat_hamlib.c */

#include <sys/types.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <hamlib/rig.h>

#include "cp_cat_hamlib.h"

static int	cp_hamlib_copy_path(RIG *, const char *);
static void	cp_hamlib_set_status(struct cp_cat_snapshot *,
		    enum cp_cat_status, const char *);

int
cp_cat_hamlib_mode_to_text(uint64_t mode, char *buffer, size_t buffer_size)
{
	const char *text;

	if (buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;

	text = rig_strrmode((rmode_t)mode);
	if (text == NULL || text[0] == '\0')
		return CP_ERR_RANGE;

	return cp_cat_mode_set(buffer, buffer_size, text);
}

int
cp_cat_hamlib_ptt_to_state(int hamlib_ptt, enum cp_cat_ptt_state *ptt)
{
	if (ptt == NULL)
		return CP_ERR_NULL;

	switch ((ptt_t)hamlib_ptt) {
	case RIG_PTT_OFF:
		*ptt = CP_CAT_PTT_OFF;
		return CP_OK;
	case RIG_PTT_ON:
	case RIG_PTT_ON_MIC:
	case RIG_PTT_ON_DATA:
		*ptt = CP_CAT_PTT_ON;
		return CP_OK;
	}

	*ptt = CP_CAT_PTT_UNKNOWN;
	return CP_ERR_RANGE;
}

int
cp_cat_hamlib_snapshot_update(const struct cp_cat_config *config,
	struct cp_cat_snapshot *snapshot)
{
	RIG *rig;
	enum cp_cat_ptt_state ptt_state;
	freq_t frequency;
	pbwidth_t width;
	ptt_t ptt;
	rmode_t mode;
	int status;

	if (config == NULL || snapshot == NULL)
		return CP_ERR_NULL;
	if (cp_cat_validate_config(config) != CP_OK) {
		cp_hamlib_set_status(snapshot, CP_CAT_STATUS_UNAVAILABLE,
		    "invalid hamlib config");
		return CP_OK;
	}
	if (config->hamlib_rig_model == 0) {
		cp_hamlib_set_status(snapshot, CP_CAT_STATUS_UNAVAILABLE,
		    "hamlib rig model required");
		return CP_OK;
	}

	rig_set_debug(RIG_DEBUG_NONE);
	rig = rig_init((rig_model_t)config->hamlib_rig_model);
	if (rig == NULL) {
		cp_hamlib_set_status(snapshot, CP_CAT_STATUS_UNAVAILABLE,
		    "hamlib rig init failed");
		return CP_OK;
	}

	rig->state.rigport.timeout = (int)config->timeout_ms;
	rig->state.rigport.retry = 0;
	if (config->hamlib_rig_speed != 0) {
		rig->state.rigport.parm.serial.rate =
		    (int)config->hamlib_rig_speed;
	}
	if (config->hamlib_rig_path[0] != '\0' &&
	    cp_hamlib_copy_path(rig, config->hamlib_rig_path) != CP_OK) {
		cp_hamlib_set_status(snapshot, CP_CAT_STATUS_UNAVAILABLE,
		    "invalid hamlib rig path");
		rig_cleanup(rig);
		return CP_OK;
	}

	status = rig_open(rig);
	if (status != RIG_OK) {
		cp_hamlib_set_status(snapshot, CP_CAT_STATUS_UNAVAILABLE,
		    "hamlib rig open failed");
		rig_cleanup(rig);
		return CP_OK;
	}

	frequency = 0.0;
	status = rig_get_freq(rig, RIG_VFO_CURR, &frequency);
	if (status != RIG_OK || !isfinite((double)frequency) ||
	    frequency < 0.0 || frequency > (freq_t)UINT64_MAX) {
		cp_hamlib_set_status(snapshot, CP_CAT_STATUS_ERROR,
		    "hamlib frequency read failed");
		rig_close(rig);
		rig_cleanup(rig);
		return CP_OK;
	}
	snapshot->frequency_hz = (uint64_t)(frequency + 0.5);

	mode = RIG_MODE_NONE;
	width = 0;
	status = rig_get_mode(rig, RIG_VFO_CURR, &mode, &width);
	if (status != RIG_OK ||
	    cp_cat_hamlib_mode_to_text((uint64_t)mode, snapshot->mode,
	    sizeof(snapshot->mode)) != CP_OK) {
		cp_hamlib_set_status(snapshot, CP_CAT_STATUS_ERROR,
		    "hamlib mode read failed");
		rig_close(rig);
		rig_cleanup(rig);
		return CP_OK;
	}

	ptt = RIG_PTT_OFF;
	status = rig_get_ptt(rig, RIG_VFO_CURR, &ptt);
	if (status == RIG_OK &&
	    cp_cat_hamlib_ptt_to_state((int)ptt, &ptt_state) == CP_OK) {
		snapshot->ptt = ptt_state;
		cp_hamlib_set_status(snapshot, CP_CAT_STATUS_OK, "ok");
	} else {
		snapshot->ptt = CP_CAT_PTT_UNKNOWN;
		cp_hamlib_set_status(snapshot, CP_CAT_STATUS_OK,
		    "ok ptt unknown");
	}

	snapshot->connected = 1;
	rig_close(rig);
	rig_cleanup(rig);

	return CP_OK;
}

static int
cp_hamlib_copy_path(RIG *rig, const char *path)
{
	int written;

	if (rig == NULL || path == NULL)
		return CP_ERR_NULL;

	written = snprintf(rig->state.rigport.pathname,
	    sizeof(rig->state.rigport.pathname), "%s", path);
	if (written < 0 ||
	    (size_t)written >= sizeof(rig->state.rigport.pathname))
		return CP_ERR_RANGE;

	return CP_OK;
}

static void
cp_hamlib_set_status(struct cp_cat_snapshot *snapshot,
	enum cp_cat_status status, const char *text)
{
	int written;

	if (snapshot == NULL || text == NULL)
		return;

	snapshot->status = status;
	written = snprintf(snapshot->status_text,
	    sizeof(snapshot->status_text), "%s", text);
	if (written < 0 ||
	    (size_t)written >= sizeof(snapshot->status_text)) {
		snapshot->status_text[sizeof(snapshot->status_text) - 1u] =
		    '\0';
	}
}

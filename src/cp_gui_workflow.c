/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_gui_workflow.c */

#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "cp_gui_workflow.h"
#include "cp_playlist_check.h"

static int	cp_gui_workflow_copy_path(char *, size_t, const char *);
static int	cp_gui_workflow_set_reason(struct cp_gui_workflow_request *,
		    const char *);
static int	cp_gui_workflow_snprintf(char *, size_t, const char *, ...);
static int	cp_gui_workflow_status_format(
		    const struct cp_gui_workflow_request *, char *, size_t);

void
cp_gui_workflow_request_clear(struct cp_gui_workflow_request *request)
{
	if (request == NULL)
		return;

	(void)memset(request, 0, sizeof(*request));
	request->type = CP_GUI_WORKFLOW_REQUEST_NONE;
	request->device_index = -1;
	request->validated = 0;
	request->validation_status = CP_OK;
	request->playlist_index = 0;
}

int
cp_gui_workflow_request_format(
	const struct cp_gui_workflow_request *request, char *buffer,
	size_t buffer_size)
{
	if (buffer == NULL || buffer_size == 0)
		return CP_ERR_NULL;
	if (request == NULL ||
	    request->type == CP_GUI_WORKFLOW_REQUEST_NONE) {
		return cp_gui_workflow_snprintf(buffer, buffer_size,
		    "workflow=none");
	}
	if (request->validated)
		return cp_gui_workflow_status_format(request, buffer,
		    buffer_size);

	switch (request->type) {
	case CP_GUI_WORKFLOW_REQUEST_LOAD_WAV:
	case CP_GUI_WORKFLOW_REQUEST_LOAD_PLAYLIST:
		return cp_gui_workflow_snprintf(buffer, buffer_size,
		    "workflow=%s path=%s",
		    cp_gui_workflow_request_type_string(request->type),
		    request->path);
	case CP_GUI_WORKFLOW_REQUEST_CUE_PLAYLIST_ITEM:
		return cp_gui_workflow_snprintf(buffer, buffer_size,
		    "workflow=%s index=%zu path=%s",
		    cp_gui_workflow_request_type_string(request->type),
		    request->playlist_index, request->path);
	case CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE:
		return cp_gui_workflow_snprintf(buffer, buffer_size,
		    "workflow=%s device=%d",
		    cp_gui_workflow_request_type_string(request->type),
		    request->device_index);
	default:
		return cp_gui_workflow_snprintf(buffer, buffer_size,
		    "workflow=unknown");
	}
}

int
cp_gui_workflow_request_from_key(int key, const char *cue_wav_path,
	const char *cue_playlist_path, const char *current_path,
	size_t playlist_index, size_t playlist_count, int output_device,
	struct cp_gui_workflow_request *request)
{
	if (request == NULL)
		return CP_ERR_NULL;

	cp_gui_workflow_request_clear(request);
	switch (key) {
	case 'l':
	case 'L':
		if (cue_wav_path == NULL || cue_wav_path[0] == '\0')
			return CP_ERR_RANGE;
		return cp_gui_workflow_request_set_path(request,
		    CP_GUI_WORKFLOW_REQUEST_LOAD_WAV, cue_wav_path);
	case 'p':
	case 'P':
		if (cue_playlist_path == NULL ||
		    cue_playlist_path[0] == '\0')
			return CP_ERR_RANGE;
		return cp_gui_workflow_request_set_path(request,
		    CP_GUI_WORKFLOW_REQUEST_LOAD_PLAYLIST,
		    cue_playlist_path);
	case 'c':
	case 'C':
		if (current_path == NULL || current_path[0] == '\0' ||
		    playlist_count == 0 || playlist_index >= playlist_count)
			return CP_ERR_RANGE;
		return cp_gui_workflow_request_set_playlist_item(request,
		    current_path, playlist_index);
	case 'o':
		if (output_device < -1)
			return CP_ERR_RANGE;
		return cp_gui_workflow_request_set_device(request,
		    output_device + 1);
	case 'O':
		if (output_device <= -1)
			return cp_gui_workflow_request_set_device(request, -1);
		return cp_gui_workflow_request_set_device(request,
		    output_device - 1);
	default:
		return CP_ERR_RANGE;
	}
}

int
cp_gui_workflow_output_device_restart_needed(int current_output_device,
	const struct cp_gui_workflow_request *request, int *restart_needed,
	int *requested_output_device)
{
	if (request == NULL || restart_needed == NULL ||
	    requested_output_device == NULL)
		return CP_ERR_NULL;
	if (request->type != CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE)
		return CP_ERR_RANGE;
	if (request->device_index < -1)
		return CP_ERR_RANGE;

	*requested_output_device = request->device_index;
	*restart_needed = request->device_index != current_output_device;

	return CP_OK;
}

int
cp_gui_workflow_request_set_device(
	struct cp_gui_workflow_request *request, int device_index)
{
	if (request == NULL)
		return CP_ERR_NULL;
	if (device_index < -1)
		return CP_ERR_RANGE;

	cp_gui_workflow_request_clear(request);
	request->type = CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE;
	request->device_index = device_index;

	return CP_OK;
}

int
cp_gui_workflow_request_set_path(
	struct cp_gui_workflow_request *request,
	enum cp_gui_workflow_request_type type, const char *path)
{
	int status;

	if (request == NULL || path == NULL)
		return CP_ERR_NULL;
	if (type != CP_GUI_WORKFLOW_REQUEST_LOAD_WAV &&
	    type != CP_GUI_WORKFLOW_REQUEST_LOAD_PLAYLIST)
		return CP_ERR_RANGE;

	cp_gui_workflow_request_clear(request);
	status = cp_gui_workflow_copy_path(request->path,
	    sizeof(request->path), path);
	if (status != CP_OK) {
		cp_gui_workflow_request_clear(request);
		return status;
	}
	request->type = type;

	return CP_OK;
}

int
cp_gui_workflow_request_set_playlist_item(
	struct cp_gui_workflow_request *request, const char *path,
	size_t playlist_index)
{
	int status;

	if (request == NULL || path == NULL)
		return CP_ERR_NULL;

	cp_gui_workflow_request_clear(request);
	status = cp_gui_workflow_copy_path(request->path,
	    sizeof(request->path), path);
	if (status != CP_OK) {
		cp_gui_workflow_request_clear(request);
		return status;
	}
	request->type = CP_GUI_WORKFLOW_REQUEST_CUE_PLAYLIST_ITEM;
	request->playlist_index = playlist_index;

	return CP_OK;
}

int
cp_gui_workflow_request_validate(struct cp_gui_workflow_request *request)
{
	struct cp_playlist_check_result playlist_result;
	int status;

	if (request == NULL)
		return CP_ERR_NULL;

	request->validated = 1;
	request->validation_status = CP_OK;
	request->reason[0] = '\0';

	switch (request->type) {
	case CP_GUI_WORKFLOW_REQUEST_NONE:
		return cp_gui_workflow_set_reason(request, "none");
	case CP_GUI_WORKFLOW_REQUEST_LOAD_WAV:
	case CP_GUI_WORKFLOW_REQUEST_CUE_PLAYLIST_ITEM:
		if (request->path[0] == '\0') {
			request->validation_status = CP_ERR_RANGE;
			(void)cp_gui_workflow_set_reason(request,
			    "empty path");
			return CP_ERR_RANGE;
		}
		if (!cp_playlist_check_path_is_wav(request->path)) {
			request->validation_status = CP_ERR_RANGE;
			(void)cp_gui_workflow_set_reason(request,
			    "unsupported format: convert to WAV first");
			return CP_ERR_RANGE;
		}
		return cp_gui_workflow_set_reason(request, "ok");
	case CP_GUI_WORKFLOW_REQUEST_LOAD_PLAYLIST:
		if (request->path[0] == '\0') {
			request->validation_status = CP_ERR_RANGE;
			(void)cp_gui_workflow_set_reason(request,
			    "empty path");
			return CP_ERR_RANGE;
		}
		status = cp_playlist_check_file(request->path,
		    &playlist_result, NULL);
		if (status != CP_PLAYLIST_CHECK_OK) {
			request->validation_status = CP_ERR_RANGE;
			if (status == CP_PLAYLIST_CHECK_ERR_OPEN) {
				(void)cp_gui_workflow_set_reason(request,
				    "could not open playlist");
			} else {
				(void)cp_gui_workflow_set_reason(request,
				    playlist_result.errors > 0 ?
				    "playlist has errors" :
				    "could not validate playlist");
			}
			return CP_ERR_RANGE;
		}
		return cp_gui_workflow_set_reason(request, "ok");
	case CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE:
		return cp_gui_workflow_request_validate_device(request);
	default:
		request->validation_status = CP_ERR_RANGE;
		(void)cp_gui_workflow_set_reason(request,
		    "unknown workflow request");
		return CP_ERR_RANGE;
	}
}

int
cp_gui_workflow_request_validate_device(
	struct cp_gui_workflow_request *request)
{
	if (request == NULL)
		return CP_ERR_NULL;
	if (request->type != CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE)
		return CP_ERR_RANGE;

	request->validated = 1;
	if (request->device_index < -1) {
		request->validation_status = CP_ERR_RANGE;
		(void)cp_gui_workflow_set_reason(request,
		    "invalid output device");
		return CP_ERR_RANGE;
	}
	request->validation_status = CP_OK;
	return cp_gui_workflow_set_reason(request,
	    "deferred output device request");
}

const char *
cp_gui_workflow_request_type_string(enum cp_gui_workflow_request_type type)
{
	switch (type) {
	case CP_GUI_WORKFLOW_REQUEST_NONE:
		return "none";
	case CP_GUI_WORKFLOW_REQUEST_LOAD_WAV:
		return "load_wav";
	case CP_GUI_WORKFLOW_REQUEST_LOAD_PLAYLIST:
		return "load_playlist";
	case CP_GUI_WORKFLOW_REQUEST_CUE_PLAYLIST_ITEM:
		return "cue_playlist_item";
	case CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE:
		return "select_output_device";
	default:
		return "unknown";
	}
}

static int
cp_gui_workflow_copy_path(char *buffer, size_t buffer_size, const char *path)
{
	size_t length;

	if (buffer == NULL || buffer_size == 0 || path == NULL)
		return CP_ERR_NULL;
	if (path[0] == '\0')
		return CP_ERR_RANGE;

	length = strlen(path);
	if (length >= buffer_size)
		return CP_ERR_RANGE;

	(void)memcpy(buffer, path, length + 1);

	return CP_OK;
}

static int
cp_gui_workflow_set_reason(struct cp_gui_workflow_request *request,
	const char *reason)
{
	size_t length;

	if (request == NULL || reason == NULL)
		return CP_ERR_NULL;

	length = strlen(reason);
	if (length >= sizeof(request->reason))
		return CP_ERR_RANGE;
	(void)memcpy(request->reason, reason, length + 1);

	return CP_OK;
}

static int
cp_gui_workflow_snprintf(char *buffer, size_t buffer_size,
	const char *format, ...)
{
	va_list ap;
	int written;

	if (buffer == NULL || buffer_size == 0 || format == NULL)
		return CP_ERR_NULL;

	va_start(ap, format);
	written = vsnprintf(buffer, buffer_size, format, ap);
	va_end(ap);
	if (written < 0 || (size_t)written >= buffer_size) {
		buffer[buffer_size - 1] = '\0';
		return CP_ERR_RANGE;
	}

	return CP_OK;
}

static int
cp_gui_workflow_status_format(
	const struct cp_gui_workflow_request *request, char *buffer,
	size_t buffer_size)
{
	const char *status;

	status = request->validation_status == CP_OK ? "ok" : "error";
	switch (request->type) {
	case CP_GUI_WORKFLOW_REQUEST_LOAD_WAV:
	case CP_GUI_WORKFLOW_REQUEST_LOAD_PLAYLIST:
		return cp_gui_workflow_snprintf(buffer, buffer_size,
		    "workflow=%s status=%s reason=%s path=%s",
		    cp_gui_workflow_request_type_string(request->type),
		    status, request->reason, request->path);
	case CP_GUI_WORKFLOW_REQUEST_CUE_PLAYLIST_ITEM:
		return cp_gui_workflow_snprintf(buffer, buffer_size,
		    "workflow=%s status=%s reason=%s index=%zu path=%s",
		    cp_gui_workflow_request_type_string(request->type),
		    status, request->reason, request->playlist_index,
		    request->path);
	case CP_GUI_WORKFLOW_REQUEST_SELECT_OUTPUT_DEVICE:
		return cp_gui_workflow_snprintf(buffer, buffer_size,
		    "workflow=%s status=%s reason=%s device=%d",
		    cp_gui_workflow_request_type_string(request->type),
		    status, request->reason, request->device_index);
	default:
		return cp_gui_workflow_snprintf(buffer, buffer_size,
		    "workflow=unknown status=%s reason=%s", status,
		    request->reason);
	}
}

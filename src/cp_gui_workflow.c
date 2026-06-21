/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_gui_workflow.c */

#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "cp_gui_workflow.h"

static int	cp_gui_workflow_copy_path(char *, size_t, const char *);
static int	cp_gui_workflow_snprintf(char *, size_t, const char *, ...);

void
cp_gui_workflow_request_clear(struct cp_gui_workflow_request *request)
{
	if (request == NULL)
		return;

	(void)memset(request, 0, sizeof(*request));
	request->type = CP_GUI_WORKFLOW_REQUEST_NONE;
	request->device_index = -1;
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
cp_gui_workflow_request_set_device(
	struct cp_gui_workflow_request *request, int device_index)
{
	if (request == NULL)
		return CP_ERR_NULL;
	if (device_index < 0)
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

/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_cat_flrig.c */

#define _POSIX_C_SOURCE 200112L

#include <sys/types.h>
#include <sys/socket.h>

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cp_cat_flrig.h"

#define CP_FLRIG_METHOD_VFO	"rig.get_vfo"
#define CP_FLRIG_METHOD_MODE	"rig.get_mode"
#define CP_FLRIG_METHOD_PTT	"rig.get_ptt"
#define CP_FLRIG_REQUEST_BYTES	512
#define CP_FLRIG_BODY_BYTES	160
#define CP_FLRIG_VALUE_BYTES	64

static int	cp_flrig_call(const struct cp_cat_config *, const char *,
		    char *, size_t);
static int	cp_flrig_connect(const char *, unsigned int, unsigned int);
static const char
		*cp_flrig_find_body(const char *);
static int	cp_flrig_http_ok(const char *);
static int	cp_flrig_extract_value(const char *, char *, size_t);
static int	cp_flrig_recv_response(int, unsigned int, char *, size_t);
static int	cp_flrig_send_all(int, const char *, size_t, unsigned int);

int
cp_cat_flrig_parse_frequency(const char *xml, uint64_t *frequency_hz)
{
	char value[CP_FLRIG_VALUE_BYTES];
	char *end;
	double parsed;

	if (xml == NULL || frequency_hz == NULL)
		return CP_ERR_NULL;
	if (cp_flrig_extract_value(xml, value, sizeof(value)) != CP_OK)
		return CP_ERR_RANGE;

	errno = 0;
	parsed = strtod(value, &end);
	if (errno != 0 || end == value || *end != '\0' ||
	    !isfinite(parsed) || parsed < 0.0 ||
	    parsed > (double)UINT64_MAX)
		return CP_ERR_RANGE;

	*frequency_hz = (uint64_t)(parsed + 0.5);
	return CP_OK;
}

int
cp_cat_flrig_parse_mode(const char *xml, char *mode, size_t mode_size)
{
	char value[CP_FLRIG_VALUE_BYTES];

	if (xml == NULL || mode == NULL || mode_size == 0)
		return CP_ERR_NULL;
	if (cp_flrig_extract_value(xml, value, sizeof(value)) != CP_OK)
		return CP_ERR_RANGE;

	return cp_cat_mode_set(mode, mode_size, value);
}

int
cp_cat_flrig_parse_ptt(const char *xml, enum cp_cat_ptt_state *ptt)
{
	char value[CP_FLRIG_VALUE_BYTES];
	char *end;
	long parsed;

	if (xml == NULL || ptt == NULL)
		return CP_ERR_NULL;
	if (cp_flrig_extract_value(xml, value, sizeof(value)) != CP_OK)
		return CP_ERR_RANGE;

	errno = 0;
	parsed = strtol(value, &end, 10);
	if (errno != 0 || end == value || *end != '\0')
		return CP_ERR_RANGE;
	if (parsed == 0) {
		*ptt = CP_CAT_PTT_OFF;
		return CP_OK;
	}
	if (parsed == 1) {
		*ptt = CP_CAT_PTT_ON;
		return CP_OK;
	}

	return CP_ERR_RANGE;
}

int
cp_cat_flrig_snapshot_update(const struct cp_cat_config *config,
	struct cp_cat_snapshot *snapshot)
{
	char response[CP_CAT_FLRIG_RESPONSE_BYTES];
	int status;

	if (config == NULL || snapshot == NULL)
		return CP_ERR_NULL;
	if (cp_cat_validate_config(config) != CP_OK) {
		snapshot->status = CP_CAT_STATUS_UNAVAILABLE;
		(void)snprintf(snapshot->status_text,
		    sizeof(snapshot->status_text), "%s",
		    "invalid flrig config");
		return CP_OK;
	}

	status = cp_flrig_call(config, CP_FLRIG_METHOD_VFO, response,
	    sizeof(response));
	if (status != CP_OK) {
		snapshot->status = status == CP_ERR_RANGE ?
		    CP_CAT_STATUS_ERROR : CP_CAT_STATUS_UNAVAILABLE;
		(void)snprintf(snapshot->status_text,
		    sizeof(snapshot->status_text), "%s",
		    status == CP_ERR_RANGE ? "flrig malformed response" :
		    "flrig unavailable");
		return CP_OK;
	}
	status = cp_cat_flrig_parse_frequency(response,
	    &snapshot->frequency_hz);
	if (status != CP_OK) {
		snapshot->status = CP_CAT_STATUS_ERROR;
		(void)snprintf(snapshot->status_text,
		    sizeof(snapshot->status_text), "%s",
		    "flrig malformed frequency");
		return CP_OK;
	}

	status = cp_flrig_call(config, CP_FLRIG_METHOD_MODE, response,
	    sizeof(response));
	if (status != CP_OK ||
	    cp_cat_flrig_parse_mode(response, snapshot->mode,
	    sizeof(snapshot->mode)) != CP_OK) {
		snapshot->status = CP_CAT_STATUS_ERROR;
		(void)snprintf(snapshot->status_text,
		    sizeof(snapshot->status_text), "%s",
		    "flrig malformed mode");
		return CP_OK;
	}

	status = cp_flrig_call(config, CP_FLRIG_METHOD_PTT, response,
	    sizeof(response));
	if (status != CP_OK ||
	    cp_cat_flrig_parse_ptt(response, &snapshot->ptt) != CP_OK) {
		snapshot->status = CP_CAT_STATUS_ERROR;
		(void)snprintf(snapshot->status_text,
		    sizeof(snapshot->status_text), "%s",
		    "flrig malformed ptt");
		return CP_OK;
	}

	snapshot->connected = 1;
	snapshot->status = CP_CAT_STATUS_OK;
	(void)snprintf(snapshot->status_text,
	    sizeof(snapshot->status_text), "%s", "ok");

	return CP_OK;
}

static int
cp_flrig_call(const struct cp_cat_config *config, const char *method,
	char *response, size_t response_size)
{
	char body[CP_FLRIG_BODY_BYTES];
	char request[CP_FLRIG_REQUEST_BYTES];
	int body_len;
	int fd;
	int request_len;
	int status;

	if (config == NULL || method == NULL || response == NULL ||
	    response_size == 0)
		return CP_ERR_NULL;

	body_len = snprintf(body, sizeof(body),
	    "<?xml version=\"1.0\"?>"
	    "<methodCall><methodName>%s</methodName>"
	    "<params></params></methodCall>", method);
	if (body_len < 0 || (size_t)body_len >= sizeof(body))
		return CP_ERR_BUFFER;

	request_len = snprintf(request, sizeof(request),
	    "POST /RPC2 HTTP/1.1\r\n"
	    "Host: %s:%u\r\n"
	    "User-Agent: CarrierPress\r\n"
	    "Content-Type: text/xml\r\n"
	    "Content-Length: %d\r\n"
	    "Connection: close\r\n"
	    "\r\n"
	    "%s",
	    config->flrig_host, config->flrig_port, body_len, body);
	if (request_len < 0 || (size_t)request_len >= sizeof(request))
		return CP_ERR_BUFFER;

	fd = cp_flrig_connect(config->flrig_host, config->flrig_port,
	    config->timeout_ms);
	if (fd < 0)
		return CP_ERR_BUFFER;

	status = cp_flrig_send_all(fd, request, (size_t)request_len,
	    config->timeout_ms);
	if (status == CP_OK)
		status = cp_flrig_recv_response(fd, config->timeout_ms,
		    response, response_size);
	close(fd);
	if (status != CP_OK)
		return status;
	if (!cp_flrig_http_ok(response))
		return CP_ERR_RANGE;
	if (strstr(response, "<fault>") != NULL)
		return CP_ERR_RANGE;

	return CP_OK;
}

static int
cp_flrig_connect(const char *host, unsigned int port, unsigned int timeout_ms)
{
	struct addrinfo hints;
	struct addrinfo *addresses;
	struct addrinfo *item;
	struct pollfd pfd;
	char port_text[16];
	int error_value;
	int fd;
	int flags;
	int poll_status;
	int status;
	socklen_t error_size;

	if (host == NULL)
		return -1;
	if (snprintf(port_text, sizeof(port_text), "%u", port) < 0)
		return -1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV;
	if (getaddrinfo(host, port_text, &hints, &addresses) != 0)
		return -1;

	fd = -1;
	for (item = addresses; item != NULL; item = item->ai_next) {
		fd = socket(item->ai_family, item->ai_socktype,
		    item->ai_protocol);
		if (fd < 0)
			continue;
#ifdef SO_NOSIGPIPE
		{
			int one;

			one = 1;
			(void)setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE,
			    &one, sizeof(one));
		}
#endif
		flags = fcntl(fd, F_GETFL, 0);
		if (flags < 0 ||
		    fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
			close(fd);
			fd = -1;
			continue;
		}
		status = connect(fd, item->ai_addr, item->ai_addrlen);
		if (status == 0)
			break;
		if (errno != EINPROGRESS) {
			close(fd);
			fd = -1;
			continue;
		}
		pfd.fd = fd;
		pfd.events = POLLOUT;
		pfd.revents = 0;
		poll_status = poll(&pfd, 1, (int)timeout_ms);
		if (poll_status <= 0) {
			close(fd);
			fd = -1;
			continue;
		}
		error_value = 0;
		error_size = sizeof(error_value);
		if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error_value,
		    &error_size) != 0 || error_value != 0) {
			close(fd);
			fd = -1;
			continue;
		}
		break;
	}

	freeaddrinfo(addresses);
	return fd;
}

static const char *
cp_flrig_find_body(const char *response)
{
	const char *body;

	if (response == NULL)
		return NULL;

	body = strstr(response, "\r\n\r\n");
	if (body != NULL)
		return body + 4;

	return response;
}

static int
cp_flrig_http_ok(const char *response)
{
	const char *line_end;
	const char *status;
	size_t length;

	if (response == NULL)
		return 0;
	if (strncmp(response, "HTTP/", 5) != 0)
		return 1;

	line_end = strstr(response, "\r\n");
	if (line_end == NULL)
		return 0;
	length = (size_t)(line_end - response);
	status = strstr(response, " 200 ");
	if (status == NULL || (size_t)(status - response) >= length)
		return 0;

	return 1;
}

static int
cp_flrig_extract_value(const char *xml, char *value, size_t value_size)
{
	const char *body;
	const char *end;
	const char *start;
	const char *tags[][2] = {
		{ "<string>", "</string>" },
		{ "<int>", "</int>" },
		{ "<i4>", "</i4>" },
		{ "<double>", "</double>" },
		{ "<boolean>", "</boolean>" }
	};
	size_t index;
	size_t length;

	if (xml == NULL || value == NULL || value_size == 0)
		return CP_ERR_NULL;
	if (strstr(xml, "<fault>") != NULL)
		return CP_ERR_RANGE;

	body = cp_flrig_find_body(xml);
	if (body == NULL)
		return CP_ERR_RANGE;
	for (index = 0; index < sizeof(tags) / sizeof(tags[0]); index++) {
		start = strstr(body, tags[index][0]);
		if (start == NULL)
			continue;
		start += strlen(tags[index][0]);
		end = strstr(start, tags[index][1]);
		if (end == NULL)
			return CP_ERR_RANGE;
		length = (size_t)(end - start);
		if (length == 0 || length >= value_size)
			return CP_ERR_RANGE;
		memcpy(value, start, length);
		value[length] = '\0';
		return CP_OK;
	}

	return CP_ERR_RANGE;
}

static int
cp_flrig_recv_response(int fd, unsigned int timeout_ms, char *response,
	size_t response_size)
{
	struct pollfd pfd;
	ssize_t received;
	size_t used;

	if (response == NULL || response_size == 0)
		return CP_ERR_NULL;

	used = 0;
	while (used < response_size - 1u) {
		pfd.fd = fd;
		pfd.events = POLLIN;
		pfd.revents = 0;
		if (poll(&pfd, 1, (int)timeout_ms) <= 0)
			break;
		received = recv(fd, response + used,
		    response_size - used - 1u, 0);
		if (received == 0)
			break;
		if (received < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				continue;
			return CP_ERR_BUFFER;
		}
		used += (size_t)received;
	}
	if (used == 0)
		return CP_ERR_BUFFER;
	response[used] = '\0';
	if (used >= response_size - 1u)
		return CP_ERR_BUFFER;

	return CP_OK;
}

static int
cp_flrig_send_all(int fd, const char *request, size_t request_size,
	unsigned int timeout_ms)
{
	struct pollfd pfd;
	ssize_t written;
	size_t sent;
	int flags;

	if (request == NULL)
		return CP_ERR_NULL;

	sent = 0;
#ifdef MSG_NOSIGNAL
	flags = MSG_NOSIGNAL;
#else
	flags = 0;
#endif
	while (sent < request_size) {
		pfd.fd = fd;
		pfd.events = POLLOUT;
		pfd.revents = 0;
		if (poll(&pfd, 1, (int)timeout_ms) <= 0)
			return CP_ERR_BUFFER;
		written = send(fd, request + sent, request_size - sent,
		    flags);
		if (written < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				continue;
			return CP_ERR_BUFFER;
		}
		if (written == 0)
			return CP_ERR_BUFFER;
		sent += (size_t)written;
	}

	return CP_OK;
}

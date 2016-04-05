#include <libarmadito.h>
#include "libarmadito-config.h"

#include "ui.h"
#include "net/unixsockclient.h"

#include <errno.h>
#include <unistd.h>

enum a6o_json_status json_handler_ui_request(const char * ip_path, const char * request, int request_len, char * response, int response_len)
{
	enum a6o_json_status status = JSON_OK;
	int fd;
	ssize_t n_read;

	fd = unix_client_connect(ip_path, 10);

	if (fd < 0) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, "error connecting to UI (%s)", strerror(errno));
		return JSON_UNEXPECTED_ERR;
	}

	if (write(fd, request, request_len) < 0) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, "error writing JSON request to UI (%s)", strerror(errno));
		status = JSON_REQUEST_FAILED;
		goto get_out;
	}

	if (write(fd, "\r\n\r\n", 4) < 0) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, "error writing JSON request to UI (%s)", strerror(errno));
		status = JSON_REQUEST_FAILED;
		goto get_out;
	}

	n_read = read(fd, response, response_len);

	if (n_read < 0) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_ERROR, "error reading JSON response from UI (%s)", strerror(errno));
		status = JSON_REQUEST_FAILED;
	}

get_out:
	if (close(fd) < 0) {
		a6o_log(ARMADITO_LOG_MODULE, ARMADITO_LOG_LEVEL_WARNING, "error closing JSON socket to UI (%s)", strerror(errno));
		status = JSON_UNEXPECTED_ERR;
	}

	return status;
}

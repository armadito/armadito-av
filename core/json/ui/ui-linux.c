/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

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

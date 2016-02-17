#include "libuhuru-config.h"
#include <libuhuru/core.h>

#include "os/string.h"

#include "net/unixsockclient.h"
#define DEFAULT_SOCKET_PATH   "/tmp/.uhuru-ihm"

#include <errno.h>
#include <unistd.h>

int json_handler_ui_request(const char *ui_ipc_path, const char *req, int req_len, char *resp, int resp_len)
{
  int fd;
  ssize_t n_read;

  fd = unix_client_connect(ui_ipc_path, 10);

  if (fd < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "error connecting to UI (%s)", strerror(errno));
    return -1;
  }

  if (write(fd, req, strlen(req)) < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "error writing JSON request to UI (%s)", strerror(errno));
    return -1;
  }

  if (write(fd, "\r\n\r\n", 4) < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "error writing JSON request to UI (%s)", strerror(errno));
    return -1;
  }

  n_read = read(fd, resp, resp_len);

  if (n_read < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_ERROR, "error reading JSON response from UI (%s)", strerror(errno));
    return -1;
  }

  if (close(fd) < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "error closing JSON socket to UI (%s)", strerror(errno));
    return -1;
  }

  return 0;
}

#ifdef linux
#include "net/unixsockclient.h"
#define DEFAULT_SOCKET_PATH   "/tmp/.uhuru-ihm"
#endif

  int fd;

  fd = unix_client_connect(DEFAULT_SOCKET_PATH, 10);

  if (fd < 0) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "error connecting to IHM (%s)", strerror(errno));
    return;
  }

  if (write(fd, req, strlen(req)) < 0)
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "error writing JSON response (%s)", strerror(errno));

  if (write(fd, "\r\n\r\n", 4) < 0)
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "error writing JSON response (%s)", strerror(errno));

  close(fd);

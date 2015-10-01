#include "os/ipc.h"

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>

#define UNIX_PATH_MAX	108  /* sic... from taken from /usr/include/linux/un.h */

int os_ipc_connect(const char *url, int max_retry)
{
  int fd =0 ;
  
  return fd;
}


#include "libarmadito-config.h"

#include "os/string.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// Windows specific strerror
char *os_strerror(int errnum)
{
  char * msg = NULL;
  int size = MAXPATHLEN;

  msg = (char*)calloc(size + 1, sizeof(char));
  msg[size] = '\0';

  strerror_s(msg,size,errno);
  
  return msg;
}

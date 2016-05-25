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

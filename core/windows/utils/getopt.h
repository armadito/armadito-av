/***

Copyright (C) 2015, 2016, 2017 Teclib'

This file is part of Armadito.

Armadito is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef _LIBARMADITO_GETOPT_H_
#define _LIBARMADITO_GETOPT_H_

#include <stdlib.h>

struct opt {
  const char *long_form;
  char short_form;
  int need_arg;
  int is_set;
  const char *value;
};

int opt_parse(struct opt *opt, int argc, const char **argv);
int opt_is_set(struct opt *opt, const char *opt_name);
const char *opt_value(struct opt *opt, const char *opt_name, const char *default_value);

#endif

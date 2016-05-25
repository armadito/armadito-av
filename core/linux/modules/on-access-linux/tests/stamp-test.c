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

#include "stamp.h"
#include <stdio.h>

static void stamp_print(struct timespec *t, const char *name)
{
	printf("%s: %ld.%ld s\n", name, t->tv_sec, t->tv_nsec);
}

static void stamp_test(void)
{
	struct timespec now;
	struct timespec now_minus_timeout;
	struct timespec one_s_2 = { 1L,  200000000L};
	struct timespec delta =   { 0L,  700000000L};
	struct timespec timeout = { 1L,  500000000L};

	stamp_now(&now);
	stamp_print(&now, "now");

	stamp_print(&one_s_2, "1.2s");
	stamp_sub(&one_s_2, &delta);
	stamp_print(&one_s_2, "1.2s after sub");

	stamp_print(&timeout, "timeout");

	stamp_cpy(&now_minus_timeout, &now);
	stamp_sub(&now_minus_timeout, &timeout);
	stamp_print(&now_minus_timeout, "now minus timeout");

	printf("stamp_cmp(now_minus_timeout, now) = %d\n", stamp_cmp(&now_minus_timeout, &now));
	printf("stamp_cmp(now, now) = %d\n", stamp_cmp(&now, &now));
	printf("stamp_cmp(now, now_minus_timeout) = %d\n", stamp_cmp(&now, &now_minus_timeout));
}

int main(int argc, char **argv)
{
	stamp_test();
}


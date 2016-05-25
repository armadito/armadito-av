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

#include <libarmadito.h>
#include <stdlib.h>


static a6o_notify_handler_t current_handler = a6o_notify_default_handler;

int a6o_notify_default_handler(enum notif_type type, const char *message)
{
	printf("[-] Warning :: no notify handler! :: call a6o_notify_set_handler first\n");
	return -1;
}

void a6o_notify_set_handler(a6o_notify_handler_t handler)
{
	if (handler != NULL)
		current_handler = handler;
	else
		current_handler = a6o_notify_default_handler;

	return;
}

int a6o_notify(enum notif_type type, const char *format, ... )
{
	int ret = 0;
	va_list args;
	int len = 0;
	char * buffer = NULL;

	// retrieve the variable arguments.
	va_start(args, format);

	len = _vscprintf(format, args) +1;
	//printf("[+] Debug :: a6o_notify :: len = %d\n",len);
	buffer = (char*)calloc(len+1,sizeof(char));
	buffer[len] = '\0';

	vsprintf_s(buffer,len, format, args);

	va_end(args);

	//printf("[+] Debug :: a6o_notify :: message = %s\n",buffer);

	ret = (*current_handler)(type,buffer);

	free(buffer);
	buffer = NULL;

	return ret;
}



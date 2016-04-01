#include <libarmadito/core.h>


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



#include <libuhuru/core.h>


static uhuru_notify_handler_t current_handler = uhuru_notify_default_handler;

int uhuru_notify_default_handler(enum notif_type type, const char *message) {

	printf("[-] Warning :: no notify handler! :: call uhuru_notify_set_handler first\n");
	return -1;

}


void uhuru_notify_set_handler(uhuru_notify_handler_t handler) {

	if (handler != NULL)
		current_handler = handler;
	else
		current_handler = uhuru_notify_default_handler;

	return;
}


int uhuru_notify(enum notif_type type, const char *format, ... ) {

	int ret = 0;
	va_list args;
	int len = 0;
	char * buffer = NULL;

	// retrieve the variable arguments.
	va_start(args, format);

	len = _vscprintf(format, args) +1;
	//printf("[+] Debug :: uhuru_notify :: len = %d\n",len);
	buffer = (char*)calloc(len+1,sizeof(char));
	buffer[len] = '\0';

	vsprintf_s(buffer,len, format, args);

	va_end(args);
	
	//printf("[+] Debug :: uhuru_notify :: message = %s\n",buffer);

	ret = (*current_handler)(type,buffer);

	free(buffer);
	buffer = NULL;

	return ret;
}



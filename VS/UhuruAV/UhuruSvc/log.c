#include "log.h"
#include <windows.h>

int gfirst_time = 1;

void uhLog(const char *fmt, ... )
{
	//static int first_time = 1;
	FILE * f = NULL;
	const char *mode = "a";
	errno_t err;
	va_list ap;

	if (gfirst_time) {
		mode = "w";
		gfirst_time = 0;
	}
	err = fopen_s(&f,"C:\\UHURU.TXT", mode);
	if (err != 0)
		return;

	va_start(ap, fmt);
	vfprintf(f, fmt, ap);
	va_end(ap);
	fclose(f);

	return;
}
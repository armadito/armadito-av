#include "fanotify.h"

#include <glib.h>

int main(int argc, char **argv)
{
	struct watchd *w;
	GMainLoop * loop;

	w = watchd_new();

	while(--argc)
		watchd_add(w, *++argv);

	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);

	watchd_free(w);

	return 0;
}

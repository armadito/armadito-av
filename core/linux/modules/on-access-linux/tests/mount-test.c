/* compile with:
   gcc $(pkg-config --cflags glib-2.0 gio-2.0 gio-unix-2.0) t.c mount.c -o mount-test $(pkg-config --libs glib-2.0 gio-2.0 gio-unix-2.0) -DDEBUG
*/
#include "mount.h"

#include <stdio.h>
#include <gio/gio.h>

static const char *event_type_str(enum mount_event_type ev_type)
{
	switch(ev_type) {
	case EVENT_MOUNT: return "MOUNT";
	case EVENT_UMOUNT: return "UNMOUNT";
	default: return "UNKNOWN";
	}

	return "ZOB";
}

static void test_cb(enum mount_event_type ev_type, const char *path, void *user_data)
{
	printf("test_cb: event type %s path %s\n", event_type_str(ev_type), path);
}

static gpointer dbus_thread_fun(gpointer data)
{
	struct mount_monitor *m;
	GMainLoop *loop;

	m = mount_monitor_new(test_cb, NULL);

	loop = g_main_loop_new(NULL, FALSE);

	g_main_loop_run(loop);
}

int main(int argc, char **argv)
{
	GThread *dbus_thr = g_thread_new("dbus", dbus_thread_fun, NULL);

	g_thread_join(dbus_thr);

	return 0;
}

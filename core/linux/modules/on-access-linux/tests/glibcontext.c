/* From: http://osdir.com/ml/gtk-app-devel-list/2010-03/msg00081.html */

#include <glib.h>
#include <unistd.h>
#include <sys/syscall.h>

GMainContext *thread1_context, *thread2_context;
GMainLoop *main_loop1, *main_loop2;

gboolean idle_callback(gpointer data)
{
  g_print("idle_callback() %d\n", (pid_t) syscall (SYS_gettid));
  return FALSE;
}

gboolean input_callback(GIOChannel *source,
			GIOCondition condition,
			gpointer data)
{
  GSource *idle_source;
  gchar buffer[16];
  gsize bytes_read;
  g_print("input_callback() %d\n", (pid_t) syscall (SYS_gettid));

  if(g_io_channel_read_chars(source, buffer, 1, &bytes_read, NULL) !=
     G_IO_STATUS_NORMAL)
    return FALSE;

  g_print(" bytes_read = %d\n", (int) bytes_read);

  idle_source = g_idle_source_new();
  g_source_set_callback(idle_source, idle_callback, NULL, NULL);
  g_source_attach(idle_source, thread1_context);
  g_source_unref(idle_source);
  return TRUE;
}

gpointer thread2_entry(gpointer data)
{
  GIOChannel *channel;
  GSource *source;
  g_print("thread2_entry() %d\n", (pid_t) syscall (SYS_gettid));

  main_loop2 = g_main_loop_new(thread2_context, FALSE);

  channel = g_io_channel_unix_new(0);

  source = g_io_create_watch(channel, G_IO_IN);

  g_source_set_callback(source, (GSourceFunc) input_callback, NULL, NULL);

  g_source_attach(source, thread2_context);

  g_source_unref(source);

  g_main_loop_run(main_loop2);
}

int main(int argc, char **argv)
{
  g_thread_init(NULL);

  thread1_context = g_main_context_default();
  thread2_context = g_main_context_new();

  main_loop1 = g_main_loop_new(thread1_context, FALSE);

  g_thread_create(thread2_entry, NULL, FALSE, NULL);

  g_main_loop_run(main_loop1);
  return 0;
}

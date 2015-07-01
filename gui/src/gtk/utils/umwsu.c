#include "utils/umwsu.h"

#include <glib.h>

static struct umwsu *unique_umwsu = NULL;

static GThread *umwsu_open_thread = NULL;

static gpointer umwsu_open_thread_func(gpointer data)
{
  struct umwsu *u;

  u = umwsu_open(1);
  umwsu_print(u);

  return u;
}

struct umwsu *umwsu_handle(void)
{
  if (unique_umwsu == NULL) {
    unique_umwsu = (struct umwsu *)g_thread_join(umwsu_open_thread);
  }

  return unique_umwsu;
}

void umwsu_open_thread_start(void)
{
#if 0
  umwsu_open_thread = g_thread_new("UMWSU open", umwsu_open_thread_func, NULL);
#endif
#if 1
  struct umwsu *u;

  u = umwsu_open(1);
  umwsu_print(u);
  unique_umwsu = u;
#endif
}

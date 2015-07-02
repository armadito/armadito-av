#include "utils/uhuru.h"

#include <glib.h>

static struct uhuru *unique_uhuru = NULL;

static GThread *uhuru_open_thread = NULL;

static gpointer uhuru_open_thread_func(gpointer data)
{
  struct uhuru *u;

  u = uhuru_open(1);
  uhuru_print(u);

  return u;
}

struct uhuru *uhuru_handle(void)
{
  if (unique_uhuru == NULL) {
    unique_uhuru = (struct uhuru *)g_thread_join(uhuru_open_thread);
  }

  return unique_uhuru;
}

void uhuru_open_thread_start(void)
{
#if 0
  uhuru_open_thread = g_thread_new("UHURU open", uhuru_open_thread_func, NULL);
#endif
#if 1
  struct uhuru *u;

  u = uhuru_open(1);
  uhuru_print(u);
  unique_uhuru = u;
#endif
}

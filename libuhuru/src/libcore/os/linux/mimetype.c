#include "libuhuru-config.h"

#include "os/mimetype.h"

#include <glib.h>
#include <magic.h>

/* Unfortunately, libmagic is not thread-safe. */
/* We create a new magic_t for each thread, and keep it  */
/* in thread's private data, so that it is created only once. */

static int init_done = 0;
static GPrivate *private_key;

static void magic_destroy_notify(gpointer data)
{
  magic_close((magic_t)data);
}

void os_mime_type_init(void)
{
  if (!init_done) {
    private_key = g_private_new(magic_destroy_notify);
    init_done = 1;
  }
}

static magic_t get_private_magic(void)
{
  magic_t m;

  m = (magic_t)g_private_get(private_key);

  if (m == NULL) {
    m = magic_open(MAGIC_MIME_TYPE);
    magic_load(m, NULL);

    g_private_set(private_key, (gpointer)m);
  }

  return m;
}

const char *os_mime_type_guess(const char *path)
{
  magic_t m;
  const char *mime_type;

  m = get_private_magic();

  mime_type = magic_file(m, path);

  return mime_type;
}

#include "libuhuru-config.h"

#include "os/mimetype.h"

#include <glib.h>
#include <magic.h>

/* Unfortunately, libmagic is not thread-safe. */
/* We create a new magic_t for each thread, and keep it  */
/* in thread's private data, so that it is created only once. */
static void magic_destroy_notify(gpointer data)
{
  magic_close((magic_t)data);
}

static magic_t get_private_magic(struct uhuru_scan *scan)
{
  magic_t m = (magic_t)g_private_get(scan->local.private_magic_key);

  if (m == NULL) {
    m = magic_open(MAGIC_MIME_TYPE);
    magic_load(m, NULL);

    g_private_set(scan->local.private_magic_key, (gpointer)m);
  }

  return m;
}

/* somewhere */
/* scan->local.private_magic_key = g_private_new(magic_destroy_notify); */
  /* GPrivate *private_magic_key; */

const char *mime_type_guess(const char *path)
{
  const char *mime_type;

  mime_type = magic_file(magic, path);

  return mime_type;
}

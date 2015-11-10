#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include <assert.h>
#include <glib.h>
#include <stdlib.h>

uhuru_error *uhuru_error_new(int error_code, const char *error_message)
{
  uhuru_error *e = (uhuru_error *)malloc(sizeof(uhuru_error));

  assert(e != NULL);

  e->error_code = error_code;
  e->error_message = error_message;

  return e;
}

void uhuru_error_set(uhuru_error **error, int error_code, const char *error_message)
{
  if (error == NULL)
    return;

  /* same check as in glib: if error location is already set, do not overwrite it!!! */
  if (*error != NULL) {
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "uhuru_error set over the top of a previous uhuru_error or uninitialized memory.\n\
This indicates a bug in someone's code. You must ensure an error is NULL before it's set.\n\
The overwriting error message was: %s", error_message);

    return;
  }

  *error = uhuru_error_new(error_code, error_message);
}

void uhuru_error_free(uhuru_error *err)
{
  if (err != NULL)
    free(err);
}

void uhuru_error_print(uhuru_error *err, FILE *out)
{
  if (err == NULL)
    return;

  fprintf(out, "** Uhuru: ERROR: %s\n", err->error_message);
}

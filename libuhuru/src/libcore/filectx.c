#include "libuhuru-config.h"
#include <libuhuru/core.h>

#include "os/string.h"
#include "os/io.h"
#include "os/mimetype.h"

#include <stdlib.h>

const char *uhuru_file_context_status_str(enum uhuru_file_context_status status)
{
  switch(status) {
#undef M
#define M(S) case S: return #S
    M(UHURU_FC_MUST_SCAN);
    M(UHURU_FC_WHITE_LISTED_DIRECTORY);
    M(UHURU_FC_FILE_TOO_BIG);
    M(UHURU_FC_FILE_CACHED);
    M(UHURU_FC_FILE_TYPE_NOT_SCANNED);
  }

  return "UNKNOWN STATUS";
}

/* beware: ctx is filled *only* if file must be scanned, otherwise it is left un-initialized, except for the status field */
/* returns 0 if file must be scanned, !0 otherwise */
enum uhuru_file_context_status uhuru_file_context_get(struct uhuru_file_context *ctx, int fd, const char *path, struct uhuru_scan_conf *conf)
{
  struct uhuru_module **applicable_modules;
  const char *mime_type;

  ctx->status = UHURU_FC_MUST_SCAN;
  ctx->fd = fd;
  ctx->path = NULL;
  ctx->mime_type = NULL;
  ctx->applicable_modules = NULL;

  /* 1) check file name vs. directories white list */
  if (uhuru_scan_conf_is_white_listed(conf, path)) {
    ctx->status = UHURU_FC_WHITE_LISTED_DIRECTORY;
    return ctx->status;
  }
  
  /* 2) cache => NOT YET */

  /* open file if no fd given */
  if (ctx->fd == -1) {
	
    /* open the file */
    ctx->fd = os_open(path, O_RDONLY);
    if (ctx->fd < 0) {
      ctx->status = UHURU_FC_FILE_OPEN_ERROR;
      return ctx->status;
    }
  }

  /* 3) fstat file descriptor and get file size */
  /* not yet */

  /* 4) file type using mime_type_guess and applicable modules from configuration */
  mime_type = os_mime_type_guess_fd(ctx->fd);
  applicable_modules = uhuru_scan_conf_get_applicable_modules(conf, mime_type);

  if (applicable_modules == NULL) {
    free((void *)mime_type);
    
    ctx->status = UHURU_FC_FILE_TYPE_NOT_SCANNED;
    return ctx->status;
  }

  ctx->status = UHURU_FC_MUST_SCAN;
  ctx->path = os_strdup(path);
  ctx->mime_type = mime_type;
  ctx->applicable_modules = applicable_modules;

  return ctx->status;
}

struct uhuru_file_context *uhuru_file_context_clone(struct uhuru_file_context *ctx)
{
  struct uhuru_file_context *clone_ctx = malloc(sizeof(struct uhuru_file_context));

  clone_ctx->status = ctx->status;
  clone_ctx->fd = ctx->fd;
  clone_ctx->path = ctx->path;
  clone_ctx->mime_type = ctx->mime_type;
  clone_ctx->applicable_modules = ctx->applicable_modules;  

  return clone_ctx;
}

void uhuru_file_context_close(struct uhuru_file_context *ctx)
{
  if (ctx->fd > 0)
    os_close(ctx->fd);
}

void uhuru_file_context_destroy(struct uhuru_file_context *ctx)
{
  uhuru_file_context_close(ctx);

  if (ctx->path != NULL)
    free((void *)ctx->path);
  if (ctx->mime_type != NULL)
    free((void *)ctx->mime_type);
}

void uhuru_file_context_free(struct uhuru_file_context *ctx)
{
  uhuru_file_context_destroy(ctx);

  free(ctx);
}


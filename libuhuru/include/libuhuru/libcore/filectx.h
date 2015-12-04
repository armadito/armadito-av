#ifndef _LIBUHURU_LIBCORE_FILECTX_H_
#define _LIBUHURU_LIBCORE_FILECTX_H_

enum uhuru_file_context_status {
  UHURU_FC_MUST_SCAN = 0,                     /* !< file must be scanned                              */
  UHURU_FC_WHITE_LISTED_DIRECTORY,            /* !< a directory ancestor of path is white listed      */
  UHURU_FC_FILE_TOO_BIG,                      /* !< file size is >= maximum file size                 */
  UHURU_FC_FILE_CACHED,                       /* !< file path is in path cache                        */
  UHURU_FC_FILE_TYPE_NOT_SCANNED,             /* !< file mime type has no associated scan modules     */
  UHURU_FC_FILE_OPEN_ERROR,                   /* !< error when opening the file                      */
};

const char *uhuru_file_context_status_str(enum uhuru_file_context_status status);

struct uhuru_file_context {
  enum uhuru_file_context_status status;
  int fd;
  const char *path;
  const char *mime_type;
  struct uhuru_module **applicable_modules;  
};

enum uhuru_file_context_status uhuru_file_context_get(struct uhuru_file_context *ctx, int fd, const char *path, struct uhuru_scan_conf *conf);

struct uhuru_file_context *uhuru_file_context_clone(struct uhuru_file_context *ctx);

void uhuru_file_context_close(struct uhuru_file_context *ctx);

void uhuru_file_context_destroy(struct uhuru_file_context *ctx);

void uhuru_file_context_free(struct uhuru_file_context *ctx);

#endif

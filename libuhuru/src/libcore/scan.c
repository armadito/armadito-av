#include <libuhuru/core.h>
#include "libuhuru-config.h"

#include "statusp.h"
#include "reportp.h"
#include "scanp.h"
#include "uhurup.h"
#ifdef HAVE_ALERT_MODULE
#include "builtin-modules/alert.h"
#endif
#ifdef HAVE_QUARANTINE_MODULE
#include "builtin-modules/quarantine.h"
#endif
#include "os/string.h"
#include "os/mimetype.h"

#include <errno.h>
#include <glib.h>
#include <stdlib.h>

struct callback_entry {
  uhuru_scan_callback_t callback;
  void *callback_data;
};

/* later, these modules will be loaded dynamically and modules will export  */
/* a 'post_scan_fun' that will be called automatically after scanning a file */
/* so that there will be no need to add the callbacks by hand */
static void uhuru_scan_add_builtin_callbacks(struct uhuru_scan *scan, struct uhuru *uhuru)
{
#ifdef HAVE_ALERT_MODULE
  struct uhuru_module *alert_module;
#endif
#ifdef HAVE_QUARANTINE_MODULE
  struct uhuru_module *quarantine_module;
#endif

#ifdef HAVE_ALERT_MODULE
  alert_module = uhuru_get_module_by_name(uhuru, "alert");
  uhuru_scan_add_callback(scan, alert_callback, alert_module->data);
#endif

#ifdef HAVE_QUARANTINE_MODULE
  quarantine_module = uhuru_get_module_by_name(uhuru, "quarantine");
  uhuru_scan_add_callback(scan, quarantine_callback, quarantine_module->data);
#endif
}

struct uhuru_scan *uhuru_scan_new(struct uhuru *uhuru, int scan_id)
{
  struct uhuru_scan *scan = (struct uhuru_scan *)malloc(sizeof(struct uhuru_scan));

  /* use a GArray, and not a GPtrArray, because GArray can contain structs instead of pointers to struct */
  scan->callbacks = g_array_new(FALSE, FALSE, sizeof(struct callback_entry));
  uhuru_scan_add_builtin_callbacks(scan, uhuru);

  scan->scan_id = scan_id;

  scan->to_scan_count = 0;
  scan->scanned_count = 0;

  return scan;
}

void uhuru_scan_add_callback(struct uhuru_scan *scan, uhuru_scan_callback_t callback, void *callback_data)
{
  struct callback_entry entry;

  entry.callback = callback;
  entry.callback_data = callback_data;

  /* this function copies the structure that is passed as argument */
  g_array_append_val(scan->callbacks, entry);
}

/* call all the registered callbacks */
void uhuru_scan_call_callbacks(struct uhuru_scan *scan, struct uhuru_report *report)
{
  int i;

  /* just iterate over the array of structures */
  for(i = 0; i < scan->callbacks->len; i++) {
    struct callback_entry *entry = &g_array_index(scan->callbacks, struct callback_entry, i);
    uhuru_scan_callback_t callback = entry->callback;

    (*callback)(report, entry->callback_data);
  }
}

/* apply the modules contained in 'modules' in order to compute the scan status of 'path' */
/* 'modules' is a NULL-terminated array of pointers to struct uhuru_module */
/* 'mime_type' is the mime-type of the file */
static enum uhuru_file_status scan_apply_modules(int fd, const char *path, const char *mime_type, struct uhuru_module **modules,  struct uhuru_report *report)
{
  enum uhuru_file_status current_status = UHURU_UNDECIDED;

  /* iterate over the modules */
  for (; *modules != NULL; modules++) {
    struct uhuru_module *mod = *modules;
    enum uhuru_file_status mod_status;
    char *mod_report = NULL;

    uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_DEBUG, "scanning fd %d path %s with module %s", fd, path, mod->name);

    /* if module status is not OK, don't call it */
    if (mod->status != UHURU_MOD_OK)
      continue;

    /* call the scan function of the module */
    /* but, after rewinding the file !!! */
    if (lseek(fd, 0, SEEK_SET) < 0)  {
      uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_WARNING, "cannot seek on file %s (error %s)", path, os_strerror(errno));
      return UHURU_IERROR;
    }

    mod_status = (*mod->scan_fun)(mod, fd, path, mime_type, &mod_report);

    /* then compare the status that was returned by the module with current status */
    /* if current status is 'less than' (see status.c for comparison meaning) */
    /* adopt the module returned status as current status */
    /* for instance, if current_status is UNDECIDED and module status is MALWARE, */
    /* current_status become MALWARE */
    if (uhuru_file_status_cmp(current_status, mod_status) < 0) {
      current_status = mod_status;
      if (report != NULL)
	uhuru_report_change(report, mod_status, (char *)mod->name, mod_report);
    } else if (mod_report != NULL)
      free(mod_report);

    /* if module has returned an authoritative status, no need to go on */
    if (current_status == UHURU_WHITE_LISTED || current_status == UHURU_MALWARE)
      break;
  }

  return current_status;
}

static void scan_progress(struct uhuru_scan *scan, struct uhuru_report *report)
{
  int progress;

  if (scan->to_scan_count == 0) {
    report->progress = REPORT_PROGRESS_UNKNOWN;
    return;
  }

  /* update the progress */
  /* may be not thread safe, but who cares about precise values? */
  scan->scanned_count++;

  progress = (int)((100.0 * scan->scanned_count) / scan->to_scan_count);

  if (progress > 100)
    progress = 100;

  report->progress = progress;
}

/* scan a file context: */
/* - apply the modules to scan the file */
/* - and call the callbacks  */
enum uhuru_file_status uhuru_scan_context(struct uhuru_scan *scan, struct uhuru_file_context *ctx)
{
  enum uhuru_file_status status;
  struct uhuru_report report;

  uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_DEBUG, "scanning file %s", ctx->path);

  /* initializes the structure passed to callbacks */
  uhuru_report_init(&report, scan->scan_id, ctx->path, REPORT_PROGRESS_UNKNOWN);

  /* if no modules apply, then file is not handled */
  if (ctx->applicable_modules == NULL || ctx->mime_type == NULL) {
    status = UHURU_UNKNOWN_FILE_TYPE;
    uhuru_report_change(&report, status, NULL, NULL);
  } else {
    /* otherwise we scan it by applying the modules */
    status = scan_apply_modules(ctx->fd, ctx->path, ctx->mime_type, ctx->applicable_modules, &report);
  }

  /* compute progress if have one */
  scan_progress(scan, &report);

  /* once done, call the callbacks */
  uhuru_scan_call_callbacks(scan, &report);

  /* and free the report (it may contain a strdup'ed string) */
  uhuru_report_destroy(&report);

  return status;
}

/* just free the structure */
void uhuru_scan_free(struct uhuru_scan *scan)
{
  g_array_free(scan->callbacks, TRUE);

  free(scan);
}


/*
 * **************************************************
 *
 * Maintained for now for compatibility with windows service
 *
 * **************************************************
 */


/* the simple version, for on-access scan: */
/* no callbacks */
/* no threads */
enum uhuru_file_status uhuru_scan_simple(struct uhuru *uhuru, const char *path, struct uhuru_report *report)
{
  struct uhuru_module **modules = NULL;
  const char *mime_type;
  enum uhuru_file_status status;
  int fd = -1; /* ??? */

  if (os_file_do_not_scan(path))
    return UHURU_CLEAN;

  // Initialize the scan report structure.
  if (report != NULL)
    uhuru_report_init(report, 1, path, REPORT_PROGRESS_UNKNOWN);

  /* find the mime type, as in scan_file fun */
  mime_type = os_mime_type_guess(path);

#if 0
  if (mime_type != NULL)
    modules = uhuru_get_applicable_modules(uhuru, mime_type);
#endif
  
  if (modules == NULL || mime_type == NULL)
    return UHURU_UNKNOWN_FILE_TYPE;

  status = scan_apply_modules(fd, path, mime_type, modules, report);

  free((void *)mime_type);

  return status;
}


#include <libarmadito.h>
#include "libarmadito-config.h"

#include "statusp.h"
#include "reportp.h"
#include "scanp.h"
#include "armaditop.h"
#include "os/string.h"
#include "os/io.h"
#include "os/file.h"
#include "os/mimetype.h"

#ifdef HAVE_ALERT_MODULE
#include "builtin-modules/alert.h"
#endif
#ifdef HAVE_QUARANTINE_MODULE
#include "builtin-modules/quarantine.h"
#endif
#ifdef HAVE_ON_ACCESS_WINDOWS_MODULE
#include "builtin-modules/onaccess_windows.h"
#endif

#include <errno.h>
#include <glib.h>
#include <stdlib.h>

struct callback_entry {
	a6o_scan_callback_t callback;
	void *callback_data;
};

/* later, these modules will be loaded dynamically and modules will export  */
/* a 'post_scan_fun' that will be called automatically after scanning a file */
/* so that there will be no need to add the callbacks by hand */
static void a6o_scan_add_builtin_callbacks(struct a6o_scan *scan, struct armadito *armadito)
{
#ifdef HAVE_ALERT_MODULE
	struct a6o_module *alert_module;
#endif
#ifdef HAVE_QUARANTINE_MODULE
	struct a6o_module *quarantine_module;
#endif

#ifdef HAVE_ALERT_MODULE
	alert_module = a6o_get_module_by_name(armadito, "alert");
	a6o_scan_add_callback(scan, alert_callback, alert_module->data);
#endif

#ifdef HAVE_QUARANTINE_MODULE
	quarantine_module = a6o_get_module_by_name(armadito, "quarantine");
	a6o_scan_add_callback(scan, quarantine_callback, quarantine_module->data);
#endif

}

struct a6o_scan *a6o_scan_new(struct armadito *armadito, int scan_id)
{
	struct a6o_scan *scan = (struct a6o_scan *)malloc(sizeof(struct a6o_scan));

	/* use a GArray, and not a GPtrArray, because GArray can contain structs instead of pointers to struct */
	scan->callbacks = g_array_new(0, 0, sizeof(struct callback_entry));
	a6o_scan_add_builtin_callbacks(scan, armadito);

	scan->scan_id = scan_id;

	scan->to_scan_count = 0;
	scan->scanned_count = 0;
        scan->malware_count = 0;
        scan->suspicious_count = 0;
        

	return scan;
}

void a6o_scan_add_callback(struct a6o_scan *scan, a6o_scan_callback_t callback, void *callback_data)
{
	struct callback_entry entry;

	entry.callback = callback;
	entry.callback_data = callback_data;

	/* this function copies the structure that is passed as argument */
	g_array_append_val(scan->callbacks, entry);
}

/* call all the registered callbacks */
void a6o_scan_call_callbacks(struct a6o_scan *scan, struct a6o_report *report)
{
	int i;

	/* just iterate over the array of structures */
	for(i = 0; i < scan->callbacks->len; i++) {
		struct callback_entry *entry = &g_array_index(scan->callbacks, struct callback_entry, i);
		a6o_scan_callback_t callback = entry->callback;

		(*callback)(report, entry->callback_data);
	}
}

/* apply the modules contained in 'modules' in order to compute the scan status of 'path' */
/* 'modules' is a NULL-terminated array of pointers to struct a6o_module */
/* 'mime_type' is the mime-type of the file */
static enum a6o_file_status scan_apply_modules(int fd, const char *path, const char *mime_type, struct a6o_module **modules,  struct a6o_report *report)
{
	enum a6o_file_status current_status = ARMADITO_UNDECIDED;

	/* iterate over the modules */
	for (; *modules != NULL; modules++) {
		struct a6o_module *mod = *modules;
		enum a6o_file_status mod_status;
		char *mod_report = NULL;

		//a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_DEBUG, "scanning fd %d path %s with module %s", fd, path, mod->name);

		/* if module status is not OK, don't call it */
		if (mod->status != ARMADITO_MOD_OK)
			continue;

		/* call the scan function of the module */
		/* but, after rewinding the file !!! */
		if (os_lseek(fd, 0, SEEK_SET) < 0)  {
			a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "cannot seek on file %s (error %s)", path, os_strerror(errno) );
			return ARMADITO_IERROR;
		}

		mod_status = (*mod->scan_fun)(mod, fd, path, mime_type, &mod_report);

		/* then compare the status that was returned by the module with current status */
		/* if current status is 'less than' (see status.c for comparison meaning) */
		/* adopt the module returned status as current status */
		/* for instance, if current_status is UNDECIDED and module status is MALWARE, */
		/* current_status become MALWARE */
		if (a6o_file_status_cmp(current_status, mod_status) < 0) {
			current_status = mod_status;
			if (report != NULL)
				a6o_report_change(report, mod_status, (char *)mod->name, mod_report);
		} else if (mod_report != NULL)
			free(mod_report);

		/* if module has returned an authoritative status, no need to go on */
		if (current_status == ARMADITO_WHITE_LISTED || current_status == ARMADITO_MALWARE)
			break;
	}

	return current_status;
}

static void scan_progress(struct a6o_scan *scan, struct a6o_report *report)
{
	int progress;

	/* update the progress */
	/* may be not thread safe, but who cares about precise values? */
	scan->scanned_count++;

	if (scan->to_scan_count == 0) {
		report->progress = REPORT_PROGRESS_UNKNOWN;
		report->scanned_count = scan->scanned_count;
		return;
	}

	progress = (int)((100.0 * scan->scanned_count) / scan->to_scan_count);

        // a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "Progress = %d = ( 100 * %d / %d ) ", progress, scan->scanned_count, scan->to_scan_count );

	if (progress > 100)
		progress = 100;

	report->progress = progress;
	report->scanned_count = scan->scanned_count;
}

static void update_counters (struct a6o_scan *scan, struct a6o_report *report, enum a6o_file_status status )
{
       switch(status) {
	case ARMADITO_UNDECIDED: break;
	case ARMADITO_CLEAN: break;
	case ARMADITO_UNKNOWN_FILE_TYPE: break;
	case ARMADITO_EINVAL: break;
	case ARMADITO_IERROR: break;
	case ARMADITO_SUSPICIOUS:
		scan->suspicious_count++;
		break;
	case ARMADITO_WHITE_LISTED: break;
	case ARMADITO_MALWARE:
		scan->malware_count++;
		break;
      }

      report->suspicious_count = scan->suspicious_count;	
      report->malware_count = scan->malware_count;
}

/* scan a file context: */
/* - apply the modules to scan the file */
/* - and call the callbacks  */
enum a6o_file_status a6o_scan_context(struct a6o_scan *scan, struct a6o_file_context *ctx)
{
	enum a6o_file_status status;
	struct a6o_report report;

	//a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_DEBUG, "scanning file %s", ctx->path);

	/* initializes the structure passed to callbacks */
	a6o_report_init(&report, scan->scan_id, ctx->path, REPORT_PROGRESS_UNKNOWN);

	/* if no modules apply, then file is not handled */
	if (ctx->applicable_modules == NULL || ctx->mime_type == NULL) {
		status = ARMADITO_UNKNOWN_FILE_TYPE;
		a6o_report_change(&report, status, NULL, NULL);
	} else {
		/* otherwise we scan it by applying the modules */
		status = scan_apply_modules(ctx->fd, ctx->path, ctx->mime_type, ctx->applicable_modules, &report);
	}

	/* compute progress if have one */
	scan_progress(scan, &report);

	update_counters(scan, &report, status);

	/* once done, call the callbacks */
	a6o_scan_call_callbacks(scan, &report);

	/* and free the report (it may contain a strdup'ed string) */
	a6o_report_destroy(&report);

	return status;
}

/* just free the structure */
void a6o_scan_free(struct a6o_scan *scan)
{
	g_array_free(scan->callbacks, 1);

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
enum a6o_file_status a6o_scan_simple_old(struct armadito *armadito, const char *path, struct a6o_report *report)
{
	struct a6o_module **modules = NULL;
	const char *mime_type;
	enum a6o_file_status status;
	int fd = -1; /* ??? */

	if (os_file_do_not_scan(path))
		return ARMADITO_CLEAN;

	// Initialize the scan report structure.
	if (report != NULL)
		a6o_report_init(report, 1, path, REPORT_PROGRESS_UNKNOWN);

	/* find the mime type, as in scan_file fun */
	mime_type = os_mime_type_guess(path);

#if 0
	if (mime_type != NULL)
		modules = a6o_get_applicable_modules(armadito, mime_type);
#endif

	if (modules == NULL || mime_type == NULL)
		return ARMADITO_UNKNOWN_FILE_TYPE;

	status = scan_apply_modules(fd, path, mime_type, modules, report);

	free((void *)mime_type);

	return status;
}

enum a6o_file_status a6o_scan_simple(struct armadito *armadito, const char *path, struct a6o_report *report)
{

	int fd = -1;
	struct a6o_file_context file_context;
	struct a6o_scan * scan;
	enum a6o_file_context_status context_status;
	enum a6o_file_status status;

#ifdef _WIN32
	// only for test purpose :: TODO :: separate a6o_scan from callbacks.
	if (report->status == ARMADITO_MALWARE) {
		//printf("[+] Info :: a6o_scan_simple :: %d\n",report->status);
		report->path = path;
		scan = a6o_scan_new(armadito, -1);
		a6o_scan_call_callbacks(scan, report);
		a6o_scan_free(scan);
		return ARMADITO_MALWARE;
	}

#endif

	context_status = a6o_file_context_get(&file_context, fd, path, a6o_scan_conf_on_access());
	//
	//context_status = a6o_file_context_get(&file_context, fd, path, a6o_scan_conf_on_demand());

	if (context_status == ARMADITO_FC_WHITE_LISTED_DIRECTORY)
		return ARMADITO_WHITE_LISTED;

	if (context_status != ARMADITO_FC_MUST_SCAN ) {

		/*if (context_status == ARMADITO_FC_MIME_TYPE_NOT_FOUND)
		  status = ARMADITO_UNKNOWN_FILE_TYPE;
		  else if (context_status == ARMADITO_FC_FILE_OPEN_ERROR)
		  status = ARMADITO_IERROR; // rename this error :: the file could not be opened.
		  else
		  status = ARMADITO_IERROR;
		*/
		//a6o_log( );
		//a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "path = %s  :: context_status = %d", path ,context_status);
		status = ARMADITO_IERROR;
		a6o_file_context_destroy(&file_context);
		return status;
	}

	scan = a6o_scan_new(armadito, -1);

	status = a6o_scan_context(scan, &file_context);

	//a6o_file_context_free(&file_context);
	a6o_file_context_destroy(&file_context);

	a6o_scan_free(scan);

	return status;
}

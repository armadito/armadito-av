/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "armadito-config.h"
#include <libarmadito/armadito.h>

#include "core/scanctx.h"
#include "core/report.h"
#include "core/io.h"
#include "core/mimetype.h"
#include "string_p.h"
#include "status_p.h"

#include <errno.h>
#include <stdlib.h>

const char *a6o_scan_context_status_str(enum a6o_scan_context_status status)
{
	switch(status) {
#undef M
#define M(S) case S: return #S
		M(A6O_SC_MUST_SCAN);
		M(A6O_SC_WHITE_LISTED_DIRECTORY);
		M(A6O_SC_FILE_TOO_BIG);
		M(A6O_SC_FILE_CACHED);
		M(A6O_SC_FILE_TYPE_NOT_SCANNED);
	}

	return "UNKNOWN STATUS";
}

/* beware: ctx is filled *only* if file must be scanned, otherwise it is left un-initialized, except for the status field */
/* returns 0 if file must be scanned, !0 otherwise */
enum a6o_scan_context_status a6o_scan_context_get(struct a6o_scan_context *ctx, int fd, const char *path, struct a6o_scan_conf *conf, struct a6o_report *report)
{
	struct a6o_module **applicable_modules;
	const char *mime_type;
	int err = 0;

	/* initializes the structure */
	a6o_report_init(report, path);

	if (fd < 0 && path == NULL) {
		ctx->status = A6O_SC_FILE_OPEN_ERROR;
		a6o_report_change(report, A6O_FILE_IERROR, NULL, NULL);
		return ctx->status;
	}

	ctx->status = A6O_SC_MUST_SCAN;
	ctx->fd = fd;
	ctx->path = NULL;
	ctx->mime_type = NULL;
	ctx->applicable_modules = NULL;

	/* 1) check file name vs. directories white list */
	if (path != NULL && a6o_scan_conf_is_white_listed(conf, path)) {
		ctx->status = A6O_SC_WHITE_LISTED_DIRECTORY;
		a6o_report_change(report, A6O_FILE_WHITE_LISTED, NULL, NULL);
		return ctx->status;
	}

	/* 2) cache => NOT YET */

	/* open file if no fd given */
	if (ctx->fd < 0) {

#ifdef _WIN32
		/* TODO write portable code for this function */
		err = _sopen_s(&(ctx->fd), path, O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IREAD);
#else
		ctx->fd = os_open(path, O_RDONLY);
#endif

		if (ctx->fd < 0) {
			ctx->status = A6O_SC_FILE_OPEN_ERROR;
			a6o_report_change(report, A6O_FILE_IERROR, NULL, NULL);
			a6o_log(A6O_LOG_LIB,A6O_LOG_LEVEL_WARNING, " Error :: a6o_scan_context_get :: Opening file [%s] for scan failed :: err = %d\n",path,err);
			return ctx->status;
		}
	}

	/* 3) fstat file descriptor and get file size */
	/* not yet */

	/* 4) file type using mime_type_guess and applicable modules from configuration */
	mime_type = os_mime_type_guess_fd(ctx->fd);
	if (mime_type == NULL) {
		a6o_report_change(report, A6O_FILE_UNKNOWN_TYPE, NULL, NULL);
		ctx->status = A6O_SC_FILE_TYPE_NOT_SCANNED;
		return ctx->status;
	}

	applicable_modules = a6o_scan_conf_get_applicable_modules(conf, mime_type);

	if (applicable_modules == NULL) {
		free((void *)mime_type);
		a6o_report_change(report, A6O_FILE_UNKNOWN_TYPE, NULL, NULL);
		ctx->status = A6O_SC_FILE_TYPE_NOT_SCANNED;

		return ctx->status;
	}

	if(path != NULL)
		ctx->path = os_strdup(path);

	ctx->status = A6O_SC_MUST_SCAN;
	ctx->mime_type = mime_type;
	ctx->applicable_modules = applicable_modules;

	return ctx->status;
}

/* apply the modules contained in 'modules' in order to compute the scan status of 'path' */
/* 'modules' is a NULL-terminated array of pointers to struct a6o_module */
/* 'mime_type' is the mime-type of the file */
static enum a6o_file_status scan_apply_modules(int fd, const char *path, const char *mime_type, struct a6o_module **modules,  struct a6o_report *report)
{
	enum a6o_file_status current_status = A6O_FILE_UNDECIDED;

	/* iterate over the modules */
	for (; *modules != NULL; modules++) {
		struct a6o_module *mod = *modules;
		enum a6o_file_status mod_status;
		char *module_report = NULL;

		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_DEBUG, "scanning fd %d path %s with module %s", fd, path, mod->name);

		/* if module status is not OK, don't call it */
		if (mod->status != A6O_MOD_OK)
			continue;

		/* call the scan function of the module */
		/* but, after rewinding the file !!! */
		if (os_lseek(fd, 0, SEEK_SET) < 0)  {
			a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "cannot seek on file %s (error %s)", path, os_strerror(errno) );
			return A6O_FILE_IERROR;
		}

		mod_status = (*mod->scan_fun)(mod, fd, path, mime_type, &module_report);

		/* then compare the status that was returned by the module with current status */
		/* if current status is 'less than' (see status.c for comparison meaning) */
		/* adopt the module returned status as current status */
		/* for instance, if current_status is UNDECIDED and module status is MALWARE, */
		/* current_status become MALWARE */
		if (a6o_file_status_cmp(current_status, mod_status) < 0) {
			current_status = mod_status;
			if (report != NULL)
				a6o_report_change(report, mod_status, (char *)mod->name, module_report);
		} else if (module_report != NULL)
			free(module_report);

		/* if module has returned an authoritative status, no need to go on */
		if (current_status == A6O_FILE_WHITE_LISTED || current_status == A6O_FILE_MALWARE)
			break;
	}

	return current_status;
}

/* scan a file context: */
/* - apply the modules to scan the file */
enum a6o_file_status a6o_scan_context_scan(struct a6o_scan_context *ctx, struct a6o_report *report)
{
	enum a6o_file_status status;

	a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_DEBUG, "scanning file %s", ctx->path);

	/* if no modules apply, then file is not handled */
	/* this has already been tested in a6o_scan_context_get() */
	/* but safety check may not hurt */
	if (ctx->applicable_modules == NULL || ctx->mime_type == NULL) {
		status = A6O_FILE_UNKNOWN_TYPE;
		a6o_report_change(report, status, NULL, NULL);
		return status;
	}

	/* otherwise we scan it by applying the modules */
	status = scan_apply_modules(ctx->fd, ctx->path, ctx->mime_type, ctx->applicable_modules, report);

	return status;
}

void a6o_scan_context_close(struct a6o_scan_context *ctx)
{
	if (ctx->fd < 0)
		return;

	if (os_close(ctx->fd) != 0)
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "closing file descriptor %3d failed (%s)", ctx->fd, os_strerror(errno));

	ctx->fd = -1;
}

void a6o_scan_context_destroy(struct a6o_scan_context *ctx)
{
	a6o_scan_context_close(ctx);

	if (ctx->path != NULL)
		free((void *)ctx->path);
	if (ctx->mime_type != NULL)
		free((void *)ctx->mime_type);
}


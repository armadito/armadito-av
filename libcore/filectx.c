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

#include "core/filectx.h"
#include "core/io.h"
#include "core/mimetype.h"
#include "string_p.h"

#include <errno.h>
#include <stdlib.h>

const char *a6o_file_context_status_str(enum a6o_file_context_status status)
{
	switch(status) {
#undef M
#define M(S) case S: return #S
		M(ARMADITO_FC_MUST_SCAN);
		M(ARMADITO_FC_WHITE_LISTED_DIRECTORY);
		M(ARMADITO_FC_FILE_TOO_BIG);
		M(ARMADITO_FC_FILE_CACHED);
		M(ARMADITO_FC_FILE_TYPE_NOT_SCANNED);
	}

	return "UNKNOWN STATUS";
}

/* beware: ctx is filled *only* if file must be scanned, otherwise it is left un-initialized, except for the status field */
/* returns 0 if file must be scanned, !0 otherwise */
enum a6o_file_context_status a6o_file_context_get(struct a6o_file_context *ctx, int fd, const char *path, struct a6o_scan_conf *conf)
{
	struct a6o_module **applicable_modules;
	const char *mime_type;
	int err = 0;

	if (fd < 0 && path == NULL) {
		ctx->status = ARMADITO_FC_FILE_OPEN_ERROR;
		return ctx->status;
	}

	ctx->status = ARMADITO_FC_MUST_SCAN;
	ctx->fd = fd;
	ctx->path = NULL;
	ctx->mime_type = NULL;
	ctx->applicable_modules = NULL;

	/* 1) check file name vs. directories white list */
	if (path != NULL && a6o_scan_conf_is_white_listed(conf, path)) {
		ctx->status = ARMADITO_FC_WHITE_LISTED_DIRECTORY;
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
			ctx->status = ARMADITO_FC_FILE_OPEN_ERROR;
			a6o_log(A6O_LOG_LIB,A6O_LOG_LEVEL_WARNING, " Error :: a6o_file_context_get :: Opening file [%s] for scan failed :: err = %d\n",path,err);
			return ctx->status;
		}
	}

	/* 3) fstat file descriptor and get file size */
	/* not yet */

	/* 4) file type using mime_type_guess and applicable modules from configuration */
	mime_type = os_mime_type_guess_fd(ctx->fd);
	if (mime_type == NULL) {
		ctx->status = ARMADITO_FC_FILE_TYPE_NOT_SCANNED;
		return ctx->status;
	}

	applicable_modules = a6o_scan_conf_get_applicable_modules(conf, mime_type);

	if (applicable_modules == NULL) {
		free((void *)mime_type);
		ctx->status = ARMADITO_FC_FILE_TYPE_NOT_SCANNED;

		return ctx->status;
	}

	if(path != NULL)
		ctx->path = os_strdup(path);

	ctx->status = ARMADITO_FC_MUST_SCAN;
	ctx->mime_type = mime_type;
	ctx->applicable_modules = applicable_modules;

	return ctx->status;
}

struct a6o_file_context *a6o_file_context_clone(struct a6o_file_context *ctx)
{
	struct a6o_file_context *clone_ctx = malloc(sizeof(struct a6o_file_context));

	clone_ctx->status = ctx->status;
	clone_ctx->fd = ctx->fd;
	clone_ctx->path = ctx->path;
	clone_ctx->mime_type = ctx->mime_type;
	clone_ctx->applicable_modules = ctx->applicable_modules;

	return clone_ctx;
}

void a6o_file_context_close(struct a6o_file_context *ctx)
{
	if (ctx->fd < 0)
		return;

	if (os_close(ctx->fd) != 0)
		a6o_log(A6O_LOG_LIB, A6O_LOG_LEVEL_WARNING, "closing file descriptor %3d failed (%s)", ctx->fd, os_strerror(errno));

	ctx->fd = -1;
}

void a6o_file_context_destroy(struct a6o_file_context *ctx)
{
	a6o_file_context_close(ctx);

	if (ctx->path != NULL)
		free((void *)ctx->path);
	if (ctx->mime_type != NULL)
		free((void *)ctx->mime_type);
}

void a6o_file_context_free(struct a6o_file_context *ctx)
{
	a6o_file_context_destroy(ctx);

	free(ctx);
}


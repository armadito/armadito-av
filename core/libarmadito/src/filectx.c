#include "libarmadito-config.h"
#include <libarmadito.h>

#include "os/string.h"
#include "os/io.h"
#include "os/mimetype.h"

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

	if (fd == -1 && path == NULL) {
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
		/* open the file :: TODO write portable code for this function */
		err = _sopen_s(&(ctx->fd), path, O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IREAD);
#else
		/* open the file */
		ctx->fd = os_open(path, O_RDONLY);
#endif

		if (ctx->fd < 0) {
			ctx->status = ARMADITO_FC_FILE_OPEN_ERROR;
			a6o_log(ARMADITO_LOG_LIB,ARMADITO_LOG_LEVEL_WARNING, " Error :: a6o_file_context_get :: Opening file [%s] for scan failed :: err = %d\n",path,err);
			printf("[-] Error :: a6o_file_context_get :: Opening file [%s] for scan failed :: err = %d\n",path,err);
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

	ctx->status = ARMADITO_FC_MUST_SCAN;
	ctx->path = os_strdup(path);
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
		a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_WARNING, "closing file descriptor %3d failed (%s)", ctx->fd, os_strerror(errno));

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


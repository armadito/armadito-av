#ifndef _LIBUHURU_HANDLE_H_
#define _LIBUHURU_HANDLE_H_

#ifdef __cplusplus
extern "C" {
#endif

struct uhuru;

struct uhuru *uhuru_open(void);

void uhuru_print(struct uhuru *u);

void uhuru_close(struct uhuru *u);

#ifdef __cplusplus
}
#endif

#endif

#ifndef _MIMETYPE_H_
#define _MIMETYPE_H_

void mime_type_init(void);

const char *mime_type_guess_fd(int fd);

#endif



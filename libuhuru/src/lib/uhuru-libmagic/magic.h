#ifndef _UHURU_LIBMAGIC_H
#define _UHURU_LIBMAGIC_H

// bouchon. 
typedef struct magic_set *magic_t;
magic_t magic_open(int flag);
void magic_close(magic_t cookie);
int magic_load(magic_t cookie, const char * filename);
const char * magic_file( magic_t cookie, const char * filename);


#endif
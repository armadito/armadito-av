#ifndef _UNISTD_H
#define _UNISTD_H

#include <dirent.h>
#include <sys/types.h>
#include <direct.h>
#define ssize_t int

// to implement
ssize_t read(int fd, void * buf, size_t count);
int readdir_r(DIR * dirp, struct dirent * entry, struct dirent ** result);

// in <sys/stat.h>
// modif by ulrich: use mkdir function in direct.h
#define mkdir(path, mode) _mkdir(path)


#endif
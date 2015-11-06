#include "os/dir.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *flag_str(enum os_file_flag flag)
{
#define M(F) case F: return #F
  switch(flag) {
    M(FILE_FLAG_IS_ERROR);
    M(FILE_FLAG_IS_DIRECTORY);
    M(FILE_FLAG_IS_PLAIN_FILE);
    M(FILE_FLAG_IS_LINK);
    M(FILE_FLAG_IS_DEVICE);
    M(FILE_FLAG_IS_IPC);
    M(FILE_FLAG_IS_UNKNOWN);
  }

  return "arghhh";
}

static void test_dirent_cb(const char *full_path, enum os_file_flag flags, int entry_errno, void *data)
{
  printf("path: %s flags: %s errno: %d (%s)\n", full_path, flag_str(flags), entry_errno, strerror(entry_errno));
}

int main(int argc, char **argv)
{
  assert(argc >= 2);

  os_dir_map(argv[1], 1, test_dirent_cb, NULL);
}

#include "lib/os/dir.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *flag_str(enum dir_entry_flag flag)
{
#define M(F) case F: return #F
  switch(flag) {
    M(DIR_ENTRY_IS_ERROR);
    M(DIR_ENTRY_IS_DIRECTORY);
    M(DIR_ENTRY_IS_PLAIN_FILE);
    M(DIR_ENTRY_IS_LINK);
    M(DIR_ENTRY_IS_DEVICE);
    M(DIR_ENTRY_IS_IPC);
    M(DIR_ENTRY_IS_UNKNOWN);
  }

  return "arg";
}

static void test_dirent_cb(const char *full_path, enum dir_entry_flag flags, int entry_errno, void *data)
{
  printf("path: %s flags: %s errno: %d (%s)\n", full_path, flag_str(flags), entry_errno, strerror(entry_errno));
}

int main(int argc, char **argv)
{
  assert(argc >= 2);

  os_dir_map(argv[1], 1, test_dirent_cb, NULL);
}

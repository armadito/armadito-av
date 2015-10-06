#include "dir.h"
#include <stdio.h>

static void print_entry(const char *full_path, void *data)
{
  printf("%s\n", full_path);
}


int main(int argc, char **argv)
{
  os_dir_map(argv[1], 1, print_entry, NULL);
}

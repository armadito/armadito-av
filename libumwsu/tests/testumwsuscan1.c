#include <libumwsu/scan.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  struct umwsu *u = umwsu_open();
  int i;
  
  for (i = 1; i < argc; i++) {
    enum umwsu_status status;

    status = umwsu_scan_file(u, argv[i]);
    printf("%s: %s\n", argv[i], umwsu_status_str(status));
  }

  umwsu_close(u);
}

#include <libumwsu/scan.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  struct umw *u = umw_open();
  int i;
  
  for (i = 1; i < argc; i++) {
    enum umw_status status;

    status = umw_scan_file(u, argv[i]);
    printf("%s: %s\n", argv[i], umw_status_str(status));
  }

  umw_close(u);
}

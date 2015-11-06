#include <libuhuru/core.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  struct uhuru *u = uhuru_open(NULL);
  int i;
  
  for (i = 1; i < argc; i++) {
    enum uhuru_file_status status;

    /* status = uhuru_scan_file(u, argv[i]); */
    printf("%s: %s\n", argv[i], uhuru_file_status_str(status));
  }

  uhuru_close(u, NULL);
}

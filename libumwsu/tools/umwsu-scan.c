#include <libumwsu/scan.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static void usage(int argc, char **argv)
{
  fprintf(stderr, "usage: %s [-r] FILE|DIR ...\n", argv[0]);
  exit(1);
}

int main(int argc, char **argv)
{
  struct umwsu *u;
  int argp = 1;
  int recurse = 0;
  int threaded = 0;
  
  if (argc < 2)
    usage(argc, argv);

  if (!strcmp(argv[argp], "-r")) {
    argp++;
    recurse = 1;
  }

  if (!strcmp(argv[argp], "-t")) {
    argp++;
    threaded = 1;
  }

  u = umwsu_open();

  umwsu_set_verbose(u, 1);

  umwsu_print(u);

  while (argp < argc) {
#if 0
    struct stat sb;
    enum umwsu_status status;

    if (stat(argv[argp], &sb) == -1) {
      perror("stat");
      exit(EXIT_FAILURE);
    }

    if (S_ISDIR(sb.st_mode))
      umwsu_scan_dir(u, argv[argp], recurse, threaded, NULL, NULL);
    else {
      status = umwsu_scan_file(u, NULL, argv[argp], NULL, NULL);
    }
#endif

    argp++;
  }

  umwsu_close(u);
}

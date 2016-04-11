#include <libarmadito.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	struct armadito *u = a6o_open(NULL, NULL);
	int i;

	for (i = 1; i < argc; i++) {
		enum a6o_file_status status;

		/* status = a6o_scan_file(u, argv[i]); */
		printf("%s: %s\n", argv[i], a6o_file_status_str(status));
	}

	a6o_close(u, NULL);
}

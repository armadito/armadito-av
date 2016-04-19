#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static dev_t get_dev_id(const char *path)
{
	struct stat buf;

	if (stat(path, &buf) < 0) {
		fprintf(stderr, "stat() failed (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return buf.st_dev;
}

static void test(const char *path, dev_t slash_id)
{
	dev_t dev_id;

	dev_id = get_dev_id(path);

	printf("%s (%d) vs / (%d): %s\n", path, (unsigned int)dev_id, (unsigned int)slash_id, dev_id == slash_id ? "same device" : "different device");
}

int main(int argc, char **argv)
{
	dev_t slash_id;
	int i;

	slash_id = get_dev_id("/");

	for (i = 1; i < argc; i++)
		test(argv[i], slash_id);

	return 0;
}

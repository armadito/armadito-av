#include "libarmadito-config.h"

#include "daemon/ipc.h"
#include "net/unixsockclient.h"
#include "net/netdefaults.h"

#include <assert.h>
#include <getopt.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_NAME "armadito-scan"
#define PROGRAM_VERSION PACKAGE_VERSION

struct scan_summary {
	int scanned;
	int malware;
	int suspicious;
	int unhandled;
	int clean;
};

struct scan {
	struct scan_summary *summary;
	int print_clean;
};

struct scan_options {
	const char *unix_path;
	int recursive;
	int threaded;
	int no_summary;
	int print_clean;
	const char *path_to_scan;
};

static struct option scan_option_defs[] = {
	{"help",        no_argument,        0, 'h'},
	{"version",     no_argument,        0, 'V'},
	{"recursive",   no_argument,        0, 'r'},
	{"threaded",    no_argument,        0, 't'},
	{"no-summary",  no_argument,        0, 'n'},
	{"print-clean", no_argument,        0, 'c'},
	{"path",        required_argument,  0, 'a'},
	{0, 0, 0, 0}
};

static void version(void)
{
	printf("%s %s\n", PROGRAM_NAME, PROGRAM_VERSION);
	exit(EXIT_SUCCESS);
}

static void usage(void)
{
	fprintf(stderr, "usage: armadito-scan [options] FILE|DIR\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Armadito antivirus scanner\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  --help  -h                    print help and quit\n");
	fprintf(stderr, "  --version -V                  print program version\n");
	fprintf(stderr, "  --recursive  -r               scan directories recursively\n");
	fprintf(stderr, "  --threaded -t                 scan using multiple threads\n");
	fprintf(stderr, "  --no-summary -n               disable summary at end of scanning\n");
	fprintf(stderr, "  --print-clean -c              print also clean files as they are scanned\n");
	fprintf(stderr, "  --path=PATH | -a PATH         unix socket path (default is " DEFAULT_SOCKET_PATH ")\n");
	fprintf(stderr, "\n");

	exit(1);
}

static void parse_options(int argc, char **argv, struct scan_options *opts)
{
	opts->recursive = 0;
	opts->threaded = 0;
	opts->no_summary = 0;
	opts->print_clean = 0;
	opts->unix_path = DEFAULT_SOCKET_PATH;
	opts->path_to_scan = NULL;

	while (1) {
		int c, option_index = 0;

		c = getopt_long(argc, argv, "hVrtnca:", scan_option_defs, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 'h': /* help */
			usage();
			break;
		case 'V': /* version */
			version();
			break;
		case 'r': /* recursive */
			opts->recursive = 1;
			break;
		case 't': /* threaded */
			opts->threaded = 1;
			break;
		case 'n': /* no-summary */
			opts->no_summary = 1;
			break;
		case 'c': /* print-clean */
			opts->print_clean = 1;
			break;
		case 'a': /* path */
			opts->unix_path = strdup(optarg);
			break;
		case '?':
			/* getopt_long already printed an error message. */
			break;
		default:
			abort ();
		}
	}

	if (optind != argc - 1)
		usage();

	opts->path_to_scan = strdup(argv[optind]);
}

static void ipc_handler_scan_file(struct ipc_manager *m, void *data)
{
	struct scan *scan = (struct scan *)data;
	char *path, *status, *mod_name, *mod_report, *action;
	int progress, clean = 0;

	ipc_manager_get_arg_at(m, 0, IPC_STRING_T, &path);
	ipc_manager_get_arg_at(m, 1, IPC_STRING_T, &status);
	ipc_manager_get_arg_at(m, 2, IPC_STRING_T, &mod_name);
	ipc_manager_get_arg_at(m, 3, IPC_STRING_T, &mod_report);
	ipc_manager_get_arg_at(m, 4, IPC_STRING_T, &action);
	ipc_manager_get_arg_at(m, 5, IPC_INT32_T, &progress);

	/* path is empty string, do nothing */
	if (!*path)
		return;

	if (scan->summary != NULL) {
		scan->summary->scanned++;

		if (!strcmp(status, "ARMADITO_MALWARE"))
			scan->summary->malware++;
		else if (!strcmp(status, "ARMADITO_SUSPICIOUS"))
			scan->summary->suspicious++;
		else if (!strcmp(status, "ARMADITO_EINVAL")
			|| !strcmp(status, "ARMADITO_IERROR")
			|| !strcmp(status, "ARMADITO_UNKNOWN_FILE_TYPE")
			|| !strcmp(status, "ARMADITO_UNDECIDED"))
			scan->summary->unhandled++;
		else if (!strcmp(status, "ARMADITO_WHITE_LISTED")
			|| !strcmp(status, "ARMADITO_CLEAN")) {
			scan->summary->clean++;
			clean = 1;
		}
	}

	if (!scan->print_clean && clean)
		return;

	printf("%s: %s", path, status);
	if (strcmp(status, "ARMADITO_UNDECIDED") && strcmp(status, "ARMADITO_CLEAN") && strcmp(status, "ARMADITO_UNKNOWN_FILE_TYPE")) {
		printf(" [%s - %s]", mod_name, mod_report);
		printf(" (action %s) ", action);
	}
	printf("\n");
}

static void ipc_handler_scan_end(struct ipc_manager *m, void *data)
{
	struct a6o_scan *scan = (struct a6o_scan *)data;

	/* ??? */
}

static void print_summary(struct scan_summary *summary)
{
	printf("\nSCAN SUMMARY:\n");
	printf("scanned files     : %d\n", summary->scanned);
	printf("malware files     : %d\n", summary->malware);
	printf("suspicious files  : %d\n", summary->suspicious);
	printf("unhandled files   : %d\n", summary->unhandled);
	printf("clean files       : %d\n", summary->clean);
}

static struct scan_summary *scan_summary_new(void)
{
	struct scan_summary *s = malloc(sizeof(struct scan_summary));

	s->scanned = 0;
	s->malware = 0;
	s->suspicious = 0;
	s->unhandled = 0;
	s->clean = 0;

	return s;
}

static void do_scan(struct scan_options *opts, int client_sock)
{
	struct scan *scan;
	struct ipc_manager *manager;

	scan = (struct scan *)malloc(sizeof(struct scan));
	assert(scan != NULL);

	scan->summary = (opts->no_summary) ? NULL : scan_summary_new();
	scan->print_clean = opts->print_clean;

	manager = ipc_manager_new(client_sock);

	ipc_manager_add_handler(manager, IPC_MSG_ID_SCAN_FILE, ipc_handler_scan_file, scan);
	ipc_manager_add_handler(manager, IPC_MSG_ID_SCAN_END, ipc_handler_scan_end, scan);

	ipc_manager_msg_send(manager,
			IPC_MSG_ID_SCAN,
			IPC_STRING_T, opts->path_to_scan,
			IPC_INT32_T, opts->threaded,
			IPC_INT32_T, opts->recursive,
			IPC_NONE_T);

	while (ipc_manager_receive(manager) > 0)
		;

	if (!opts->no_summary) {
		print_summary(scan->summary);
		free(scan->summary);
	}

	ipc_manager_free(manager);
}

int main(int argc, char **argv)
{
	struct scan_options *opts = (struct scan_options *)malloc(sizeof(struct scan_options));
	int client_sock;

	parse_options(argc, argv, opts);

	client_sock = unix_client_connect(opts->unix_path, 10);

	if (client_sock < 0) {
		fprintf(stderr, "cannot open client socket (errno %d)\n", errno);
		return 1;
	}

	do_scan(opts, client_sock);

	return 0;
}

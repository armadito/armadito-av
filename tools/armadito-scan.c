/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "armadito-config.h"

#include <net/netdefaults.h>
#include <net/unixsockclient.h>
#include <rpc/io.h>
#include <rpc/rpctypes.h>
#include <libjrpc/jrpc.h>

#include <assert.h>
#include <getopt.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PROGRAM_NAME "armadito-scan"
#define PROGRAM_VERSION PACKAGE_VERSION

struct scan_options {
	int recursive;
	int threaded;
	int no_summary;
	int print_clean;
	const char *path_to_scan;
	int verbose;
	const char *unix_socket_path;
};

static struct option scan_option_defs[] = {
	{"help",         no_argument,        0, 'h'},
	{"version",      no_argument,        0, 'V'},
	{"verbose",      no_argument,        0, 'v'},
	{"recursive",    no_argument,        0, 'r'},
	{"threaded",     no_argument,        0, 't'},
	{"no-summary",   no_argument,        0, 'n'},
	{"socket-path",  required_argument,  0, 'a'},
#if O
	{"print-clean",  no_argument,        0, 'c'},
#endif
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
	fprintf(stderr, "  --verbose -v                  print HTTP trafic\n");
	fprintf(stderr, "  --recursive  -r               scan directories recursively\n");
	fprintf(stderr, "  --threaded -t                 scan using multiple threads\n");
	fprintf(stderr, "  --no-summary -n               disable summary at end of scanning\n");
	fprintf(stderr, "  --socket-path=PATH | -a PATH  unix socket path (default is " DEFAULT_SOCKET_PATH ")\n");
#if O
	/* yet not available with rpc api */
	fprintf(stderr, "  --print-clean -c              print also clean files as they are scanned\n");
#endif
	fprintf(stderr, "\n");

	exit(-1);
}

static void parse_options(int argc, char **argv, struct scan_options *opts)
{
	opts->recursive = 0;
	opts->threaded = 0;
	opts->no_summary = 0;
	opts->print_clean = 0;
	opts->path_to_scan = NULL;
	opts->unix_socket_path = DEFAULT_SOCKET_PATH;
	opts->verbose = 0;

	while (1) {
		int c, option_index = 0;

		c = getopt_long(argc, argv, "hVvrtna:", scan_option_defs, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 'h': /* help */
			usage();
			break;
		case 'V': /* version */
			version();
			break;
		case 'v': /* verbose */
			opts->verbose = 1;
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
		case 'a': /* path */
			opts->unix_socket_path = strdup(optarg);
			break;
		case '?':
			/* getopt_long already printed an error message. */
			break;
#if 0
		case 'c': /* print-clean */
			opts->print_clean = 1;
			break;
#endif
		default:
			abort ();
		}
	}

	if (optind != argc - 1)
		usage();

	opts->path_to_scan = strdup(argv[optind]);
}

static void scan_cb(json_t *result, void *user_data)
{
	int ret;

	*(int *)user_data = 1;
}

static void detection_event_print(struct a6o_detection_event *ev)
{
	printf("%s: %d [%s - %s] (action %d)\n",
		ev->path,
		ev->scan_status,
		ev->module_name,
		ev->module_report,
		ev->scan_action);
}

static void on_demand_progress_event_print(struct a6o_on_demand_progress_event *ev)
{
	/* ev->progress; */
	/* ev->path; */
	/* ev->malware_count; */
	/* ev->suspicious_count; */
	/* ev->scanned_count; */
}

static int notify_event_method(struct jrpc_connection *conn, json_t *params, json_t **result)
{
	struct a6o_event *ev;
	int ret;

	if ((ret = JRPC_JSON2STRUCT(a6o_event, params, &ev)))
		return ret;

	switch(ev->type) {
	case EVENT_DETECTION:
		detection_event_print(&ev->u.ev_detection);
		break;
	case EVENT_ON_DEMAND_PROGRESS:
		on_demand_progress_event_print(&ev->u.ev_on_demand_progress);
		break;
	}

	return JRPC_OK;
}

static struct jrpc_mapper *create_rpcfe_mapper(void)
{
	struct jrpc_mapper *rpcfe_mapper;

	rpcfe_mapper = jrpc_mapper_new();
	jrpc_mapper_add(rpcfe_mapper, "notify_event", notify_event_method);

	return rpcfe_mapper;
}

static int do_scan(struct scan_options *opts)
{
	struct jrpc_connection *conn;
	int client_sock;
	int *p_client_sock;
	int ret;
	struct a6o_rpc_scan_param param;
	json_t *j_param;
	static int done = 0;

	client_sock = unix_client_connect(opts->unix_socket_path, 10);

	if (client_sock < 0) {
		perror("cannot connect");
		exit(EXIT_FAILURE);
	}

	conn = jrpc_connection_new(create_rpcfe_mapper(), NULL);

	p_client_sock = malloc(sizeof(int));
	*p_client_sock = client_sock;

	jrpc_connection_set_read_cb(conn, unix_fd_read_cb, p_client_sock);
	jrpc_connection_set_write_cb(conn, unix_fd_write_cb, p_client_sock);

	/* jrpc_connection_set_error_handler(conn, client_error_handler); */

	param.root_path = opts->path_to_scan;
	param.send_progress = 1;
	if ((ret = JRPC_STRUCT2JSON(a6o_rpc_scan_param, &param, &j_param)))
		return ret;

	if ((ret = jrpc_call(conn, "scan", j_param, scan_cb, &done)))
		return ret;

	while((ret = jrpc_process(conn)) != JRPC_EOF && !done)
		;

	if (close(client_sock) < 0)
		perror("closing connection");

	return 0;
}

int main(int argc, char **argv)
{
	struct scan_options *opts = (struct scan_options *)malloc(sizeof(struct scan_options));

	parse_options(argc, argv, opts);

	do_scan(opts);

	return 0;
}

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
#include <time.h>
#include <unistd.h>

#define PROGRAM_NAME "armadito-scan"
#define PROGRAM_VERSION PACKAGE_VERSION

struct info_options {
	const char *unix_socket_path;
	int verbose;
};

static struct option info_option_defs[] = {
	{"help",         no_argument,        0, 'h'},
	{"version",      no_argument,        0, 'V'},
	{"verbose",      no_argument,        0, 'v'},
	{"socket-path",  required_argument,  0, 'a'},
	{0, 0, 0, 0}
};

static void version(void)
{
	printf("%s %s\n", PROGRAM_NAME, PROGRAM_VERSION);
	exit(EXIT_SUCCESS);
}

static void usage(void)
{
	fprintf(stderr, "usage: armadito-info [options]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Armadito antivirus information\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  --help  -h                    print help and quit\n");
	fprintf(stderr, "  --version -V                  print program version\n");
	fprintf(stderr, "  --verbose -v                  print HTTP trafic\n");
	fprintf(stderr, "  --socket-path=PATH | -a PATH  unix socket path (default is " DEFAULT_SOCKET_PATH ")\n");
	fprintf(stderr, "                                Prefix the path with @ for a Linux abstract socket path (see man 7 unix)\n");
	fprintf(stderr, "                                Example: --socket-path=@/org/armadito-daemon\n");
	fprintf(stderr, "\n");

	exit(1);
}

static void parse_options(int argc, char **argv, struct info_options *opts)
{
	opts->verbose = 0;
	opts->unix_socket_path = DEFAULT_SOCKET_PATH;

	while (1) {
		int c;
		int option_index = 0;

		c = getopt_long(argc, argv, "hVva:", info_option_defs, &option_index);

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
		case 'a': /* path */
			opts->unix_socket_path = strdup(optarg);
			break;
		case '?':
			/* getopt_long already printed an error message. */
			break;
		default:
			abort ();
		}
	}

	/* the command does not take arguments, just options */
	if (optind < argc)
		usage();
}

static void time_2_date(int64_t timestamp, char *buf, size_t len)
{
	strftime(buf, len, "%FT%TZ", gmtime(&timestamp));
}

static void info_print(struct a6o_info *info)
{
	char buf[sizeof("1970-01-01T00:00:00Z!")];
	struct a6o_module_info **p_mod_info;

	printf("--- Armadito info --- \n");
	printf("antivirus version: %s\n", info->antivirus_version);
	printf("global status : %s\n", a6o_update_status_str(info->global_status));
	time_2_date(info->global_update_ts, buf, sizeof(buf));
	printf("global update date : %s\n", buf);

	for (p_mod_info = info->module_infos; *p_mod_info != NULL; p_mod_info++) {
		struct a6o_module_info *mod_info = *p_mod_info;
		struct a6o_base_info **p_base_info;

		printf("Module %s \n",  mod_info->name);
		printf("- Update status : %s\n", a6o_update_status_str(mod_info->mod_status));
		time_2_date(mod_info->mod_update_ts, buf, sizeof(buf));
		printf("- Update date : %s\n", buf);

		if (mod_info->base_infos == NULL)
			continue;

		for (p_base_info = mod_info->base_infos; *p_base_info != NULL; p_base_info++) {
			struct a6o_base_info *base_info = *p_base_info;

			printf("-- Base %s \n", base_info->name);
			time_2_date(base_info->base_update_ts, buf, sizeof(buf));
			printf("--- Update date : %s\n", buf);
			if (base_info->version != NULL)
				printf("--- Version : %s\n", base_info->version);
			printf("--- Signature count : %ld\n", base_info->signature_count);
			printf("--- Full path : %s\n", base_info->full_path);
		}
	}
}

static void info_cb(json_t *result, void *user_data)
{
	int ret;
	struct a6o_info *info;

	if ((ret = JRPC_JSON2STRUCT(a6o_info, result, &info)))
		return;

	info_print(info);

	a6o_info_free(info);

	*(int *)user_data = 1;
}

static int do_info(struct info_options *opts)
{
	struct jrpc_connection *conn;
	int client_sock;
	int *p_client_sock;
	int ret;
	static int done = 0;

	client_sock = unix_client_connect(opts->unix_socket_path, 10);

	if (client_sock < 0) {
		perror("cannot connect");
		exit(EXIT_FAILURE);
	}

	conn = jrpc_connection_new(NULL, NULL);

	p_client_sock = malloc(sizeof(int));
	*p_client_sock = client_sock;

	jrpc_connection_set_read_cb(conn, unix_fd_read_cb, p_client_sock);
	jrpc_connection_set_write_cb(conn, unix_fd_write_cb, p_client_sock);

	/* jrpc_connection_set_error_handler(conn, client_error_handler); */

	ret = jrpc_call(conn, "status", NULL, info_cb, &done);
	if (ret) {
		jrpc_connection_free(conn);
		return ret;
	}

	while((ret = jrpc_process(conn)) != JRPC_EOF && !done)
		;

	if (close(client_sock) < 0)
		perror("closing connection");

	jrpc_connection_free(conn);

	return 0;
}

int main(int argc, char **argv)
{
	struct info_options *opts = (struct info_options *)malloc(sizeof(struct info_options));

	parse_options(argc, argv, opts);

	do_info(opts);

	free(opts);

	return 0;
}

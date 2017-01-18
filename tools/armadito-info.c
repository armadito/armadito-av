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
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PROGRAM_NAME "armadito-scan"
#define PROGRAM_VERSION PACKAGE_VERSION

struct info_options {
	const char *unix_path;
	int verbose;
};

static struct option info_option_defs[] = {
	{"help",      no_argument,        0, 'h'},
	{"version",   no_argument,        0, 'V'},
	{"verbose",   no_argument,        0, 'v'},
	{"path",      required_argument, 0, 'a'},
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
	fprintf(stderr, "  --path=PATH | -a PATH         unix socket path (default is " DEFAULT_SOCKET_PATH ")\n");
	fprintf(stderr, "\n");

	exit(1);
}

static void parse_options(int argc, char **argv, struct info_options *opts)
{
	opts->verbose = 0;
	opts->unix_path = DEFAULT_SOCKET_PATH;

	while (1) {
		int c, option_index = 0;

		c = getopt_long(argc, argv, "hVvp:", info_option_defs, &option_index);

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
			opts->unix_path = strdup(optarg);
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

#if 0
static void status_print(struct json_object *j_ev)
{
	int n_mods, m;
	struct json_object *j_mod_array;
	char buf[sizeof("1970-01-01T00:00:00Z!")];

	printf( "--- Armadito info --- \n");
	printf( "global status : %s\n", j_get_string(j_ev, "global_status"));
	time_2_date(j_get_int64(j_ev, "global_update_timestamp"), buf, sizeof(buf));
	printf( "global update date : %s\n", buf);

	json_object_object_get_ex(j_ev, "modules", &j_mod_array);
	n_mods = json_object_array_length(j_mod_array);

	for(m = 0; m < n_mods; m++) {
		struct json_object *j_mod, *j_base_array;
		int n_bases, b;

		j_mod = json_object_array_get_idx(j_mod_array, m);
		printf( "Module %s \n",  j_get_string(j_mod, "name"));
		printf( "- Update status : %s\n", j_get_string(j_mod, "mod_status"));
		time_2_date(j_get_int64(j_mod, "mod_update_timestamp"), buf, sizeof(buf));
		printf( "- Update date : %s\n", buf);

		if (!json_object_object_get_ex(j_mod, "bases", &j_base_array))
			continue;

		n_bases = json_object_array_length(j_base_array);

		for(b = 0; b < n_bases; b++) {
			struct json_object *j_base;
			const char *version;

			j_base = json_object_array_get_idx(j_base_array, b);
			printf( "-- Base %s \n", j_get_string(j_base, "name"));
			time_2_date(j_get_int64(j_base, "base_update_ts"), buf, sizeof(buf));
			printf( "--- Update date : %s\n", buf);
			version = j_get_string(j_base, "version");
			if (version != NULL)
				printf( "--- Version : %s\n", version);
			printf( "--- Signature count : %d\n", j_get_int(j_base, "signature_count"));
			printf( "--- Full path : %s\n", j_get_string(j_base, "full_path"));
		}
	}
}
#endif

static void info_cb(json_t *result, void *user_data)
{
	int ret;

	/* if ((ret = JRPC_JSON2STRUCT(operands, result, &op))) */
	/* 	return; */

}


#if 0
static int test_call(struct jrpc_connection *conn, const char *method, int op1, int op2)
{
	struct operands *s_op = operands_new(1);
	json_t *j_op;
	int ret;

	s_op->i_op1 = op1;
	s_op->i_op2 = op2;

	if ((ret = JRPC_STRUCT2JSON(operands, s_op, &j_op)))
		return ret;

	return jrpc_call(conn, method, j_op, simple_cb, NULL);
}
#endif

static void do_info(struct info_options *opts)
{
	struct jrpc_connection *conn;
	int client_sock;
	int *p_client_sock;
	int ret;

	client_sock = unix_client_connect(opts->unix_path, 10);

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
}

int main(int argc, char **argv)
{
	struct info_options *opts = (struct info_options *)malloc(sizeof(struct info_options));

	parse_options(argc, argv, opts);

	do_info(opts);

	return 0;
}

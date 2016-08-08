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

#include "libarmadito-config.h"

#include <assert.h>
#include <getopt.h>
#include <errno.h>
#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "apiclient.h"
#include "jutil.h"

#define DEFAULT_PORT          8888
#define S_DEFAULT_PORT        "8888"

#define PROGRAM_NAME "armadito-scan"
#define PROGRAM_VERSION PACKAGE_VERSION

struct info_options {
	unsigned short port;
	int verbose;
};

static struct option info_option_defs[] = {
	{"help",      no_argument,        0, 'h'},
	{"version",   no_argument,        0, 'V'},
	{"verbose",   no_argument,        0, 'v'},
	{"port",      required_argument,  0, 'p'},
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
	fprintf(stderr, "  --port=PORT | -p PORT         TCP port to connect to Armadito-AV (default is " S_DEFAULT_PORT ")\n");
	fprintf(stderr, "\n");

	exit(1);
}

static void parse_options(int argc, char **argv, struct info_options *opts)
{
	opts->port = DEFAULT_PORT;
	opts->verbose = 0;

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
		case 'p': /* port */
			opts->port = (unsigned short)atoi(optarg);
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

static void do_info(struct info_options *opts, struct api_client *client)
{
	int end_of_info = 0;

	if (api_client_register(client) != 0) {
		fprintf(stderr, "cannot register client: %s\n", api_client_get_error(client));
		return;
	}

	if (api_client_call(client, "/status", NULL, NULL) != 0) {
		fprintf(stderr, "cannot get status from server: %s\n", api_client_get_error(client));
		return;
	}

	while (!end_of_info) {
		struct json_object *j_response = NULL;

		if (api_client_call(client, "/event", NULL, &j_response) != 0) {
			fprintf(stderr, "cannot get event from server: %s\n", api_client_get_error(client));
			return;
		}

		if (j_response == NULL)
			continue;

		if (api_client_is_verbose(client)) {
			json_object_to_file("/dev/stderr", j_response);
			fprintf(stderr, "\n");
		}

		if (!strcmp(j_get_string(j_response, "event_type"), "StatusEvent")) {
			status_print(j_response);
			end_of_info = 1;
		}
	}

	if (api_client_unregister(client) != 0)
		fprintf(stderr, "cannot unregister client: %s\n", api_client_get_error(client));
}

int main(int argc, char **argv)
{
	struct info_options *opts = (struct info_options *)malloc(sizeof(struct info_options));
	struct api_client *client;

	parse_options(argc, argv, opts);

	client = api_client_new(opts->port, opts->verbose);

	do_info(opts, client);

	return 0;
}

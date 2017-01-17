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

#include <assert.h>
#include <getopt.h>
#include <errno.h>
#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apiclient.h"
#include "jutil.h"

#define DEFAULT_PORT          8888
#define S_DEFAULT_PORT        "8888"

#define PROGRAM_NAME "armadito-scan"
#define PROGRAM_VERSION PACKAGE_VERSION

struct scan_options {
	const char *unix_path;
	int recursive;
	int threaded;
	int no_summary;
	int print_clean;
	const char *path_to_scan;
	unsigned short port;
	int verbose;
};

static struct option scan_option_defs[] = {
	{"help",        no_argument,        0, 'h'},
	{"version",     no_argument,        0, 'V'},
	{"verbose",     no_argument,        0, 'v'},
	{"recursive",   no_argument,        0, 'r'},
	{"threaded",    no_argument,        0, 't'},
	{"no-summary",  no_argument,        0, 'n'},
#if O
	{"print-clean", no_argument,        0, 'c'},
#endif
	{"port",        required_argument,  0, 'p'},
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
#if O
	/* yet not available with rest api */
	fprintf(stderr, "  --print-clean -c              print also clean files as they are scanned\n");
#endif
	fprintf(stderr, "  --port=PORT | -p PORT         TCP port to connect to Armadito-AV (default is " S_DEFAULT_PORT ")\n");
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
	opts->port = DEFAULT_PORT;
	opts->verbose = 0;

	while (1) {
		int c, option_index = 0;

		c = getopt_long(argc, argv, "hVvrtnp:", scan_option_defs, &option_index);

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
#if 0
		case 'c': /* print-clean */
			opts->print_clean = 1;
			break;
#endif
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

	if (optind != argc - 1)
		usage();

	opts->path_to_scan = strdup(argv[optind]);
}

static void detection_event_print(struct json_object *j_ev)
{
	printf("%s: %s [%s - %s] (action %s)\n",
		j_get_string(j_ev, "path"),
		j_get_string(j_ev, "scan_status"),
		j_get_string(j_ev, "module_name"),
		j_get_string(j_ev, "module_report"),
		j_get_string(j_ev, "scan_action"));
}

static void completed_event_print(struct json_object *j_ev)
{
	printf("\nSCAN SUMMARY:\n");
	printf("scanned files     : %d\n", j_get_int(j_ev, "total_scanned_count"));
	printf("malware files     : %d\n", j_get_int(j_ev, "total_malware_count"));
	printf("suspicious files  : %d\n", j_get_int(j_ev, "total_suspicious_count"));
}

static int do_scan(struct scan_options *opts, struct api_client *client)
{
	struct json_object *j_request, *j_response;
	int end_of_scan, status = 0;
	const char *ev_type;

	if (api_client_register(client) != 0) {
		fprintf(stderr, "cannot register client: %s\n", api_client_get_error(client));
		return -1;
	}

	j_request = json_object_new_object();
	json_object_object_add(j_request, "path", json_object_new_string(opts->path_to_scan));
	if (api_client_call(client, "/scan", j_request, &j_response) != 0) {
		fprintf(stderr, "cannot start scan on server: %s\n", api_client_get_error(client));
		return -1;
	}

	for (end_of_scan = 0; !end_of_scan; ) {
		j_response = NULL;

		if (api_client_call(client, "/event", NULL, &j_response) != 0) {
			fprintf(stderr, "cannot get event from server: %s\n", api_client_get_error(client));
			return -1;
		}

		if (j_response == NULL)
			continue;

		if (api_client_is_verbose(client)) {
			json_object_to_file("/dev/stderr", j_response);
			fprintf(stderr, "\n");
		}

		ev_type = j_get_string(j_response, "event_type");
		if (!strcmp(ev_type, "OnDemandCompletedEvent")) {
			if (!opts->no_summary)
				completed_event_print(j_response);
			if (j_get_int(j_response, "total_malware_count") != 0)
				status = 1;
			end_of_scan = 1;
		} else if (!strcmp(ev_type, "DetectionEvent"))
			detection_event_print(j_response);
	}

	if (api_client_unregister(client) != 0) {
		fprintf(stderr, "cannot unregister client: %s\n", api_client_get_error(client));
		return -1;
	}

	return status;
}

int main(int argc, char **argv)
{
	struct scan_options *opts = (struct scan_options *)malloc(sizeof(struct scan_options));
	struct api_client *client;

	parse_options(argc, argv, opts);

	client = api_client_new(opts->port, opts->verbose);

	return do_scan(opts, client);
}

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

#include "apiclient.h"

#define DEFAULT_PORT          8888
#define S_DEFAULT_PORT        "8888"

#define PROGRAM_NAME "armadito-scan"
#define PROGRAM_VERSION PACKAGE_VERSION

struct detection_event {
	const char *detection_time;
	const char *context;
	const char *path;
	const char *scan_status;
	const char *scan_action;
	const char *module_name;
	const char *module_report;
};

struct on_demand_completed_event {
	int total_malware_count;
	int total_suspicious_count;
	int total_scanned_count;
};

struct event {
	enum event_type {
		DETECTION_EVENT,
		ON_DEMAND_PROGRESS_EVENT,
		ON_DEMAND_COMPLETED_EVENT,
		UNKNOWN_EVENT,
	} type;
	union {
		struct detection_event d;
		struct on_demand_completed_event c;
	} content;
};

struct scan_options {
	const char *unix_path;
	int recursive;
	int threaded;
	int no_summary;
	int print_clean;
	const char *path_to_scan;
	unsigned short port;
};

static struct option scan_option_defs[] = {
	{"help",        no_argument,        0, 'h'},
	{"version",     no_argument,        0, 'V'},
	{"recursive",   no_argument,        0, 'r'},
	{"threaded",    no_argument,        0, 't'},
	{"no-summary",  no_argument,        0, 'n'},
#if O
	{"print-clean", no_argument,        0, 'c'},
#endif
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
#if O
	/* yet not available with rest api */
	fprintf(stderr, "  --print-clean -c              print also clean files as they are scanned\n");
#endif
	fprintf(stderr, "  --port=PORT | -p PORT         TCP port to connect to Armadito-AV (default is " S_DEFAULT_PORT ")\n");
	fprintf(stderr, "\n");

	exit(1);
}

static void parse_options(int argc, char **argv, struct scan_options *opts)
{
	opts->recursive = 0;
	opts->threaded = 0;
	opts->no_summary = 0;
	opts->print_clean = 0;
	opts->path_to_scan = NULL;
	opts->port = DEFAULT_PORT;

	while (1) {
		int c, option_index = 0;

		c = getopt_long(argc, argv, "hVrtn", scan_option_defs, &option_index);

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

static const char *json_object_get_string_or_null(struct json_object *j_obj, const char *key)
{
	struct json_object *j_str = NULL;

	if (!json_object_object_get_ex(j_obj, key, &j_str)
		|| !json_object_is_type(j_str, json_type_string))
		return NULL;

	return strdup(json_object_get_string(j_str));
}

static int json_object_get_int_or_def(struct json_object *j_obj, const char *key, int def)
{
	struct json_object *j_int = NULL;

	if (!json_object_object_get_ex(j_obj, key, &j_int)
		|| !json_object_is_type(j_int, json_type_int))
		return def;

	return json_object_get_int(j_int);
}

static enum event_type process_json_event(struct json_object *j_ev, struct event *ev)
{
	const char *ev_type = json_object_get_string_or_null(j_ev, "event_type");

	if (ev_type == NULL) {
		ev->type = UNKNOWN_EVENT;
		return ev->type;
	}
	if (!strcmp(ev_type, "DetectionEvent"))
		ev->type = DETECTION_EVENT;
	else if (!strcmp(ev_type, "OnDemandProgressEvent"))
		ev->type = ON_DEMAND_PROGRESS_EVENT;
	else if (!strcmp(ev_type, "OnDemandCompletedEvent"))
		ev->type = ON_DEMAND_COMPLETED_EVENT;
	else
		ev->type = UNKNOWN_EVENT;

	switch(ev->type) {
	case DETECTION_EVENT:
		ev->content.d.path = json_object_get_string_or_null(j_ev, "path");
		ev->content.d.detection_time = json_object_get_string_or_null(j_ev, "detection_time");
		ev->content.d.context = json_object_get_string_or_null(j_ev, "context");
		ev->content.d.path = json_object_get_string_or_null(j_ev, "path");
		ev->content.d.scan_status = json_object_get_string_or_null(j_ev, "scan_status");
		ev->content.d.scan_action = json_object_get_string_or_null(j_ev, "scan_action");
		ev->content.d.module_name = json_object_get_string_or_null(j_ev, "module_name");
		ev->content.d.module_report = json_object_get_string_or_null(j_ev, "module_report");
		break;
	case ON_DEMAND_COMPLETED_EVENT:
		ev->content.c.total_scanned_count = json_object_get_int_or_def(j_ev, "total_scanned_count", 0);
		ev->content.c.total_malware_count = json_object_get_int_or_def(j_ev, "total_malware_count", 0);
		ev->content.c.total_suspicious_count = json_object_get_int_or_def(j_ev, "total_suspicious_count", 0);
		break;
	}

	return ev->type;
}

static void do_scan(struct scan_options *opts, struct api_client *client)
{
	struct json_object *j_request, *j_response;
	int end_of_scan = 0;
	struct event ev;

	api_client_register(client);

	j_request = json_object_new_object();
	json_object_object_add(j_request, "path", json_object_new_string(opts->path_to_scan));
	api_client_call(client, "/scan", j_request, &j_response);

	while (!end_of_scan) {
	j_response = NULL;
		api_client_call(client, "/event", NULL, &j_response);
		if (j_response == NULL)
			continue;

		json_object_to_file("/dev/stderr", j_response);
		fprintf(stderr, "\n");

		switch(process_json_event(j_response, &ev)) {
		case DETECTION_EVENT:
			printf("%s: %s [%s - %s] (action %s)\n",
				ev.content.d.path,
				ev.content.d.scan_status,
				ev.content.d.module_name,
				ev.content.d.module_report,
				ev.content.d.scan_action);
			break;
		case ON_DEMAND_COMPLETED_EVENT:
			end_of_scan = 1;
			if (!opts->no_summary) {
				printf("\nSCAN SUMMARY:\n");
				printf("scanned files     : %d\n", ev.content.c.total_scanned_count);
				printf("malware files     : %d\n", ev.content.c.total_malware_count);
				printf("suspicious files  : %d\n", ev.content.c.total_suspicious_count);
			}
			break;
		}
	}

	api_client_unregister(client);
}

int main(int argc, char **argv)
{
	struct scan_options *opts = (struct scan_options *)malloc(sizeof(struct scan_options));
	struct api_client *client;

	parse_options(argc, argv, opts);

	client = api_client_new(opts->port);

	do_scan(opts, client);

	return 0;
}

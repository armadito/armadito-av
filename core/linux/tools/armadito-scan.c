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

struct scan_summary {
	int scanned;
	int malware;
	int suspicious;
	int unhandled;
	int clean;
};

struct scan_report {
	const char *path;
	const char *scan_status;
	const char *mod_name;
	const char *mod_report;
	const char *scan_action;
	int progress;
	int malware_count;
	int suspicious_count;
	int scanned_count;
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
	/* not available with rest api */
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

static void process_json_report(struct json_object *j_report, struct scan_report *report)
{
	report->path = json_object_get_string_or_null(j_report, "path");
	report->scan_status = json_object_get_string_or_null(j_report, "scan_status");
	report->mod_name = json_object_get_string_or_null(j_report, "mod_name");
	report->mod_report = json_object_get_string_or_null(j_report, "mod_report");
	report->scan_action = json_object_get_string_or_null(j_report, "scan_action");

	report->progress = json_object_get_int_or_def(j_report, "progress", -1);
	report->malware_count = json_object_get_int_or_def(j_report, "malware_count", -1);
	report->suspicious_count = json_object_get_int_or_def(j_report, "suspicious_count", -1);
	report->scanned_count = json_object_get_int_or_def(j_report, "scanned_count", -1);
}

static int process_report(struct scan_report *report)
{
#if 0
	/* path is empty string, do nothing */
	if (!*path)
		return;
#endif

#if 0
	if (!strcmp(report->scan_status, "white_listed") || !strcmp(report->scan_status, "clean"))
		return 0;
#endif

	printf("%s: %s", report->path, report->scan_status);
	if (strcmp(report->scan_status, "undecided")
		&& strcmp(report->scan_status, "clean")
		&& strcmp(report->scan_status, "unknown_file_type")) {
		printf(" [%s - %s]", report->mod_name, report->mod_report);
		printf(" (action %s) ", report->scan_action);
	}
	printf("\n");

	return report->progress >= 100;
}


static void do_scan(struct scan_options *opts)
{
	struct api_client *client;
	struct json_object *j_request;
	struct json_object *j_response;
	int end_of_scan = 0;

#if 0
	scan->summary = (opts->no_summary) ? NULL : scan_summary_new();
	scan->print_clean = opts->print_clean;
#endif

	client = api_client_new(opts->port);

	api_client_register(client);

	j_request = json_object_new_object();
	json_object_object_add(j_request, "path", json_object_new_string(opts->path_to_scan));
	api_client_call(client, "/scan", j_request, &j_response);

	while (!end_of_scan) {
		j_response = NULL;
		api_client_call(client, "/poll", NULL, &j_response);
		if (j_response != NULL) {
			struct scan_report report;

			json_object_to_file("/dev/stderr", j_response);
			fprintf(stderr, "\n");

			process_json_report(j_response, &report);
			end_of_scan = process_report(&report);
		}
	}

#if 0
	if (!opts->no_summary) {
		print_summary(scan->summary);
		free(scan->summary);
	}
#endif
}

int main(int argc, char **argv)
{
	struct scan_options *opts = (struct scan_options *)malloc(sizeof(struct scan_options));

	parse_options(argc, argv, opts);

	do_scan(opts);

	return 0;
}

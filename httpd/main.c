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

#include <getopt.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "httpd.h"
#include "log.h"

#define DEFAULT_LOG_MODE     "stderr"
#define DEFAULT_PORT          8888
#define S_DEFAULT_PORT        "8888"

#define PROGRAM_NAME "armadito-httpd"
#define PROGRAM_VERSION PACKAGE_VERSION

struct httpd_options {
	const char *s_log_mode;
	unsigned short port;
};

static struct option daemon_option_defs[] = {
	{"help",      no_argument,        0, 'h'},
	{"version",   no_argument,        0, 'V'},
	{"log-mode",  required_argument,  0, 'l'},
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
	fprintf(stderr, "usage: armadito-daemon [options]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Armadito antivirus scanner daemon\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  --help  -h                         print help and quit\n");
	fprintf(stderr, "  --version -V                       print program version\n");
	fprintf(stderr, "  --log-mode=mode | -l MODE          set log mode\n");
	fprintf(stderr, "                                     log level can be: system, stderr\n");
	fprintf(stderr, "                                     (default is: " DEFAULT_LOG_MODE "\n");
	fprintf(stderr, "  --port=PORT | -p PORT              listening TCP port (default is " S_DEFAULT_PORT ")\n");
	fprintf(stderr, "\n");

	exit(EXIT_FAILURE);
}

static int check_log_mode(const char *s_log_mode)
{
	return !strcmp(s_log_mode,"stderr")
		|| !strcmp(s_log_mode,"system");
}

static void parse_options(int argc, char **argv, struct httpd_options *opts)
{
	opts->s_log_mode = DEFAULT_LOG_MODE;
	opts->port = DEFAULT_PORT;

	while (1) {
		int c;
		int option_index = 0;

		c = getopt_long(argc, argv, "hVl:p:", daemon_option_defs, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 'h': /* help */
			usage();
			break;
		case 'V': /* version */
			version();
			break;
		case 'l': /* log-mode */
			if (!check_log_mode(optarg))
				usage();
			opts->s_log_mode = strdup(optarg);
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

	if (optind < argc)
		usage();
}

static enum log_mode log_mode_from_str(const char *s_log_mode)
{
	if (!strcmp(s_log_mode,"stderr"))
		return LOG_TO_STDERR;

	if (!strcmp(s_log_mode,"system"))
		return LOG_TO_SYSTEM_LOG;

	return -1;
}

static void start_http_server(const char *progname, struct httpd_options *opts)
{
	struct httpd *h;
	GMainLoop *loop;

	log_init(log_mode_from_str(opts->s_log_mode));

	log_i("starting http serveur on port %d", opts->port);

	h = httpd_new(opts->port);

	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);
}

int main(int argc, char **argv)
{
	struct httpd_options opts;

	parse_options(argc, argv, &opts);

	start_http_server(argv[0], &opts);

	return EXIT_SUCCESS;
}

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

#define _GNU_SOURCE
#include <libarmadito.h>
#include "armadito-config.h"

#include "log.h"
#include "server.h"
#include "daemonize.h"
#include "unixsockserver.h"
#include "net/netdefaults.h"

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <getopt.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

#define DEFAULT_LOG_LEVEL     "error"
#define DEFAULT_PID_FILE      LOCALSTATEDIR "/run/armadito-scand.pid"

#define PROGRAM_NAME "armadito-scand"
#define PROGRAM_VERSION PACKAGE_VERSION

struct a6o_daemon_options {
	int no_daemon;
	const char *s_log_level;
	const char *pid_file;
};

static struct option daemon_option_defs[] = {
	{"help",      no_argument,        0, 'h'},
	{"version",   no_argument,        0, 'V'},
	{"no-daemon", no_argument,        0, 'n'},
	{"log-level", required_argument,  0, 'l'},
	{"pidfile",   required_argument,  0, 'i'},
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
	fprintf(stderr, "  --no-daemon -n                     do not fork and go to background\n");
	fprintf(stderr, "  --log-level=LEVEL | -l LEVEL       set log level\n");
	fprintf(stderr, "                                     log level can be: error, warning, info, debug\n");
	fprintf(stderr, "                                     (default is: " DEFAULT_LOG_LEVEL "\n");
	fprintf(stderr, "  --pidfile=PATH | -i PATH           create PID file at specified location\n");
	fprintf(stderr, "  --port=PORT | -p PORT              listening TCP port (default is " S_DEFAULT_PORT ")\n");
	fprintf(stderr, "                                     (default is: " DEFAULT_PID_FILE ")\n");
	fprintf(stderr, "\n");

	exit(EXIT_FAILURE);
}

static int check_log_level(const char *s_log_level)
{
	return strcmp(s_log_level,"error")
		&& strcmp(s_log_level,"warning")
		&& strcmp(s_log_level,"info")
		&& strcmp(s_log_level,"debug");
}

static void parse_options(int argc, char **argv, struct a6o_daemon_options *opts)
{
	opts->no_daemon = 0;
	opts->s_log_level = DEFAULT_LOG_LEVEL;
	opts->pid_file = DEFAULT_PID_FILE;
	opts->port = DEFAULT_PORT;

	while (1) {
		int c, option_index = 0;

		c = getopt_long(argc, argv, "hVnl:i:p:", daemon_option_defs, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 'h': /* help */
			usage();
			break;
		case 'V': /* version */
			version();
			break;
		case 'n': /* no-daemon */
			opts->no_daemon = 1;
			break;
		case 'l': /* log-level */
			if (check_log_level(optarg))
				usage();
			opts->s_log_level = strdup(optarg);
			break;
		case 'p': /* port */
			opts->port = (unsigned short)atoi(optarg);
			break;
		case 'i': /* pidfile */
			opts->pid_file = strdup(optarg);
			break;
		case '?':
			/* getopt_long already printed an error message. */
			break;
		default:
			abort ();
		}
	}

	/* Print any remaining command line arguments (not options). */
	if (optind < argc)
		usage();
}

static void create_pid_file(const char *pidfile)
{
	FILE *f = fopen(pidfile, "w");

	if (f == NULL)
		goto err;

	if (fprintf(f, "%d\n", getpid()) < 0)
		goto err;

	if (fclose(f) != 0)
		goto err;

	return;

err:
	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR, "cannot create PID file %s (errno %d)", pidfile, errno);
	exit(EXIT_FAILURE);
}

static void load_conf(struct a6o_conf *conf)
{
	const char *conf_file;
	a6o_error *error = NULL;

	conf_file = a6o_std_path(CONFIG_FILE_LOCATION);

	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_INFO, "loading configuration file %s", conf_file);

	if (a6o_conf_load_file(conf, conf_file, &error)) {
		a6o_error_print(error, stderr);
		exit(EXIT_FAILURE);
	}

	free((void *)conf_file);
}

static void load_conf_dir(struct a6o_conf *conf)
{
	const char *conf_dir;
	DIR *dir;
	struct dirent *dp;

	conf_dir = a6o_std_path(CONFIG_DIR_LOCATION);

	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_INFO, "loading configuration directory %s", conf_dir);

	dir = opendir(conf_dir);
	if (dir == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_WARNING, "error opening configuration directory %s (%s)", conf_dir, strerror(errno));
		return;
	}

	while ((dp = readdir(dir)) != NULL) {
		const char *file_name = dp->d_name;
		size_t len;
		char *full_path;
		a6o_error *error = NULL;

		if ( !strcmp(file_name, ".") || !strcmp(file_name, ".."))
			continue;

		len = strlen(file_name);
		if (len > 5 && !strcmp(file_name + len - 5, ".conf")) {
			if (asprintf(&full_path, "%s/%s", conf_dir, file_name) == -1)
				break;

			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_INFO, "loading configuration file %s", full_path);

			if (a6o_conf_load_file(conf, full_path, &error)) {
				a6o_error_print(error, stderr);
				exit(EXIT_FAILURE);
			}

			free((void *)full_path);
		}
	}

	closedir(dir);

	free((void *)conf_dir);
}


#if 0
static void start_daemon(const char *progname, struct a6o_daemon_options *opts)
{
	struct a6o_conf *conf;
	struct armadito *armadito;
	int server_sock;
	struct server *server;
	a6o_error *error = NULL;
	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);

	log_init(opts->s_log_level, !opts->no_daemon);

	if (!opts->no_daemon)
		daemonize();

	if (opts->pid_file != NULL)
		create_pid_file(opts->pid_file);

	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_NONE, "starting %s%s", progname, opts->no_daemon ? "" : " in daemon mode");

	conf = a6o_conf_new();
	load_conf(conf);
	load_conf_dir(conf);

	armadito = a6o_open(conf, &error);
	if (armadito == NULL) {
		a6o_error_print(error, stderr);
		exit(EXIT_FAILURE);
	}

	g_main_loop_run(loop);
}
#endif

static void start_http_server(const char *progname, struct a6o_daemon_options *opts)
{
	struct a6o_conf *conf;
	struct armadito *armadito;
	a6o_error *error = NULL;
	struct httpd *h;
	GMainLoop *loop;

	log_init(opts->s_log_level, !opts->no_daemon);

	if (!opts->no_daemon)
		daemonize();

	if (opts->pid_file != NULL)
		create_pid_file(opts->pid_file);

	a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_NONE, "starting %s%s", progname, opts->no_daemon ? "" : " in daemon mode");

	conf = a6o_conf_new();
	load_conf(conf);
	load_conf_dir(conf);

	armadito = a6o_open(conf, &error);
	if (armadito == NULL) {
		a6o_error_print(error, stderr);
		exit(EXIT_FAILURE);
	}

	h = httpd_new(opts->port, armadito);

	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);
}

int main(int argc, char **argv)
{
	struct a6o_daemon_options opts;

#ifdef HAVE_GTHREAD_INIT
	g_thread_init(NULL);
#endif

	parse_options(argc, argv, &opts);

	start_http_server(argv[0], &opts);

	return EXIT_SUCCESS;
}

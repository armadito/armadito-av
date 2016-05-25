/***

Copyright (C) 2015, 2016, 2017 Teclib'

This file is part of Armadito.

Armadito is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "monitor.h"
#include "mimetype.h"
#include "pollset.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/fanotify.h>

struct fanotify_entry {
	const char *path;
	unsigned int flags;
};

struct fanotify_options {
	int enable_permission;
	int type_check;
	int log_event;
	struct fanotify_entry *entries;
	int n_entries;
};

static void usage(void)
{
	fprintf(stderr, "usage: fanotify-test OPTIONS...\n");
	fprintf(stderr, "linux fanotify test\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -h                             print help and quit\n");
	fprintf(stderr, "  -e                             enable permission\n");
	fprintf(stderr, "  -t                             check file mime type before ALLOWing\n");
	fprintf(stderr, "  -l                             log every fanotify event\n");
	fprintf(stderr, "  -M DIR                         add DIR to the fanotify mark mask with flag FAN_MARK_MOUNT\n");
	fprintf(stderr, "  -m DIR                         add DIR to the fanotify mark mask\n");
	fprintf(stderr, "  -i DIR                         add DIR to the fanotify ignore mask\n");
	fprintf(stderr, "\n");

	exit(1);
}

static void opts_init(struct fanotify_options *opts, int argc, const char **argv)
{
	int i;

	opts->enable_permission = 0;
	opts->type_check = 0;
	opts->log_event = 0;

	/* fat allocation ;-) */
	opts->entries = malloc((argc - 1) * sizeof(struct fanotify_entry));
	for (i = 0; i < argc - 1; i++) {
		opts->entries[i].path = NULL;
		opts->entries[i].flags = 0;
	}

	opts->n_entries = 0;
}

static void opts_add_entry(struct fanotify_options *opts, const char *path, unsigned int flags)
{
	opts->entries[opts->n_entries].path = strdup(path);
	opts->entries[opts->n_entries].flags = flags;

	opts->n_entries++;
}

static void opts_process(struct fanotify_options *opts, int argc, const char **argv)
{
	int opt;

	if (argc < 2)
		usage();

	opts_init(opts, argc, argv);

	while ((opt = getopt(argc, (char * const *)argv, "hetlM:m:i:")) != -1) {
		switch (opt) {
		case 'h':
			usage();
			break;
		case 'e':
			opts->enable_permission = 1;
			break;
		case 't':
			opts->type_check = 1;
			break;
		case 'l':
			opts->log_event = 1;
			break;
		case 'M':
			opts_add_entry(opts, optarg, FAN_MARK_MOUNT);
			break;
		case 'i':
			opts_add_entry(opts, optarg, FAN_MARK_IGNORED_MASK);
			break;
		case 'm':
			opts_add_entry(opts, optarg, 0);
			break;
		default:
			usage();
		}
	}

	if (optind < argc)
		usage();
}

int main(int argc, const char **argv)
{
	struct fanotify_options opts;
	enum access_monitor_flags flags = 0;
	struct access_monitor *monitor;
	struct poll_set *ps;
	int i;

	opts_process(&opts, argc, argv);

	mime_type_init();

	if (opts.type_check)
		flags |= MONITOR_TYPE_CHECK;

	if (opts.log_event)
		flags |= MONITOR_LOG_EVENT;

	if (opts.enable_permission)
		flags |= MONITOR_ENABLE_PERM;

	monitor = access_monitor_new(flags);

	if (monitor == NULL)
		return 2;

	ps = poll_set_new();

	poll_set_add_fd(ps, access_monitor_get_poll_fd(monitor), access_monitor_cb, monitor);

	for (i = 0; i < opts.n_entries; i++)
		access_monitor_add(monitor, opts.entries[i].path, opts.entries[i].flags);

	return poll_set_loop(ps);
}

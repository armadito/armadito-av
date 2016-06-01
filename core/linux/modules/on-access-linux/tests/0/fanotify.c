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

#include "fanotify.h"


/*
  This function create a fanotify fd for the path given in parameter.
*/
int create_fanotify_fd(char * path){

	int fd;

	fd = fanotify_init(FAN_CLOEXEC | FAN_CLASS_CONTENT | FAN_NONBLOCK,
			O_RDONLY | O_LARGEFILE );

	if (fd == -1) {
		perror("[-] create_fanotify_fd() :: fanotify_init failed");

		exit(EXIT_FAILURE);
	}

	printf("[+] create_fanotify_fd() :: fanotify_init ... (fd = %d)[OK]\n",fd);


	// FAN_OPEN_PERM temporarly removed
	if (fanotify_mark(fd, FAN_MARK_ADD | FAN_MARK_MOUNT,
				FAN_CLOSE_WRITE | FAN_OPEN | FAN_EVENT_ON_CHILD, AT_FDCWD,
				path) == -1) {
		perror("[-] create_fanotify_fd() :: fanotify_mark");
		close(fd); ///
		exit(EXIT_FAILURE);
	}

	printf("[+] create_fanotify_fd() :: fanotify_mark on (%s) ... [OK]\n",path);

	return fd;
}


void fanotify_callback(GIOChannel * source, GIOCondition * condition, gpointer data){

	// TODO fa_notify_callback

	//printf("fa_notify_callback...\n");

	const struct fanotify_event_metadata *metadata;
	struct fanotify_event_metadata buf[200];
	ssize_t len;
	char path[PATH_MAX];
	ssize_t path_len;
	char procfd_path[PATH_MAX];
	struct fanotify_response response;
	int fd = data;

	//printf("fd = %d\n",fd);
	/* Loop while events can be read from fanotify file descriptor */

	/* for(;;) { */

	/* Read some events */

	len = read(fd, (void *) &buf, sizeof(buf));
	if (len == -1 && errno != EAGAIN) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	/* Check if end of available data reached */

	if (len <= 0)
		//break;
		return;

	/* Point to the first event in the buffer */

	metadata = buf;

	/* Loop over all events in the buffer */

	while (FAN_EVENT_OK(metadata, len)) {

		/* Check that run-time and compile-time structures match */

		if (metadata->vers != FANOTIFY_METADATA_VERSION) {
			fprintf(stderr,
				"Mismatch of fanotify metadata version.\n");
			exit(EXIT_FAILURE);
		}

		/* metadata->fd contains either FAN_NOFD, indicating a
		   queue overflow, or a file descriptor (a nonnegative
		   integer). Here, we simply ignore queue overflow. */

		if (metadata->fd >= 0) {

			/* Handle open permission event */

			if (metadata->mask & FAN_OPEN_PERM) {
				printf("FAN_OPEN_PERM: ");

				/* Allow file to be opened */

				response.fd = metadata->fd;
				//response.response = FAN_DENY;
				response.response = FAN_ALLOW;
				write(fd, &response,
					sizeof(struct fanotify_response));
			}

			/* Handle closing of writable file event */

			if (metadata->mask & FAN_CLOSE_WRITE)
				printf("FAN_CLOSE_WRITE: ");

			if (metadata->mask & FAN_CLOSE_NOWRITE)
				printf("FAN_CLOSE_NOWRITE: ");

			if (metadata->mask & FAN_MODIFY)
				printf("FAN_MODIFY: ");

			if (metadata->mask & FAN_ACCESS)
				printf("FAN_ACCESS: ");

			if (metadata->mask & FAN_OPEN)
				printf("FAN_OPEN: ");

			/* Retrieve and print pathname of the accessed file */

			snprintf(procfd_path, sizeof(procfd_path),
				"/proc/self/fd/%d", metadata->fd);

			//printf("[i] procfd_path = %s\n",procfd_path);

			path_len = readlink(procfd_path, path,
					sizeof(path) - 1);
			if (path_len == -1) {
				perror("readlink");
				exit(EXIT_FAILURE);
			}

			path[path_len] = '\0';
			printf("File %s :: Program PID %d\n", path, metadata->pid );

			// get the pid of the program that caused the event.
			//printf("Program PID = %d\n",metadata->pid );


			/* Close the file descriptor of the event */

			close(metadata->fd);

		}

		/* Advance to next event */

		metadata = FAN_EVENT_NEXT(metadata, len);
	}
	/* } */

	return;

}



/*
  This function add a new path to watch
*/
int add_fanotify_watch(char * path){

	int fd;
	GIOChannel * channel;
	guint res ;


	fd = create_fanotify_fd(path);

	channel = g_io_channel_unix_new(fd);

	// g_io_add_watch (GIOChannel *channel, GIOCondition condition, GIOFunc func, gpointer user_data);
	res = g_io_add_watch(channel, G_IO_IN, fanotify_callback, fd);


	return fd;

}

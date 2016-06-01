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

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

int daemonize(void)
{
	pid_t pid, sid;

	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	umask(0);

	/* Open any logs here */

	sid = setsid();
	if (sid < 0) {
		/* Log the failure */
		exit(EXIT_FAILURE);
	}

	if ((chdir("/")) < 0) {
		/* Log the failure */
		exit(EXIT_FAILURE);
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

/* refs:
   http://stackoverflow.com/questions/17954432/creating-a-daemon-in-linux
   http://www.netzmafia.de/skripten/unix/linux-daemon-howto.html
   http://shahmirj.com/blog/beginners-guide-to-creating-a-daemon-in-linux
*/
int other_daemonize(char* name, char* path, char* outfile, char* errfile, char* infile )
{
	if(!path) { path="/"; }
	if(!name) { name="medaemon"; }
	if(!infile) { infile="/dev/null"; }
	if(!outfile) { outfile="/dev/null"; }
	if(!errfile) { errfile="/dev/null"; }
	//printf("%s %s %s %s\n",name,path,outfile,infile);
	pid_t child;

	//fork, detach from process group leader
	if( (child=fork())<0 ) { //failed fork
		fprintf(stderr,"error: failed fork\n");
		exit(EXIT_FAILURE);
	}
	if (child>0) { //parent
		exit(EXIT_SUCCESS);
	}
	if( setsid()<0 ) { //failed to become session leader
		fprintf(stderr,"error: failed setsid\n");
		exit(EXIT_FAILURE);
	}

	//catch/ignore signals
	signal(SIGCHLD,SIG_IGN);
	signal(SIGHUP,SIG_IGN);

	//fork second time
	if ( (child=fork())<0) { //failed fork
		fprintf(stderr,"error: failed fork\n");
		exit(EXIT_FAILURE);
	}
	if( child>0 ) { //parent
		exit(EXIT_SUCCESS);
	}

	//new file permissions
	umask(0);
	//change to path directory
	chdir(path);

	//Close all open file descriptors
	int fd;
	for( fd=sysconf(_SC_OPEN_MAX); fd>0; --fd )
	{
		close(fd);
	}

	//reopen stdin, stdout, stderr
	stdin=fopen(infile,"r");   //fd=0
	stdout=fopen(outfile,"w+");  //fd=1
	stderr=fopen(errfile,"w+");  //fd=2

	//open syslog
	openlog(name,LOG_PID,LOG_DAEMON);
	return(0);
}


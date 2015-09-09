#include <stdio.h>
#include <stdlib.h>
#include "fanotify.h"
#include "dbus.h"


int main(int argc, char ** argv){


	GMainLoop * loop;

	printf("-------------------------------\n");
	printf("---- UHURU-AV REAL TIME POC ---\n");
	printf("-------------------------------\n");


	//printf("argc = %d\n",argc);
	
	if(argc < 2 ){
		printf("USAGE : ./rtime [init_path]\n");
		return -1;
	}


	// set a initial path
	add_fanotify_watch(argv[1]);


	add_mount_watch();

	// Starting the main loop
	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);


	return 0;
}
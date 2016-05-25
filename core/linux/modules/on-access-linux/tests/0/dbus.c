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

#include "dbus.h"
#include "fanotify.h"


static gchar * query_mounted_dev(GDBusConnection * conn, gchar * device_path){

	GVariant * ret;
	GError * error = NULL;
	GVariant * child, *tmp;
	GVariant * child2, *child3;
	const gchar * mount_dir_path, * value;
	gboolean bMounted = FALSE;
	int i,j;

	// Test parameters
	if(conn == NULL || device_path == NULL){
		printf("[-] query_mounted_dev :: :: NULL parameters\n");
		return;
	}

	ret = g_dbus_connection_call_sync (conn,
					"org.freedesktop.UDisks", // bus_name
					device_path,//"/org/freedesktop/UDisks/devices/sdd1", // object path
					"org.freedesktop.DBus.Properties", // interface name
					"Get", // method
					g_variant_new("(ss)","","DeviceMountPaths"), // parameters
					G_VARIANT_TYPE_ANY,		// reply type
					G_DBUS_CALL_FLAGS_NONE,		// flag
					-1,		// timeout
					NULL,	// cancellable
					&error);

	//g_variant_new("(ss)","org.freedesktop.devices.sdd1","DeviceMountPaths")
	if(ret == NULL){
		fprintf(stderr, "Error:: g_dbus_connection_call_sync() failed :: %s\n", error->message);
		return NULL;
	}

#ifdef DEBUG
	printf("[+] g_dbus_connection_call_sync() succeeded\n");
#endif


	if(g_variant_n_children(ret) <1){
		fprintf(stderr, "[-] Error :: query_mounted_dev() :: %ld parameters???\n",g_variant_n_children(ret) );
		return NULL;
	}

#ifdef DEBUG
	printf("\tparameter type : %s > with %ld sub parameters\n",g_variant_get_type_string(ret),g_variant_n_children(ret) );
#endif
	//printf("ret got %ld parameters\n", g_variant_n_children(ret));

	i = 0;
	j = 1;
	tmp = g_variant_get_child_value(ret,i);


	while(g_variant_is_of_type(tmp,G_VARIANT_TYPE_VARIANT) || g_variant_is_of_type(tmp,G_VARIANT_TYPE_STRING_ARRAY) ){

		if(g_variant_n_children(tmp) <1){

			// Check if the device is mounted.
			// TODO function bool_check_if_mounted().

			if(bMounted == FALSE ){
				fprintf(stderr, "[i] %s device not mounted yet\n",device_path);

			}else{
				fprintf(stderr, "%ld parameters???\n",g_variant_n_children(tmp) );
			}

			return NULL;
		}

		tmp = g_variant_get_child_value(tmp,i);
		printf("\t-> sub parameter %d -> type = %s\n",j, g_variant_get_type_string(tmp) );
		j++;

	}

	if(g_variant_is_of_type(tmp,G_VARIANT_TYPE_STRING) || g_variant_is_of_type(tmp,G_VARIANT_TYPE_OBJECT_PATH)){

		value = g_variant_get_string(tmp,NULL);
		printf("-> value = %s\n",value);
		return value;

	}else{

		printf("Error :: query_mounted_dev() couldn't get the string value !\n");
		return NULL;

	}

	return NULL;


}


static void mount_callback(GDBusConnection * conn, const gchar* sender_name, const gchar* object_path, const gchar * interface_name, const gchar* signal_name, GVariant* parameters, gpointer user_data)
{


	int i;
	GVariant * argv, *sub_argv;
	const gchar * mount_dir, * value;
	GDBusInterface * iface;

	mount_dir = NULL;
	printf("\n[i] Signal Info :: server_name %s :: object_path = %s :: interface_name = %s, signal = %s\n",sender_name,object_path,interface_name,signal_name);


	// If the device is removed , remove the watcher.
	if(strncmp(signal_name,"DeviceRemoved",13 ) == 0){
		printf("[+] mount_callback() :: Device removed ::  Removing fanotify on mount point \n");
		return;
	}


	if(parameters == NULL){
		fprintf(stderr, "[-] mount_callback() :: No parameters ??\n");
		return;
	}


	if(g_variant_n_children(parameters) <1){

#ifdef DEBUG
		fprintf(stderr, "[i] %ld child parameters???\n",g_variant_n_children(parameters) );
#endif

		return;
	}

#ifdef DEBUG
	printf("parameters type %s :: with >> %ld sub parameters\n", g_variant_get_type_string(parameters), g_variant_n_children(parameters));
#endif
	//printf(" got %ld sub parameters\n", g_variant_n_children(parameters));


	for(i = 0; i < g_variant_n_children(parameters) ; i++ ){

		argv = g_variant_get_child_value(parameters,i);
		//printf("-> sub parameter %d -> type = %s",i, g_variant_get_type_string(argv) );

		//sub_argv = g_variant_get_child_value(argv,argc);

		if(g_variant_is_of_type(argv,G_VARIANT_TYPE_STRING)){

			value = g_variant_get_string(argv,NULL);
			//printf("-> value = %s\n",value);

		}else if(g_variant_is_of_type(argv,G_VARIANT_TYPE_OBJECT_PATH)){

			value = g_variant_get_string(argv,NULL);
			//printf("-> value = %s\n",value);


			// Get the mount point path
			if(value != NULL)
				mount_dir = query_mounted_dev(conn, value);

			// add fanotify on this mount point.
			if(mount_dir != NULL){
				printf("[+] Adding fanotify on mount point ::  %s\n", mount_dir);
				add_fanotify_watch(mount_dir);
			}

			// TODO ...

		}else{
			printf("\n");
		}

		g_variant_unref(argv);
	}

	return;
}


static void subscribe_sig_mount(GDBusConnection * conn){

	guint sig_sub_id = 0;
	static guint mount_cb_data = 42;


	sig_sub_id = g_dbus_connection_signal_subscribe(conn,NULL,
							MOUNT_INTERFACE,//MOUNT_INTERFACE, // interface_name : D-Bus interface name to match on or NULL to match on all interfaces
							NULL, // member : D-Bus signal name to match on or NULL to match on all signals
							NULL, // object_path : Object path to match on or NULL to match on all object paths
							NULL, // arg0 : Contents of first string argument to match on or NULL to match on all kinds of arguments
							0,	// flag : Flags describing how to subscribe to the signal (currently unused).
							mount_callback, //  callback : Callback to invoke when there is a signal matching the requested data.
							&mount_cb_data, // user_data : User data to pass to callback.
							NULL // user_data_free_func : Function to free user_data with when subscription is removed or NULL.
		);

#ifdef DEBUG
	printf("[i] g_dbus_connection_signal_subscribe :: signal_id =  %d\n",sig_sub_id );
#endif

	return;

}

int add_mount_watch(){

	GDBusConnection * gd_conn;
	GError * error = NULL;

	g_type_init();
	gd_conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM,NULL,&error);

	if(gd_conn == NULL){
		fprintf(stderr, "[-] add_mount_watch() :: Error getting connection. Error: %s\n",error->message);
		return -1;
	}
	printf("[+] add_mount_watch() :: g_bus_get_sync() succeeded\n");

	// subscribe to mount signal
	subscribe_sig_mount(gd_conn);

	return 0;

}

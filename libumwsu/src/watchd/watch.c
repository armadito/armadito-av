#include <gio/gio.h>
#include <stdio.h>

#define UHURU_BUS_NAME          "com.uhuru.ScanService"
#define UHURU_INTERFACE_NAME    "com.uhuru.Scan"
#define UHURU_OBJECT_PATH       "/com/uhuru/ScanObject"

static void ping_done_cb(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  GVariant *ret, *sub_ret;
  GError *error = NULL;

  ret = g_dbus_connection_call_finish(G_DBUS_CONNECTION(source_object), res, &error);

  printf("parameters type %s\n", g_variant_get_type_string(ret));
  printf("got %ld parameters\n", g_variant_n_children(ret));

  sub_ret = g_variant_get_child_value(ret, 0);
  g_assert(g_variant_is_of_type(sub_ret, G_VARIANT_TYPE_STRING));

  printf("ping returned \"%s\"\n", g_variant_get_string(sub_ret, NULL));

  fprintf(stderr, "ping done %d\n", *(int *)user_data);
}

static guint ping_cb_data = 42;

static void call_ping(GDBusConnection *conn)
{
  g_dbus_connection_call(conn,
                         UHURU_BUS_NAME,
                         UHURU_OBJECT_PATH,
                         UHURU_INTERFACE_NAME,
                         "Ping",
                         NULL,
                         NULL,
                         G_DBUS_CALL_FLAGS_NONE,
                         -1,
                         NULL,
                         (GAsyncReadyCallback)ping_done_cb,
                         &ping_cb_data);
}

static void scan_done_cb(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  fprintf(stderr, "scan done\n");
}

static void call_scan(GDBusConnection *conn, const char *scan_path, int show_dialog)
{
  printf("calling scan(\"%s\", %d)\n", scan_path, show_dialog);

  g_dbus_connection_call(conn,
                         UHURU_BUS_NAME,
                         UHURU_OBJECT_PATH,
                         UHURU_INTERFACE_NAME,
                         "Scan",
                         g_variant_new("(sb)", scan_path, show_dialog),
                         NULL,
                         G_DBUS_CALL_FLAGS_NONE,
                         -1,
                         NULL,
                         (GAsyncReadyCallback)scan_done_cb,
                         NULL);
}

#define MOUNT_INTERFACE "org.gtk.Private.RemoteVolumeMonitor"
#define MOUNT_MEMBER    "MountAdded"

static void mount_cb(GDBusConnection *conn,
	      const gchar *sender_name,
	      const gchar *object_path,
	      const gchar *interface_name,
	      const gchar *signal_name,
	      GVariant *parameters,
	      gpointer user_data)
{
  int argc;
  GVariant *argv, *sub_argv;
  const gchar *mount_dir;

#ifdef DEBUG
  printf("mount sender_name %s object_path %s interface_name %s signal_name %s\n", sender_name, object_path, interface_name, signal_name);
#endif

  if (parameters == NULL) {
    fprintf(stderr, "No parameters???\n");
    return;
  }

  if (g_variant_n_children(parameters) < 3) {
    fprintf(stderr, "%ld parameters???\n", g_variant_n_children(parameters));
    return;
  }
  
#ifdef DEBUG
  printf("parameters type %s\n", g_variant_get_type_string(parameters));
  printf("got %ld parameters\n", g_variant_n_children(parameters));

  for(argc = 0; argc < g_variant_n_children(parameters); argc++) {
    argv = g_variant_get_child_value(parameters, argc);
    printf("parameter %d -> type %s\n", argc, g_variant_get_type_string(argv));
    g_variant_unref(argv);
  }
#endif

  argv = g_variant_get_child_value(parameters, 2);
  if (g_variant_n_children(argv) < 6) {
    fprintf(stderr, "%ld sub-parameters???\n", g_variant_n_children(argv));
    return;
  }

  sub_argv = g_variant_get_child_value(argv, 5);

  if (!g_variant_is_of_type(sub_argv, G_VARIANT_TYPE_STRING)) {
    fprintf(stderr, "sub-parameter 5 of type %s???\n", g_variant_get_type_string(sub_argv));
    return;
  }

  mount_dir = g_variant_get_string(sub_argv, NULL);

  call_scan(conn, mount_dir, 0);

  g_variant_unref(sub_argv);
  g_variant_unref(argv);
}


static void subscribe_sig_mount(GDBusConnection *conn)
{
  guint sig_sub_id = 0;
  static guint mount_cb_data = 42;

  sig_sub_id = g_dbus_connection_signal_subscribe (conn, NULL, MOUNT_INTERFACE, MOUNT_MEMBER, NULL, NULL, 0, mount_cb, &mount_cb_data, NULL);
}


void watch_loop(void)
{
  GDBusConnection *gd_conn;
  GError *error = NULL;
  GMainLoop *loop;

  gd_conn = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
  if (gd_conn == NULL) {
    fprintf(stderr, "Error getting connection. Error: %s\n", error->message);
    return;
  }

  subscribe_sig_mount(gd_conn);

  loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(loop);
}

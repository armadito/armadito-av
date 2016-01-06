#include <libuhuru/core.h>

#include "config/libuhuru-config.h"

#include "mount.h"

#include <gio/gio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MOUNT_INTERFACE        "org.gtk.Private.RemoteVolumeMonitor"
#define MOUNT_ADD_MEMBER       "MountAdded"
#define MOUNT_REMOVE_MEMBER    "MountRemoved"

struct mount_monitor {
  mount_cb_t cb;
  void *user_data;
  GDBusConnection *conn;
  guint sub_add_id;
  guint sub_remove_id;
};

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
  struct mount_monitor *m = (struct mount_monitor *)user_data;
  enum mount_event_type ev_type = !strcmp(signal_name, "MountAdded") ? EVENT_MOUNT : EVENT_UMOUNT;

#ifdef DEBUG
  printf("mount_cb sender_name %s object_path %s interface_name %s signal_name %s\n", sender_name, object_path, interface_name, signal_name);
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

  if (!strncmp(mount_dir, "file://", 7))
    (*m->cb)(ev_type, mount_dir + 7, m->user_data);

  g_variant_unref(sub_argv);
  g_variant_unref(argv);
}

static void mount_monitor_subscribe_signals(struct mount_monitor *m)
{
  m->sub_add_id = g_dbus_connection_signal_subscribe(m->conn, NULL, MOUNT_INTERFACE, MOUNT_ADD_MEMBER, NULL, NULL, 0, mount_cb, m, NULL);
  m->sub_remove_id = g_dbus_connection_signal_subscribe(m->conn, NULL, MOUNT_INTERFACE, MOUNT_REMOVE_MEMBER, NULL, NULL, 0, mount_cb, m, NULL);

  fprintf(stderr, "D-Bus connections %d %d", m->sub_add_id, m->sub_remove_id);
  /* uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_INFO, "D-Bus connections %d %d", m->sub_add_id, m->sub_remove_id); */
}

static void mount_monitor_unsubscribe_signals(struct mount_monitor *m)
{
  g_dbus_connection_signal_unsubscribe(m->conn, m->sub_add_id);
  g_dbus_connection_signal_unsubscribe(m->conn, m->sub_remove_id);
}

struct mount_monitor *mount_monitor_new(mount_cb_t cb, void *user_data)
{
  struct mount_monitor *m = malloc(sizeof(struct mount_monitor));
  GError *error = NULL;

  m->conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
  if (m->conn == NULL) {
    /* uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "error getting connection to D-Bus (%s)", error->message); */
    fprintf(stderr, "error getting connection to D-Bus (%s)", error->message);
    free(m);

    return NULL;
  }

  mount_monitor_subscribe_signals(m);

  m->cb = cb;
  m->user_data = user_data;

  return m;
}

void mount_monitor_free(struct mount_monitor *m)
{
  mount_monitor_unsubscribe_signals(m);

  g_dbus_connection_close_sync(m->conn, NULL, NULL);

  free(m);
}

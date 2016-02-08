#include <libuhuru/core.h>

#include "config/libuhuru-config.h"

#include "mount.h"
#include "onaccessmod.h"

#include <gio/gio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef DEBUG_DBUS_MESSAGE

#define MON_DBUS_TYPE          G_BUS_TYPE_SYSTEM
#define MON_DBUS_INTERFACE     "org.freedesktop.DBus.Properties"
#define MON_DBUS_MEMBER        "PropertiesChanged"

struct mount_monitor {
  GDBusConnection *conn;
  guint signal_sub_id;
  mount_cb_t cb;
  void *user_data;
};

#ifdef DEBUG_DBUS_MESSAGE
static void debug_variant(GVariant *parameters, const char *msg)
{
  int argc;

  fprintf(stderr, "%s: parameters %ld children type_string %s\n", msg, g_variant_n_children(parameters), g_variant_get_type_string(parameters));

  for(argc = 0; argc < g_variant_n_children(parameters); argc++) {
    GVariant *argv = g_variant_get_child_value(parameters, argc);

    fprintf(stderr, "parameter %d -> type %s\n", argc, g_variant_get_type_string(argv));

    g_variant_unref(argv);
  }
}
#endif

static int get_mount_event(GVariant *parameters, enum mount_event_type *pev_type, const char **pmount_point)
{
  GVariant *arg0, *arg1, *arg2, *val;
  const char *prop_name;
  const char **mount_points;
  gsize n_mount_points;

  if (g_variant_n_children(parameters) < 3)
    return 0;
    
  arg0 = g_variant_get_child_value(parameters, 0);
  arg1 = g_variant_get_child_value(parameters, 1);
  arg2 = g_variant_get_child_value(parameters, 2);

  if (!g_variant_is_of_type(arg0, G_VARIANT_TYPE_STRING)
      || !g_variant_is_of_type(arg1, G_VARIANT_TYPE_VARDICT)
      || !g_variant_is_of_type(arg2, G_VARIANT_TYPE_STRING_ARRAY))
    return 0;

  prop_name = g_variant_get_string(arg0, NULL);
  if (strcmp(prop_name, "org.freedesktop.UDisks2.Filesystem"))
    return 0;

  val = g_variant_lookup_value(arg1, "MountPoints", NULL);
  if (val == NULL)
    return 0;

#ifdef DEBUG_DBUS_MESSAGE
  debug_variant(val, "key value");
#endif

  mount_points = g_variant_get_bytestring_array(val, &n_mount_points);

  if (n_mount_points > 0) {
    *pmount_point = mount_points[0];
    *pev_type = EVENT_MOUNT;
  } else {
    *pev_type = EVENT_UMOUNT;
  }    

  g_free(mount_points);

  return 1;
}

static void mount_cb(GDBusConnection *conn,
		     const gchar *sender_name,
		     const gchar *object_path,
		     const gchar *interface_name,
		     const gchar *signal_name,
		     GVariant *parameters,
		     gpointer user_data)
{
  struct mount_monitor *m = (struct mount_monitor *)user_data;
  enum mount_event_type ev_type = EVENT_UNKNOWN;
  const char *mount_point = NULL;

#ifdef DEBUG_DBUS_MESSAGE
  fprintf(stderr, "mount_cb: sender_name %s object_path %s interface_name %s signal_name %s\n", sender_name, object_path, interface_name, signal_name);
#endif

  if (parameters == NULL) {
    fprintf(stderr, "No parameters???\n");
    return;
  }

#ifdef DEBUG_DBUS_MESSAGE
  debug_variant(parameters, "parameters");
#endif

  if (!get_mount_event(parameters, &ev_type, &mount_point))
    return;

  (*m->cb)(ev_type, mount_point, m->user_data);
}

static void mount_monitor_subscribe_signals(struct mount_monitor *m)
{
  m->signal_sub_id = g_dbus_connection_signal_subscribe(m->conn, NULL, MON_DBUS_INTERFACE, MON_DBUS_MEMBER, NULL, NULL, 0, mount_cb, m, NULL);

#ifdef DEBUG_DBUS_MESSAGE
  fprintf(stderr, "subscribed to D-Bus signal interface %s id=%d\n", MON_DBUS_INTERFACE, m->signal_sub_id);
#endif
}

static void mount_monitor_unsubscribe_signals(struct mount_monitor *m)
{
  g_dbus_connection_signal_unsubscribe(m->conn, m->signal_sub_id);
}

struct mount_monitor *mount_monitor_new(mount_cb_t cb, void *user_data)
{
  struct mount_monitor *m = malloc(sizeof(struct mount_monitor));
  GError *error = NULL;

  m->conn = g_bus_get_sync(MON_DBUS_TYPE, NULL, &error);

  if (m->conn == NULL) {
    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, MODULE_LOG_NAME ": " "error getting connection to D-Bus (%s)", error->message);
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

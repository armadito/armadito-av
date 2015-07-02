#include "uhuru-linux-gui-config.h"
#include "app.h"

#include <gio/gio.h>
#include <glib.h>
#include <libnotify/notify.h>
#include <glib/gi18n.h>
#include <stdlib.h>
#include <stdio.h>

#define UHURU_BUS_NAME          "com.uhuru.ScanService"
#define UHURU_INTERFACE_NAME    "com.uhuru.Scan"
#define UHURU_OBJECT_NAME      "/com/uhuru/ScanObject"

static const gchar introspection_xml[] =
  "<node>"
  "  <interface name='" UHURU_INTERFACE_NAME "'>"
  "    <method name='Ping'>"
  "      <arg direction='out' type='s' />"
  "    </method>"
  "    <method name='Scan'>"
  "      <arg direction='in'  type='s' name='scan_path' />"
  "      <arg direction='in'  type='u' name='show_dialog' />"
  "    </method>"
  "  </interface>"
  "</node>";

static GDBusNodeInfo *introspection_data = NULL;

static guint owner_id = 0;

static void on_bus_acquired(GDBusConnection *connection,
			    const gchar     *name,
			    gpointer         user_data);
static void on_name_acquired(GDBusConnection *connection,
			     const gchar     *name,
			     gpointer         user_data);
static void on_name_lost(GDBusConnection *connection,
			 const gchar     *name,
			 gpointer         user_data);

void uhuru_dbus_start(void)
{
  if (owner_id > 0)
    return;

  introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, NULL);

  owner_id = g_bus_own_name(G_BUS_TYPE_SESSION,
			    UHURU_BUS_NAME,
			    G_BUS_NAME_OWNER_FLAGS_NONE,
			    on_bus_acquired,
			    on_name_acquired,
			    on_name_lost,
			    NULL,
			    NULL);
}

void uhuru_dbus_stop(void)
{
  if (owner_id == 0)
    return;

  g_bus_unown_name(owner_id);

  g_dbus_node_info_unref(introspection_data);
}

static void handle_method_call(GDBusConnection       *connection,
			       const gchar           *sender,
			       const gchar           *object_path,
			       const gchar           *interface_name,
			       const gchar           *method_name,
			       GVariant              *parameters,
			       GDBusMethodInvocation *invocation,
			       gpointer               user_data)
{
  if (g_strcmp0(method_name, "Ping") == 0) {
    gchar *response = "Pong";
    
#ifdef DEBUG
    fprintf(stderr, "Got Ping\n");
#endif

    g_dbus_method_invocation_return_value(invocation, g_variant_new ("(s)", response));
    
    return;
  }

  if (g_strcmp0(method_name, "Scan") == 0) {
    const gchar *val_path;
    guint val_show_what;

    g_variant_get(parameters, "(&su)", &val_path, &val_show_what);

#ifdef DEBUG
    fprintf(stderr, "Got Scan path=%s show_what=%d\n", val_path, val_show_what);
#endif

    uhuru_app_scan(val_path, val_show_what);

    g_dbus_method_invocation_return_value (invocation, NULL);

    return;
  }
}

static const GDBusInterfaceVTable interface_vtable = {
  handle_method_call,
  NULL,
  NULL,
};

static void on_bus_acquired(GDBusConnection *connection,
			    const gchar     *name,
			    gpointer         user_data)
{
  guint id;

  id = g_dbus_connection_register_object(connection,
					 UHURU_OBJECT_NAME,
					 introspection_data->interfaces[0],
					 &interface_vtable,
					 NULL,
					 NULL,
					 NULL);
  g_assert(id > 0);
}

static void on_name_acquired(GDBusConnection *connection,
			     const gchar     *name,
			     gpointer         user_data)
{
}

static void on_name_lost(GDBusConnection *connection,
			 const gchar     *name,
			 gpointer         user_data)
{
  fprintf(stderr, "Lost bus %s\n", name);
}


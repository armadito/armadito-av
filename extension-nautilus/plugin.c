#include <glib.h>
/* Nautilus extension headers */
#include <libnautilus-extension/nautilus-extension-types.h>
#include <libnautilus-extension/nautilus-file-info.h>
#include <libnautilus-extension/nautilus-info-provider.h>
#include <libnautilus-extension/nautilus-menu-provider.h>
#include <libnautilus-extension/nautilus-property-page-provider.h>

#include <gtk/deprecated/gtktable.h>
#include <gtk/deprecated/gtkvbox.h>
#include <gtk/deprecated/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <string.h>
#include <time.h>

#include <gio/gio.h>
#include <stdio.h>
#include <stdlib.h>

#define UHURU_BUS_NAME          "com.uhuru.ScanService"
#define UHURU_INTERFACE_NAME    "com.uhuru.Scan"
#define UHURU_OBJECT_PATH       "/com/uhuru/ScanObject"

static GType provider_types[1];
static GType uhuru_extension_type;
static GObjectClass *parent_class;

typedef struct {
	GObject parent_slot;
} UhuruExtension;

typedef struct {
	GObjectClass parent_slot;
} UhuruExtensionClass;

/* nautilus extension interface */
void nautilus_module_initialize (GTypeModule  *module);
void nautilus_module_shutdown (void);
void nautilus_module_list_types (const GType **types, int *num_types);
GType uhuru_extension_get_type (void);

static void uhuru_extension_register_type (GTypeModule *module);

void show_popup(const char * msg);
void scan_done_cb(GObject *source_object, GAsyncResult *res, gpointer user_data);

/* menu filler */
static GList * uhuru_extension_get_file_items (NautilusMenuProvider *provider,
                GtkWidget *window,
                GList *files);
#if 0
static GList * uhuru_extension_get_background_items (NautilusMenuProvider *provider,
                GtkWidget *window,
                NautilusFileInfo *current_folder);
static GList * uhuru_extension_get_toolbar_items (NautilusMenuProvider *provider,
                GtkWidget *window,
                NautilusFileInfo *current_folder);
#endif

/* command callback */
static void do_stuff_cb (NautilusMenuItem *item, gpointer user_data);

void nautilus_module_initialize (GTypeModule  *module)
{
        uhuru_extension_register_type (module);

        provider_types[0] = uhuru_extension_get_type ();


}

void nautilus_module_shutdown (void)
{
        /* Any module-specific shutdown */
}

void nautilus_module_list_types (const GType **types,
                                 int *num_types)
{
        *types = provider_types;
        *num_types = G_N_ELEMENTS (provider_types);
}

GType uhuru_extension_get_type (void)
{
        return uhuru_extension_type;
}

static void uhuru_extension_instance_init (UhuruExtension *object)
{
}

static void uhuru_extension_class_init(UhuruExtensionClass *class)
{
	parent_class = g_type_class_peek_parent (class);
}

static void uhuru_extension_menu_provider_iface_init(
		NautilusMenuProviderIface *iface)
{
	iface->get_file_items = uhuru_extension_get_file_items;
}

static void uhuru_extension_register_type (GTypeModule *module)
{
        static const GTypeInfo info = {
                sizeof (UhuruExtensionClass),
                (GBaseInitFunc) NULL,
                (GBaseFinalizeFunc) NULL,
                (GClassInitFunc) uhuru_extension_class_init,
                NULL,
                NULL,
                sizeof (UhuruExtension),
                0,
                (GInstanceInitFunc) uhuru_extension_instance_init,
        };

	static const GInterfaceInfo menu_provider_iface_info = {
		(GInterfaceInitFunc) uhuru_extension_menu_provider_iface_init,
		NULL,
		NULL
	};

        uhuru_extension_type = g_type_module_register_type (module,
                             G_TYPE_OBJECT,
                             "uhuruExtension",
                             &info, 0);

	g_type_module_add_interface (module,
				     uhuru_extension_type,
				     NAUTILUS_TYPE_MENU_PROVIDER,
				     &menu_provider_iface_info);
}


static GList * uhuru_extension_get_file_items (NautilusMenuProvider *provider,
                GtkWidget *window,
                GList *files)
{
        NautilusMenuItem *item;
        GList *l;
        GList *ret;

	

#if 0
        /* This extension only operates on selections that include only
         * uhuru files */
        for (l = files; l != NULL; l = l->next) {
                NautilusFileInfo *file = NAUTILUS_FILE_INFO (l->data);
                if (!nautilus_file_is_mime_type (file, "application/x-uhuru")) {
                        return;
                }
        }
#endif


        for (l = files; l != NULL; l = l->next) {
                NautilusFileInfo *file = NAUTILUS_FILE_INFO (l->data);
                char *name;
                name = nautilus_file_info_get_name (file);
                g_print ("selected %s\n", name);
                g_free (name);
        }

        item = nautilus_menu_item_new ("uhuruExtension::do_stuff",
                                       "Scan With Uhuru",
                                       "Scan folders or files with uhuruAV",
                                       NULL /* icon name */);
        g_signal_connect (item, "activate", G_CALLBACK (do_stuff_cb), provider);
        g_object_set_data_full ((GObject*) item, "uhuru_extension_files",
                                nautilus_file_info_list_copy (files),
                                (GDestroyNotify)nautilus_file_info_list_free);
        ret = g_list_append (NULL, item);

        return ret;
}

/* samples for more menu fillers */
#if 0 
static GList * uhuru_extension_get_background_items (NautilusMenuProvider *provider,
                GtkWidget *window,
                NautilusFileInfo *current_folder)
{
        /* No background items */
        return NULL;
}

static GList * uhuru_extension_get_toolbar_items (NautilusMenuProvider *provider,
                GtkWidget *window,
                NautilusFileInfo *current_folder)
{
        /* No toolbar items */
        return NULL;
}
#endif


void show_popup(const char * msg){

	GtkWidget *dialog;


	dialog = gtk_message_dialog_new(NULL,
		  GTK_DIALOG_DESTROY_WITH_PARENT,
		  GTK_MESSAGE_ERROR,
		  GTK_BUTTONS_CLOSE,
		  "Uhuru Nautilus Extension: %s",
		  msg);

	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	return;

}

void scan_done_cb(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	fprintf(stderr, "scan done\n");

	show_popup("Scan done.");

}




static void call_scan(GDBusConnection *conn, const char *scan_path, int show_dialog)
{
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

static void do_stuff_cb (NautilusMenuItem *item,
                         gpointer user_data)
{
        GList *files;
        GList *l;
	/*GtkWidget *dialog;*/
	GDBusConnection *gd_conn;
	GError *error = NULL;

        files = g_object_get_data ((GObject *) item, "uhuru_extension_files");

        for (l = files; l != NULL; l = l->next) {
                NautilusFileInfo *file = NAUTILUS_FILE_INFO (l->data);
                char *name;
                name = nautilus_file_info_get_name (file);
                g_print ("Uhuru Scan with %s\n", name);
		
		/* Process to scan file*/
		/* send a dbus msg (scan order) to the interface */
		/* gtk_init(&argc, &argv); */

		/* connect to dbus server. */
		gd_conn = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
		if (gd_conn == NULL) {
			fprintf(stderr, "Uhuru nautilus extension :: Error getting connection. Error: %s\n", error->message);

			show_popup("Error:: Connecting to uhuru application failed !!");
			/* return ; */
		}


		show_popup(name);

		call_scan(gd_conn, name, 1);

                g_free (name);
        }
}

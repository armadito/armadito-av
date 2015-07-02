#include "model/scanmodel.h"
#include "model/scanmodel-marshall.h"
#include "utils/uhuru.h"

#include <libuhuru/scan.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

/* signaux émis: */
/* count(enum counttype) */
/* scanning(char *path) */
/* completed() */

char *scan_counter_type_str(enum scan_counter_type type)
{
  switch(type) {
#define M(T) case T: return #T
    M(NONE_COUNTER);
    M(TO_SCAN_COUNTER);
    M(SCANNED_COUNTER);
    M(MALWARE_COUNTER);
    M(SUSPICIOUS_COUNTER);
    M(UNHANDLED_COUNTER);
    M(CLEAN_COUNTER);
  }

  return "???";
}

struct _scan_model_private {
  gchar *scan_path;
  guint to_scan_counter;
  guint scanned_counter;
  guint malware_counter;
  guint suspicious_counter;
  guint unhandled_counter;
  guint clean_counter;

  GTimer *duration_timer;

  struct uhuru_scan *scan;

  GPtrArray *saved_reports;
};

static void scan_model_instance_init(GTypeInstance *instance, gpointer g_class)
{
  scan_model *self = (scan_model *)instance;

  self->private = g_new(scan_model_private, 1);

  self->private->scan_path = NULL;
  self->private->to_scan_counter = 0;
  self->private->scanned_counter = 0;
  self->private->malware_counter = 0;
  self->private->suspicious_counter = 0;
  self->private->unhandled_counter = 0;
  self->private->clean_counter = 0;

  self->private->scan = NULL;

  self->private->saved_reports = g_ptr_array_new();
}

enum {
  PROP_0,
  PROP_SCAN_PATH,
};

static void scan_model_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  scan_model *self = (scan_model *)object;

  switch(property_id) {
  case PROP_SCAN_PATH:
    g_free (self->private->scan_path);
    self->private->scan_path = g_value_dup_string(value);
#if 0
    fprintf(stderr, "scan model: %s\n",self->private->scan_path);
#endif
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void scan_model_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  scan_model *self = (scan_model *)object;

  switch (property_id) {
  case PROP_SCAN_PATH:
    g_value_set_string(value, self->private->scan_path);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object,property_id,pspec);
    break;
  }
}

static void scan_model_class_init(gpointer g_class, gpointer g_class_data)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(g_class);
  scan_model_class *cl = SCAN_MODEL_CLASS(g_class);
  GParamSpec *pspec;

  gobject_class->get_property = scan_model_get_property;
  gobject_class->set_property = scan_model_set_property;

  pspec = g_param_spec_string("scan-path",
			      "scan-path",
			      "Set/get scan path",
			      "no-name-set" /* default value */,
			      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
  g_object_class_install_property(gobject_class, PROP_SCAN_PATH, pspec);

  cl->count_changed_signal_id = g_signal_new("count_changed",
					     G_TYPE_FROM_CLASS (g_class),
					     G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
					     0,
					     NULL /* accumulator */,
					     NULL /* accu_data */,
					     g_cclosure_user_marshal_VOID__UINT_UINT,
					     G_TYPE_NONE /* return_type */,
					     2     /* n_params */,
					     G_TYPE_UINT,
					     G_TYPE_UINT);

  cl->scanning_signal_id = g_signal_new("scanning",
					G_TYPE_FROM_CLASS (g_class),
					G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
					0,
					NULL /* accumulator */,
					NULL /* accu_data */,
					g_cclosure_user_marshal_VOID__UINT_UINT_STRING_DOUBLE,
					G_TYPE_NONE /* return_type */,
					4     /* n_params */,
					G_TYPE_UINT,
					G_TYPE_UINT,
					G_TYPE_STRING,
					G_TYPE_DOUBLE);

  cl->completed_signal_id = g_signal_new("completed",
					 G_TYPE_FROM_CLASS (g_class),
					 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
					 0,
					 NULL /* accumulator */,
					 NULL /* accu_data */,
					 g_cclosure_marshal_VOID__VOID,
					 G_TYPE_NONE /* return_type */,
					 0     /* n_params */);
}

GType scan_model_get_type(void)
{
  static GType type = 0;

  if(type == 0) {
    static const GTypeInfo info = {
      sizeof(scan_model_class),
      NULL,   /* base_init */
      NULL,   /* base_finalize */
      scan_model_class_init,   /* class_init */
      NULL,   /* class_finalize */
      NULL,   /* class_data */
      sizeof(scan_model),
      0,      /* n_preallocs */
      scan_model_instance_init    /* instance_init */
    };
    type = g_type_register_static(G_TYPE_OBJECT, "scan_model_type", &info, 0);
  }

  return type;
}

static int count_files(const char *path)
{
  GDir *dir;
  GError *error = NULL;
  const gchar *entry;
  GString *full_entry_path = g_string_new("");
  int count = 0;

  if (g_file_test(path, G_FILE_TEST_IS_REGULAR) && !g_file_test(path, G_FILE_TEST_IS_SYMLINK)) {
    /* fprintf(stderr, "%s: FILE %d\n", path, 1); */
    return 1;
  }

  if (!g_file_test(path, G_FILE_TEST_IS_DIR)) {
    /* fprintf(stderr, "%s: NOT DIR %d\n", path, 0); */
    return 0;
  }

  if (g_file_test(path, G_FILE_TEST_IS_SYMLINK)) {
    /* fprintf(stderr, "%s: SYMLINK %d\n", path, 0); */
    return 0;
  }

  dir = g_dir_open(path, 0, &error);
  if (dir == NULL) {
    fprintf(stderr, "Error reading directory %s: %s\n", path, error->message);
    /* fprintf(stderr, "%s: ERROR %d\n", path, 0); */
    return 0;
  }

  while((entry = g_dir_read_name(dir)) != NULL) {
    g_string_printf(full_entry_path, "%s/%s", path, entry);
    count += count_files(full_entry_path->str);
  }

  g_dir_close(dir);
  g_string_free(full_entry_path, TRUE);

  /* fprintf(stderr, "%s: DIR %d\n", path, count); */

  return count;
}

static void scan_model_save_report(scan_model *self, struct uhuru_report *report)
{
  struct uhuru_report *clone = (struct uhuru_report *)malloc(sizeof(struct uhuru_report));

  clone->path = strdup(report->path);
  clone->status = report->status;
  clone->action = report->action;
  if (report->mod_name != NULL)
    clone->mod_name = strdup(report->mod_name);
  else
    clone->mod_name = NULL;
  if (report->mod_report != NULL)
    clone->mod_report = strdup(report->mod_report);
  else
    clone->mod_report = NULL;

  g_ptr_array_add(self->private->saved_reports, clone);
}

static void scan_model_callback(struct uhuru_report *report, void *callback_data)
{
  scan_model *self = (scan_model *)callback_data;
  enum scan_counter_type which_counter = NONE_COUNTER;
  guint counter;
  guint previous_scanned_counter = self->private->scanned_counter;
  gdouble scan_duration;

  switch(report->status) {
  case UHURU_MALWARE:
    which_counter = MALWARE_COUNTER;
    counter = ++self->private->malware_counter;
    self->private->scanned_counter++;
    break;
  case UHURU_SUSPICIOUS:
    which_counter = SUSPICIOUS_COUNTER;
    counter = ++self->private->suspicious_counter;
    self->private->scanned_counter++;
    break;
  case UHURU_EINVAL:
  case UHURU_IERROR:
    break;
  case UHURU_UNKNOWN_FILE_TYPE:
  case UHURU_UNDECIDED:
    which_counter = UNHANDLED_COUNTER;
    counter = ++self->private->unhandled_counter;
    self->private->scanned_counter++;
    break;
  case UHURU_WHITE_LISTED:
  case UHURU_CLEAN:
    which_counter = CLEAN_COUNTER;
    counter = ++self->private->clean_counter;
    self->private->scanned_counter++;
    break;
  }

  g_signal_emit(self, SCAN_MODEL_GET_CLASS(self)->count_changed_signal_id, 0, which_counter, counter);

  if (previous_scanned_counter != self->private->scanned_counter)
    g_signal_emit(self, SCAN_MODEL_GET_CLASS(self)->count_changed_signal_id, 0, SCANNED_COUNTER, self->private->scanned_counter);

  scan_duration = g_timer_elapsed(self->private->duration_timer, NULL);
  g_signal_emit(self, SCAN_MODEL_GET_CLASS(self)->scanning_signal_id, 0, report->status, report->action, report->path, scan_duration);

  if (report->status != UHURU_WHITE_LISTED && report->status != UHURU_CLEAN)
    scan_model_save_report(self, report);
}

gboolean scan_channel_io_func(GIOChannel *source, GIOCondition condition, gpointer data)
{
  scan_model *self = (scan_model *)data;

  if (uhuru_scan_run(self->private->scan) != UHURU_SCAN_CONTINUE) {
    fprintf(stderr, "finished!\n");

    g_signal_emit(self, SCAN_MODEL_GET_CLASS(self)->completed_signal_id, 0);

    g_timer_stop(self->private->duration_timer);
    g_timer_destroy(self->private->duration_timer);

    return FALSE;
  }

  return TRUE;
}

void scan_model_scan(scan_model *self)
{
  enum uhuru_scan_flags flags = UHURU_SCAN_RECURSE;
  GIOChannel *channel;

  self->private->duration_timer = g_timer_new();

  self->private->scan = uhuru_scan_new(uhuru_handle(), self->private->scan_path, flags);

  uhuru_scan_add_callback(self->private->scan, scan_model_callback, self);

  uhuru_scan_start(self->private->scan);

  g_timer_start(self->private->duration_timer);

  self->private->to_scan_counter = count_files(self->private->scan_path);

  g_signal_emit(self, SCAN_MODEL_GET_CLASS(self)->count_changed_signal_id, 0, TO_SCAN_COUNTER, self->private->to_scan_counter);

  channel = g_io_channel_unix_new(uhuru_scan_get_poll_fd(self->private->scan));
  g_io_add_watch(channel, G_IO_IN, scan_channel_io_func, self);

  /* uhuru_scan_free(scan); */
}

void scan_model_reemit(scan_model *self)
{
  g_signal_emit(self, SCAN_MODEL_GET_CLASS(self)->count_changed_signal_id, 0, TO_SCAN_COUNTER, self->private->to_scan_counter);
  g_signal_emit(self, SCAN_MODEL_GET_CLASS(self)->count_changed_signal_id, 0, SCANNED_COUNTER, self->private->scanned_counter);
  g_signal_emit(self, SCAN_MODEL_GET_CLASS(self)->count_changed_signal_id, 0, MALWARE_COUNTER, self->private->malware_counter);
  g_signal_emit(self, SCAN_MODEL_GET_CLASS(self)->count_changed_signal_id, 0, SUSPICIOUS_COUNTER, self->private->suspicious_counter);
  g_signal_emit(self, SCAN_MODEL_GET_CLASS(self)->count_changed_signal_id, 0, UNHANDLED_COUNTER, self->private->unhandled_counter);
  g_signal_emit(self, SCAN_MODEL_GET_CLASS(self)->count_changed_signal_id, 0, CLEAN_COUNTER, self->private->clean_counter);

  /* g_signal_emit(self, SCAN_MODEL_GET_CLASS(self)->scanning_signal_id, 0, report->status, report->action, report->path, scan_duration); */
}


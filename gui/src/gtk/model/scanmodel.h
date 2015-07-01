#ifndef _MODEL_SCANMODEL_H_
#define _MODEL_SCANMODEL_H_

#include <glib-object.h>

GType scan_model_get_type(void);

typedef struct _scan_model scan_model;
typedef struct _scan_model_class scan_model_class;
typedef struct _scan_model_private scan_model_private;

#define SCAN_MODEL_TYPE           (scan_model_get_type())
#define SCAN_MODEL(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), SCAN_MODEL_TYPE, scan_model))
#define SCAN_MODEL_CLASS(cl)      (G_TYPE_CHECK_CLASS_CAST((cl), SCAN_MODEL_TYPE, scan_model_class))
#define SCAN_MODEL_IS_TYPE(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SCAN_MODEL_TYPE))
#define SCAN_MODEL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), SCAN_MODEL_TYPE, scan_model_class))

struct _scan_model {
  GObject parent;
  scan_model_private *private;
};

struct _scan_model_class {
  GObjectClass parent;

  guint count_changed_signal_id;
  guint scanning_signal_id;
  guint completed_signal_id;
};

enum scan_counter_type {
  NONE_COUNTER,
  TO_SCAN_COUNTER, 
  SCANNED_COUNTER,
  MALWARE_COUNTER,
  SUSPICIOUS_COUNTER,
  UNHANDLED_COUNTER,
  CLEAN_COUNTER,
};

char *scan_counter_type_str(enum scan_counter_type type);

void scan_model_reemit(scan_model *model);

void scan_model_scan(scan_model *model);

#endif

#ifndef _MODEL_CLOCKMODEL_H_
#define _MODEL_CLOCKMODEL_H_

#include <glib-object.h>

GType clock_model_get_type(void);

typedef struct _clock_model clock_model;
typedef struct _clock_model_private clock_model_private;

#define CLOCK_MODEL_TYPE           (clock_model_get_type())
#define CLOCK_MODEL(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), CLOCK_MODEL_TYPE, clock_model))

struct _clock_model {
  GObject parent;
  clock_model_private *private;
};

enum clock_counter_type {
  HOUR_COUNTER,
  MINUTE_COUNTER, 
  SECOND_COUNTER,
};

char *clock_counter_type_str(enum clock_counter_type type);

void clock_model_clock(clock_model *model, int use_thread);

#endif

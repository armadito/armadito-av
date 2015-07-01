#ifndef _MODEL_COUNTER_H_
#define _MODEL_COUNTER_H_

#include <glib-object.h>

GType counter_get_type(void);

#define COUNTER_TYPE           (counter_get_type())
#define COUNTER(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), COUNTER_TYPE, Counter))
#define COUNTER_CLASS(cl)      (G_TYPE_CHECK_CLASS_CAST((cl), COUNTER_TYPE, CounterClass))
#define COUNTER_IS_TYPE(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), COUNTER_TYPE))
#define COUNTER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), COUNTER_TYPE, CounterClass))

typedef struct _Counter Counter;
typedef struct _CounterClass CounterClass;
typedef struct _CounterPrivate CounterPrivate;

struct _Counter {
  GObject parent;
  CounterPrivate *private;
};

struct _CounterClass {
  GObjectClass parent;

  guint changed_signal_id;
};

void counter_inc(Counter *self);

#endif

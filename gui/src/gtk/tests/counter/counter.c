#include "model/counter.h"

#include <limits.h>

struct _CounterPrivate {
  guint count;
};

enum {
  PROP_0,
  PROP_COUNTER_VALUE,
};

static void counter_instance_init(GTypeInstance *instance, gpointer g_class)
{
  Counter *self = (Counter *)instance;

  self->private = g_new(CounterPrivate, 1);
  self->private->count = 0;
}

static void counter_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  Counter *self = (Counter *)object;

  switch(property_id) {
  case PROP_COUNTER_VALUE:
    /* self->private->papa_number = g_value_get_uchar(value); */
    /* g_print("papa: %u\n",self->private->papa_number); */
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void counter_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  Counter *self = (Counter *)object;

  switch (property_id) {
  case PROP_COUNTER_VALUE:
    g_value_set_uint(value, self->private->count);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object,property_id,pspec);
    break;
  }
}

static void counter_class_init(gpointer g_class, gpointer g_class_data)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(g_class);
  CounterClass *cl = COUNTER_CLASS(g_class);
  GParamSpec *pspec;

  gobject_class->get_property = counter_get_property;

  pspec = g_param_spec_uint("value",
			    "Counter value",
			    "Get counter's value",
			    0  /* minimum value */,
			    UINT_MAX /* maximum value */,
			    0  /* default value */,
			    G_PARAM_READABLE);
  g_object_class_install_property(gobject_class, PROP_COUNTER_VALUE, pspec);

  cl->changed_signal_id = g_signal_new("changed",
				       G_TYPE_FROM_CLASS (g_class),
				       G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
				       0,
				       NULL /* accumulator */,
				       NULL /* accu_data */,
				       g_cclosure_marshal_VOID__UINT,
				       G_TYPE_NONE /* return_type */,
				       1     /* n_params */,
				       G_TYPE_UINT);

}

GType counter_get_type(void)
{
  static GType type = 0;

  if(type == 0) {
    static const GTypeInfo info = {
      sizeof(CounterClass),
      NULL,   /* base_init */
      NULL,   /* base_finalize */
      counter_class_init,   /* class_init */
      NULL,   /* class_finalize */
      NULL,   /* class_data */
      sizeof(Counter),
      0,      /* n_preallocs */
      counter_instance_init    /* instance_init */
    };
    type = g_type_register_static(G_TYPE_OBJECT, "CounterType", &info, 0);
  }

  return type;
}

void counter_inc(Counter *self)
{
  self->private->count++;

  g_signal_emit(self, COUNTER_GET_CLASS(self)->changed_signal_id, 0, self->private->count);
}


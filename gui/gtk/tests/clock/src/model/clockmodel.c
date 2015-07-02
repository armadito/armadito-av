#include "model/clockmodel.h"
#include "model/clockmodel-marshall.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define CLOCK_MODEL_CLASS(cl)      (G_TYPE_CHECK_CLASS_CAST((cl), CLOCK_MODEL_TYPE, clock_model_class))
#define CLOCK_MODEL_IS_TYPE(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CLOCK_MODEL_TYPE))
#define CLOCK_MODEL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), CLOCK_MODEL_TYPE, clock_model_class))

typedef struct _clock_model_class clock_model_class;

struct _clock_model_class {
  GObjectClass parent;

  guint count_changed_signal_id;
};

char *clock_counter_type_str(enum clock_counter_type type)
{
  switch(type) {
#define M(T) case T: return #T
    M(HOUR_COUNTER);
    M(MINUTE_COUNTER);
    M(SECOND_COUNTER);
  }

  return "???";
}

struct _clock_model_private {
  guint period;
  guint hour_counter;
  guint minute_counter;
  guint second_counter;
  GThread *clock_thread;
};

static void clock_model_instance_init(GTypeInstance *instance, gpointer g_class)
{
  clock_model *self = (clock_model *)instance;

  self->private = g_new(clock_model_private, 1);

  self->private->period = 0;
  self->private->hour_counter = 0;
  self->private->minute_counter = 0;
  self->private->second_counter = 0;

  self->private->clock_thread = NULL;
}

enum {
  PROP_0,
  PROP_CLOCK_PERIOD,
};

static void clock_model_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  clock_model *self = (clock_model *)object;

  switch(property_id) {
  case PROP_CLOCK_PERIOD:
    self->private->period = g_value_get_uint(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void clock_model_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  clock_model *self = (clock_model *)object;

  switch (property_id) {
  case PROP_CLOCK_PERIOD:
    g_value_set_uint(value, self->private->period);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object,property_id,pspec);
    break;
  }
}

static void clock_model_class_init(gpointer g_class, gpointer g_class_data)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(g_class);
  clock_model_class *cl = CLOCK_MODEL_CLASS(g_class);
  GParamSpec *pspec;

  gobject_class->get_property = clock_model_get_property;
  gobject_class->set_property = clock_model_set_property;

  pspec = g_param_spec_uint("period",
			    "period",
			    "Set/get clock period",
			    0  /* minimum value */,
			    UINT_MAX /* maximum value */,
			    0  /* default value */,
			    G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
  g_object_class_install_property(gobject_class, PROP_CLOCK_PERIOD, pspec);

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
}

GType clock_model_get_type(void)
{
  static GType type = 0;

  if(type == 0) {
    static const GTypeInfo info = {
      sizeof(clock_model_class),
      NULL,   /* base_init */
      NULL,   /* base_finalize */
      clock_model_class_init,   /* class_init */
      NULL,   /* class_finalize */
      NULL,   /* class_data */
      sizeof(clock_model),
      0,      /* n_preallocs */
      clock_model_instance_init    /* instance_init */
    };
    type = g_type_register_static(G_TYPE_OBJECT, "clock_model_type", &info, 0);
  }

  return type;
}

static void clock_tick(clock_model *self)
{
  self->private->second_counter = (self->private->second_counter + 1) % 60;
  g_signal_emit(self, CLOCK_MODEL_GET_CLASS(self)->count_changed_signal_id, 0, SECOND_COUNTER, self->private->second_counter);

  if (self->private->second_counter == 0) {
    self->private->minute_counter = (self->private->minute_counter + 1) % 60;
    g_signal_emit(self, CLOCK_MODEL_GET_CLASS(self)->count_changed_signal_id, 0, MINUTE_COUNTER, self->private->minute_counter);

    if (self->private->minute_counter == 0) {
      self->private->hour_counter = (self->private->hour_counter + 1) % 24;
      g_signal_emit(self, CLOCK_MODEL_GET_CLASS(self)->count_changed_signal_id, 0, HOUR_COUNTER, self->private->hour_counter);
    }
  }
}

static gpointer clock_thread_func(gpointer data)
{
  clock_model *self = (clock_model *)data;

  printf("thread: %p\n", g_thread_self());

  while(TRUE) {
    usleep(self->private->period * 1000);

    clock_tick(self);
  }

  return NULL;
}

static void clock_child_process(int pipe_fd, int period)
{
  fprintf(stderr, "starting child process %d %d\n", pipe_fd, period);

  while(TRUE) {
    char c;

    usleep(period);

    write(pipe_fd, &c, 1);
  }
}

gboolean clock_channel_io_func(GIOChannel *source, GIOCondition condition, gpointer data)
{
  clock_model *self = (clock_model *)data;
  char tmp[1];
  gsize count;
  GError *error = NULL;

  if (g_io_channel_read_chars(source, tmp, 1, &count, &error) != G_IO_STATUS_NORMAL) {
    fprintf(stderr, "Error reading from pipe. Error: %s\n", error->message);
    return FALSE;
  }

  clock_tick(self);

  return TRUE;
}

void clock_model_clock(clock_model *self, int use_thread)
{
  if (use_thread) {
  self->private->clock_thread = g_thread_new("clock", clock_thread_func, self);
  } else { /* use child process and pipe */
    int pipefd[2];
    pid_t child_pid;

    if (pipe(pipefd) == -1) {
      perror("pipe");
      exit(EXIT_FAILURE);
    }

    child_pid = fork();
    if (child_pid == -1) {
      perror("fork");
      exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {    /* Child write to pipe */
      close(pipefd[0]);          /* Close unused write end */

      clock_child_process(pipefd[1], self->private->period * 1000);
      
      exit(EXIT_SUCCESS);
    } else {
      GIOChannel *channel;

      close(pipefd[1]);
      channel = g_io_channel_unix_new(pipefd[0]);
      g_io_add_watch(channel, G_IO_IN, clock_channel_io_func, self);
    }
  }
}



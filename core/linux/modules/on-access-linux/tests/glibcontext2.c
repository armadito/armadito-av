
/* Main function for the background thread, thread1. */
static gpointer
thread1_main (gpointer user_data)
{
  GMainContext *thread1_main_context = user_data;
  GMainLoop *main_loop;

  /* Set up the thread’s context and run it forever. */
  g_main_context_push_thread_default (thread1_main_context);

  main_loop = g_main_loop_new (thread1_main_context, FALSE);
  g_main_loop_run (main_loop);
  g_main_loop_unref (main_loop);

  g_main_context_pop_thread_default (thread1_main_context);
  g_main_context_unref (thread1_main_context);

  return NULL;
}

/* A data closure structure to carry multiple variables between
 * threads. */
typedef struct {
  gchar   *some_string;  /* owned */
  guint    some_int;
  GObject *some_object;  /* owned */
} MyFuncData;

static void
my_func_data_free (MyFuncData *data)
{
  g_free (data->some_string);
  g_clear_object (&data->some_object);
  g_free (data);
}

static void
my_func (const gchar *some_string,
         guint        some_int,
         GObject     *some_object)
{
  /* Do something long and CPU intensive! */
}

/* Convert an idle callback into a call to my_func(). */
static gboolean
my_func_idle (gpointer user_data)
{
  MyFuncData *data = user_data;

  my_func (data->some_string, data->some_int, data->some_object);

  return G_SOURCE_REMOVE;
}

/* Function to be called in the main thread to schedule a call to
 * my_func() in thread1, passing the given parameters along. */
static void
invoke_my_func (GMainContext *thread1_main_context,
                const gchar  *some_string,
                guint         some_int,
                GObject      *some_object)
{
  GSource *idle_source;
  MyFuncData *data;

  /* Create a data closure to pass all the desired variables
   * between threads. */
  data = g_new0 (MyFuncData, 1);
  data->some_string = g_strdup (some_string);
  data->some_int = some_int;
  data->some_object = g_object_ref (some_object);

  /* Create a new idle source, set my_func() as the callback with
   * some data to be passed between threads, bump up the priority
   * and schedule it by attaching it to thread1’s context. */
  idle_source = g_idle_source_new ();
  g_source_set_callback (idle_source, my_func_idle, data,
                         (GDestroyNotify) my_func_data_free);
  g_source_set_priority (idle_source, G_PRIORITY_DEFAULT);
  g_source_attach (idle_source, thread1_main_context);
  g_source_unref (idle_source);
}

/* Main function for the main thread. */
static void
main (void)
{
  GThread *thread1;
  GMainContext *thread1_main_context;

  /* Spawn a background thread and pass it a reference to its
   * GMainContext. Retain a reference for use in this thread
   * too. */
  thread1_main_context = g_main_context_new ();
  g_thread_new ("thread1", thread1_main,
                g_main_context_ref (thread1_main_context));

  /* Maybe set up your UI here, for example. */

  /* Invoke my_func() in the other thread. */
  invoke_my_func (thread1_main_context,
                  "some data which needs passing between threads",
                  123456, some_object);

  /* Continue doing other work. */
}

/* other solution: */

static void
invoke_my_func (GMainContext *thread1_main_context,
                const gchar  *some_string,
                guint         some_int,
                GObject      *some_object)
{
  MyFuncData *data;

  /* Create a data closure to pass all the desired variables
   * between threads. */
  data = g_new0 (MyFuncData, 1);
  data->some_string = g_strdup (some_string);
  data->some_int = some_int;
  data->some_object = g_object_ref (some_object);

  /* Invoke the function. */
  g_main_context_invoke_full (thread1_main_context,
                              G_PRIORITY_DEFAULT, my_func_idle,
                              data,
                              (GDestroyNotify) my_func_data_free);
}

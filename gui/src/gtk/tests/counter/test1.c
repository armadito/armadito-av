#include "counter.h"

#include <stdio.h>

static void print_prop_value(GObject *obj)
{
  guint val;

  g_object_get(obj, "value", &val, NULL);
  printf("property value is %d\n", val);
}

static void test1(void)
{
  GObject *c1;

  c1 = g_object_new(COUNTER_TYPE, NULL);

  print_prop_value(c1);
  counter_inc(COUNTER(c1));
  print_prop_value(c1);
  counter_inc(COUNTER(c1));
  print_prop_value(c1);
}

static void counter_changed_test(GObject *obj, guint value, gpointer user_data)
{
  g_assert(user_data == NULL);
  printf("callback value is %d\n", value);
}

static void test2(void)
{
  GObject *c2;

  c2 = g_object_new(COUNTER_TYPE, NULL);

  g_signal_connect(c2, "changed", (GCallback)counter_changed_test, NULL);

  counter_inc(COUNTER(c2));
  counter_inc(COUNTER(c2));
  counter_inc(COUNTER(c2));
  counter_inc(COUNTER(c2));
}

int main(int argc, char **argv)
{
  test1();
  test2();

  return 0;
}

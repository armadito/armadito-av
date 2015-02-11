#include "protocol.h"

#include <assert.h>
#include <stdio.h>

static void cb_scan_start(struct protocol_handler *h, void *data)
{
  printf("callback scan_start: %d\n", *(int *)data);
  printf("command: %s\n", protocol_handler_cmd(h));
  printf("header: %s\n", protocol_handler_header_value(h, "Path"));
  printf("header: %s\n", protocol_handler_header_value(h, "File-count"));
}

static void cb_scan_file(struct protocol_handler *h, void *data)
{
  printf("callback scan_file: %s\n", (char *)data);
  printf("command: %s\n", protocol_handler_cmd(h));
  printf("header: %s\n", protocol_handler_header_value(h, "Path"));
  printf("header: %s\n", protocol_handler_header_value(h, "Status"));
  printf("header: %s\n", protocol_handler_header_value(h, "X-Status"));
  printf("header: %s\n", protocol_handler_header_value(h, "Action"));
}

static void cb_scan_end(struct protocol_handler *h, void *data)
{
  printf("callback scan_end: %s\n", (char *)data);
  printf("command: %s\n", protocol_handler_cmd(h));
}

static int data1 = 42;
static char *data2 = "foobar";
static char *data3 = "zob";

int main(int argc, char **argv)
{
  struct protocol_handler *h;
  FILE *in = fopen(argv[1], "r");
  char c;

  assert(in != NULL);

  h = protocol_handler_new();

  protocol_handler_add_callback(h, "SCAN_START", cb_scan_start, &data1);
  protocol_handler_add_callback(h, "SCAN_FILE", cb_scan_file, data2);
  protocol_handler_add_callback(h, "SCAN_END", cb_scan_end, data3);

  while((c = fgetc(in)) != EOF)
    protocol_handler_input_char(h, c);

  return 0;
}

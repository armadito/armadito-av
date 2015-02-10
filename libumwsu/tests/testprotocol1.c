#include "protocol.h"

#include <assert.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  struct protocol_handler *h;
  FILE *in = fopen(argv[1], "r");
  char c;

  assert(in != NULL);

  h = protocol_handler_new();

  while((c = fgetc(in)) != EOF)
    protocol_handler_input_char(h, c);

  return 0;
}

#include "protocol.h"

#include "protocol.h"

#include <glib.h>
#include <stdlib.h>

struct protocol_command {
  GString *command;
  GHashTable *headers;
};

enum protocol_parser_state {
  E_COMMAND,
  IN_COMMAND,
  E_HEADER,
  IN_HEADER,
  E_VALUE,
  IN_VALUE,
};

struct protocol_parser {
  enum protocol_parser_state state;
};

#define STAY(A) (A)
#define MOVE(A,S) do { A; state = S; } while (0)

#if 0
void protocol_decode(struct protocol_hander *h)
{
  switch(state) {
  case IN_COMMAND:
    if (isalpha(c))
      append_to_command(c);
    else if (c == '\n') {
      append_to_command('\0');
      state = IN_HEADER_NAME;
    } else
      protocol_error(c);
    break;
  case IN_HEADER_NAME:
    if (isalpha(c))
      append_to_header_name(c);
    else if (c == ':') {
      append_to_header_name('\0');
      state = IN_HEADER_VALUE_SKIP_SPACE;
    } else if (c == '\n') {
      end_of_command();   /* includes reset_headers(); */
      state = IN_COMMAND;
    } else
      protocol_error(c);
    break;
  case IN_HEADER_VALUE_SKIP_SPACE:
    if (isalpha(c)) {
      append_to_header_value(c);
      state = IN_HEADER_VALUE;
    } else if (is_space(c))
      ;
    else
      protocol_error(c);
    break;
  case IN_HEADER_VALUE:
    if (isalpha(c)) {
      append_to_header_value(c);
    } else if (c == '\n') {
      append_to_header_value('\0');
      next_header();
      state = IN_HEADER_NAME;
    } else
      protocol_error(c);
    break;
  }

}
#endif

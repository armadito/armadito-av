#include "protocol.h"

#include <assert.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  struct protocol_handler *h;

  h = protocol_handler_new(0, 1);

  protocol_handler_output_message(h, "PING", NULL);
  protocol_handler_output_message(h, "PONG", NULL);
  protocol_handler_output_message(h, "SCAN", 
				  "Path", "/var/tmp/foo", 
				  NULL);
  protocol_handler_output_message(h, "SCAN_START", 
				  "Path", "/var/tmp/foo", 
				  "File-count", "42", 
				  NULL);
  protocol_handler_output_message(h, "SCAN_FILE", 
				  "Path", "/var/tmp/foo/bar/zob1", 
				  "Status", "OK", 
				  NULL);
  protocol_handler_output_message(h, "SCAN_FILE", 
				  "Path", "/var/tmp/foo/bar/zob2", 
				  "Status", "MALWARE",
				  "X-Status", "GrosTrojan",
				  "Action", "QUARANTINE",
				  NULL);
  protocol_handler_output_message(h, "SCAN_END", NULL);
  protocol_handler_output_message(h, "STATS", NULL);
  protocol_handler_output_message(h, "STATS", 
				  "Stat", "toto", 
				  NULL);

  return 0;
}

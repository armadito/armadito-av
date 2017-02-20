{
  "title": "JSON schema for Armadito JSON-RPC",
  "$schema": "http://json-schema.org/draft-04/schema#",

  "type": "object",
  "oneOf": [
#define JSON_SCHEMA_ROOT
#include "rpcdefs.h"
  ],
  "definitions": {
#define JSON_SCHEMA_DEFINITIONS
#include "rpcdefs.h"
  }
}

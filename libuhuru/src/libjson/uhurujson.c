#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include "uhurujson.h"

#include <assert.h>
#include <json.h>
#include <stdlib.h>

/* check if a struct json_tokener * can be used */
/* if called from multiple threads, no */

struct uhuru_json_handler {
  struct json_tokener *tokener;
};

struct uhuru_json_handler *uhuru_json_handler_new(void)
{
  struct uhuru_json_handler *jh = malloc(sizeof(struct uhuru_json_handler));

  jh->tokener = json_tokener_new();
  assert(jh->tokener != NULL);

  return jh;
}

void uhuru_json_handler_free(struct uhuru_json_handler *jh)
{
  json_tokener_free(jh->tokener);
  free(jh);
}

static int validate_request(struct json_object *json_req)
{
  return 0;
}

enum uhuru_json_status uhuru_json_handler_process_request(struct uhuru_json_handler *jh, const char *req, int req_len, struct uhuru *uhuru, char **p_resp, int *p_resp_len)
{
  struct json_object *json_req;

  json_tokener_reset(jh->tokener);

  json_req = json_tokener_parse_ex(jh->tokener, req, req_len);

  if (json_req == NULL) {
    enum json_tokener_error jerr = json_tokener_get_error(jh->tokener);

    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "error in JSON parsing: %s", json_tokener_error_desc(jerr));
    return JSON_PARSE_ERROR;
  }

  if (!validate_request(json_req))
    return JSON_INVALID_REQUEST;

  return 0;
}

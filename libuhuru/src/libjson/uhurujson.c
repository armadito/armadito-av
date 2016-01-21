#include "libuhuru-config.h"

#include <libuhuru/core.h>

#include "uhurujson.h"
#include "print.h"
#include "state.h"
#include "scan.h"

#include <assert.h>
#include <json.h>
#include <stdlib.h>
#include <string.h>

/* check if a struct json_tokener * can be used */
/* if called from multiple threads, no */

struct uhuru_json_handler {
  struct json_tokener *tokener;
};

struct uhuru_json_av_request {
  const char *request;
  int id;
  struct json_object *params;
};

struct uhuru_json_av_response {
  const char *response;
  int id;
  enum uhuru_json_status status;
  struct json_object *info;
  const char *error_message;
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

static enum uhuru_json_status parse_request(struct uhuru_json_handler *jh, const char *req, int req_len, struct json_object **p_json_req)
{
  json_tokener_reset(jh->tokener);

  *p_json_req = json_tokener_parse_ex(jh->tokener, req, req_len);

  if (*p_json_req == NULL) {
    enum json_tokener_error jerr = json_tokener_get_error(jh->tokener);

    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "error in JSON parsing: %s", json_tokener_error_desc(jerr));

    return JSON_PARSE_ERROR;
  }

  return JSON_OK;
}

static enum uhuru_json_status extract_request(struct json_object *j_request, struct uhuru_json_av_request *av_request)
{
  struct json_object *j_av_request, *j_id, *j_params;

  av_request->request = strdup("");
  av_request->id = 0;
  av_request->params = NULL;

  /* check if request is truly a JSON object, not a primitive value ? */
  if (!json_object_is_type(j_request, json_type_object))
    return JSON_INVALID_REQUEST;

  /* check if object contains key "av_request" with a string value */
  if (!json_object_object_get_ex(j_request, "av_request", &j_av_request)
      || !json_object_is_type(j_av_request, json_type_string))
    return JSON_INVALID_REQUEST;

  /* check if object contains key "id" with a int value */
  if (!json_object_object_get_ex(j_request, "id", &j_id)
      || !json_object_is_type(j_id, json_type_int))
    return JSON_INVALID_REQUEST;

  /* check if object contains key "params" with an object value */
  if (!json_object_object_get_ex(j_request, "params", &j_params)
      || !json_object_is_type(j_params, json_type_object))
    return JSON_INVALID_REQUEST;

  av_request->request = strdup(json_object_get_string(j_av_request));
  av_request->id = json_object_get_int(j_id);
  av_request->params = json_object_get(j_params);

  return JSON_OK;
}

static struct {
  const char *request;
  request_cb_t cb;
} request_dispatch[] = {
  { "state", state_request_cb},
  { "scan", scan_request_cb},
  { NULL, NULL},
};

enum uhuru_json_status call_request_handler(struct uhuru *uhuru, struct uhuru_json_av_request *av_request, struct uhuru_json_av_response *av_response)
{
  int i;

  av_response->response = strdup(av_request->request);
  av_response->id = av_request->id;
  av_response->info = NULL;
  av_response->status = JSON_INVALID_REQUEST;

  i = 0;
  while (request_dispatch[i].request != NULL && strcmp(request_dispatch[i].request, av_request->request))
    i++;

  if (request_dispatch[i].request != NULL) {
    request_cb_t cb = request_dispatch[i].cb;

    av_response->status = (*cb)(av_request->request, av_request->id, av_request->params, uhuru, &av_response->info, NULL);

    uhuru_json_print(av_response->info, stderr);
  }

  /* FIXME */
  av_response->error_message = NULL;

  return av_response->status;
}

static enum uhuru_json_status fill_response(struct uhuru_json_av_response *av_response, char **p_resp, int *p_resp_len)
{
  struct json_object *j_response;

  j_response = json_object_new_object();

  json_object_object_add(j_response, "av_response", json_object_new_string(av_response->response));
  json_object_object_add(j_response, "id", json_object_new_int(av_response->id));
  json_object_object_add(j_response, "status", json_object_new_int(av_response->status));
  if (av_response->info != NULL)
    json_object_object_add(j_response, "info", av_response->info);
  else
    json_object_object_add(j_response, "info", json_object_new_object());

  *p_resp = strdup(json_object_to_json_string(j_response));
  *p_resp_len = strlen(*p_resp);

  return JSON_OK;
}


enum uhuru_json_status uhuru_json_handler_process_request(struct uhuru_json_handler *jh, const char *req, int req_len, struct uhuru *uhuru, char **p_resp, int *p_resp_len)
{
  struct json_object *j_request, *j_response;
  struct uhuru_json_av_request av_request;
  struct uhuru_json_av_response av_response;
  enum uhuru_json_status status;

  status = parse_request(jh, req, req_len, &j_request);

  if (status) {
    return status;
  }

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "JSON parsed ok");
    
  uhuru_json_print(j_request, stderr);

  status = extract_request(j_request, &av_request);

  if (status) {
    /* fill_error_response_1(); */
    return status;
  }

  status = call_request_handler(uhuru, &av_request, &av_response);

  fill_response(&av_response, p_resp, p_resp_len);

  return status;
}

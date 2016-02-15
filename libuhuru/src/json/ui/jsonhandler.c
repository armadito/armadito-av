#include "libuhuru-config.h"
#include <libuhuru/core.h>

#include "jsonhandler.h"
#include "jsonhandler_p.h"
#include "debug.h"
#include "state.h"
#include "scan.h"
#include "os/string.h"

#include <assert.h>
#include <json.h>
#include <stdlib.h>
#include <string.h>

static struct {
  const char *request;
  request_cb_t cb;
} request_dispatch[] = {
  { "state", state_request_cb},
  { "scan", scan_request_cb},
  { NULL, NULL},
};

struct uhuru_json_handler {
  struct json_tokener *tokener;
  struct uhuru *uhuru;
};

struct uhuru_json_handler *uhuru_json_handler_new(struct uhuru *uhuru)
{
  struct uhuru_json_handler *jh = malloc(sizeof(struct uhuru_json_handler));

  jh->tokener = json_tokener_new();
  assert(jh->tokener != NULL);

  jh->uhuru = uhuru;

  return jh;
}

void uhuru_json_handler_free(struct uhuru_json_handler *jh)
{
  json_tokener_free(jh->tokener);
  free(jh);
}

static const char *status_2_error(enum uhuru_json_status status)
{
  switch(status) {
  case JSON_OK: return "ok";
  case JSON_PARSE_ERROR: return "invalid JSON string";
  case JSON_MALFORMED_REQUEST: return "malformed request in JSON object";
  case JSON_INVALID_REQUEST: return "invalid request";
  case JSON_REQUEST_FAILED: return "execution of request failed";
  }

  return "??? invalid status";
}

static enum uhuru_json_status parse_request(struct uhuru_json_handler *jh, const char *req, int req_len, struct json_object **p_j_request)
{
  json_tokener_reset(jh->tokener);

  *p_j_request = json_tokener_parse_ex(jh->tokener, req, req_len);

  if (*p_j_request == NULL) {
    enum json_tokener_error jerr = json_tokener_get_error(jh->tokener);

    uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_WARNING, "error in JSON parsing: %s", json_tokener_error_desc(jerr));

    return JSON_PARSE_ERROR;
  }

  return JSON_OK;
}

static enum uhuru_json_status extract_request(struct json_object *j_request, struct json_request *av_request)
{
  struct json_object *j_av_request, *j_id, *j_params;

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

  av_request->request = os_strdup(json_object_get_string(j_av_request));
  av_request->id = json_object_get_int(j_id);
  av_request->params = json_object_get(j_params);

  /* note that this will automatically free the objects contained in j_request, i.e. j_id and j_av_request */
  /* but NOT j_params which has been extracted with json_object_get() */
  assert(json_object_put(j_request));

  return JSON_OK;
}

enum uhuru_json_status call_request_handler(struct uhuru *uhuru, struct json_request *av_request, struct json_response *av_response)
{
  int i;
  enum uhuru_json_status status = JSON_INVALID_REQUEST;  

  i = 0;
  while (request_dispatch[i].request != NULL && strcmp(request_dispatch[i].request, av_request->request))
    i++;

  if (request_dispatch[i].request != NULL) {
    request_cb_t cb = request_dispatch[i].cb;

    status = (*cb)(uhuru, av_request, av_response);

    if (av_response->info != NULL)
      jobj_debug(av_response->info, "info");
  }

  return status;
}

static enum uhuru_json_status fill_response(struct json_response *av_response, char **p_resp, int *p_resp_len)
{
  struct json_object *j_response = json_object_new_object();

  if (av_response->response != NULL)
    json_object_object_add(j_response, "av_response", json_object_new_string(av_response->response));

  json_object_object_add(j_response, "id", json_object_new_int(av_response->id));

  json_object_object_add(j_response, "status", json_object_new_int(av_response->status));

  if (av_response->status == JSON_OK) {
    if (av_response->info != NULL)
      json_object_object_add(j_response, "info", json_object_get(av_response->info));
  } else {
    if (av_response->error_message == NULL)
      json_object_object_add(j_response, "error-message", json_object_new_string(status_2_error(av_response->status)));
    else
      json_object_object_add(j_response, "error-message", json_object_new_string(av_response->error_message));
  }

  jobj_debug(j_response, "AV response");

  *p_resp = (char *)os_strdup(json_object_to_json_string(j_response));
  *p_resp_len = strlen(*p_resp);

  /* note that this will automatically free the objects contained in j_response */
  /* but NOT its "info" member which has been referenced with json_object_get() */
  assert(json_object_put(j_response));

  return JSON_OK;
}

enum uhuru_json_status uhuru_json_handler_av_request(struct uhuru_json_handler *jh, const char *req, int req_len, char **p_resp, int *p_resp_len)
{
  struct json_object *j_request, *j_response;
  struct json_request av_request;
  struct json_response av_response;

  av_response.response = NULL;
  av_response.id = -1;
  av_response.info = NULL;
  av_response.error_message = NULL;

  av_response.status = parse_request(jh, req, req_len, &j_request);

  if (av_response.status)
    goto get_out;

  jobj_debug(j_request, "AV request");

  av_response.status = extract_request(j_request, &av_request);

  if (av_response.status)
    goto get_out;

  av_response.response = os_strdup(av_request.request);
  av_response.id = av_request.id;
  av_response.status = call_request_handler(jh->uhuru, &av_request, &av_response);

 get_out:
  fill_response(&av_response, p_resp, p_resp_len);

  json_request_destroy(&av_request);
  json_response_destroy(&av_response);

  return av_response.status;
}

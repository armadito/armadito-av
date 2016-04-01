#ifndef _JSONCLIENT_H_
#define _JSONCLIENT_H_

#include <libarmadito.h>

struct json_client;

struct json_client *json_client_new(int sock, struct armadito *armadito);

void json_client_free(struct json_client *cl);

int json_client_process(struct json_client *cl);

#endif

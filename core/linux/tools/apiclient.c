#define _GNU_SOURCE

#include <curl/curl.h>
#include <json.h>
#include <string.h>

struct api_client {
	unsigned short port;
	const char *token;
};

#define API_HOST "127.0.0.1"
#define API_TOKEN_HEADER "X-Armadito-Token"

struct api_client *api_client_new(unsigned short port)
{
	struct api_client *c = malloc(sizeof(struct api_client));

	c->port = port;
	c->token = NULL;

	return c;
}

static void add_json_content(CURL *hnd, struct json_object *in)
{
	const char *json_buff;

	json_buff = json_object_to_json_string(in);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, json_buff);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)strlen(json_buff));
}

int api_client_call(struct api_client *client, const char *path, struct json_object *in, struct json_object **out)
{
	char *url, *token_header;
	CURL *hnd;
	struct curl_slist *slist = NULL;
	CURLcode ret;

	hnd = curl_easy_init();

	asprintf(&url, "http://" API_HOST ":%d/api%s", client->port, path);
	curl_easy_setopt(hnd, CURLOPT_URL, url);
	free(url);

	asprintf(&token_header, API_TOKEN_HEADER ": %s", client->token);
	slist = curl_slist_append(slist, token_header);
	free(token_header);

	if (in != NULL) {
		add_json_content(hnd, in);
		slist = curl_slist_append(slist, "Content-Type: application/json");
		curl_easy_setopt(hnd, CURLOPT_POST, 1L);
	} else
		curl_easy_setopt(hnd, CURLOPT_HTTPGET, 1L);

	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.47.0");
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

	if (slist != NULL)
		curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist);

	ret = curl_easy_perform(hnd);

	curl_easy_cleanup(hnd);
	if (slist != NULL)
		curl_slist_free_all(slist);

	return (int)ret;
}

int api_client_register(struct api_client *client)
{
	int ret;
	struct json_object *out, *j_token;

	ret = api_client_call(client, "/register", NULL, &out);
	if (ret != 0)
		return ret;

	if (!json_object_object_get_ex(out, "token", &j_token)
		|| !json_object_is_type(j_token, json_type_string))
		return 1;

	client->token = strdup(json_object_get_string(j_token));

	return 0;
}

int api_client_unregister(struct api_client *client)
{

}

void api_client_free(struct api_client *client)
{
	free((void *)client->token);
	free(client);
}

/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#define _GNU_SOURCE

#include <curl/curl.h>
#include <json.h>
#include <glib.h>
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

struct post_processor {
	GByteArray *buffer;
};

static struct post_processor *post_processor_new(void)
{
	struct post_processor *p = malloc(sizeof(struct post_processor));

	p->buffer = g_byte_array_new();

	return p;
}

static const char *post_processor_append(struct post_processor *p,
	const char *data, size_t data_size)
{
	g_byte_array_append(p->buffer, data, data_size);
}

#define post_processor_get_data(p) ((p)->buffer->data)

#define post_processor_get_size(p) ((p)->buffer->len)

static void post_processor_free(struct post_processor *p)
{
	g_byte_array_free(p->buffer, TRUE);

	free((void *)p);
}

static size_t post_write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct post_processor *processor = (struct post_processor *)userp;
	char c = '\0';

	post_processor_append(processor, contents, realsize);
	post_processor_append(processor, &c, 1);

	return realsize;
}

static void add_json_content(CURL *hnd, struct json_object *in)
{
	const char *json_buff;

	json_buff = json_object_to_json_string(in);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, json_buff);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)strlen(json_buff));
}

static struct json_object *parse_json_response(const char *post_data, size_t post_data_size)
{
	struct json_object *j_response;
	struct json_tokener *tokener;

	if (post_data_size == 0)
		return NULL;

	tokener = json_tokener_new();
	json_tokener_reset(tokener);

	j_response = json_tokener_parse_ex(tokener, post_data, post_data_size);

	if (j_response == NULL) {
		enum json_tokener_error jerr = json_tokener_get_error(tokener);

		if (jerr != json_tokener_success)
			fprintf(stderr, "error in JSON parsing: %s", json_tokener_error_desc(jerr));
	}

	json_tokener_free(tokener);

	return j_response;
}

int api_client_call(struct api_client *client, const char *path, struct json_object *in, struct json_object **out)
{
	char *url, *token_header;
	CURL *hnd;
	struct curl_slist *slist = NULL;
	struct post_processor *pp;
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

	pp = post_processor_new();

	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, post_write_callback);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, pp);

	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.47.0");
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
	/* curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L); */
	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

	if (slist != NULL)
		curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist);

	ret = curl_easy_perform(hnd);

	curl_easy_cleanup(hnd);
	if (slist != NULL)
		curl_slist_free_all(slist);

	/* was it a POST request? */
	if (out != NULL)
		*out = parse_json_response(post_processor_get_data(pp), post_processor_get_size(pp));

	post_processor_free(pp);

	return (int)ret;
}

int api_client_register(struct api_client *client)
{
	int ret;
	struct json_object *out = NULL, *j_token;

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

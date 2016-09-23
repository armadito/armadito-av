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
#include <libarmadito.h>

#include "os/dir.h"
#include "modulep.h"
#include "alert.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlsave.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <time.h>
#include <glib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

struct alert {
	xmlDocPtr xml_doc;
};

static xmlNodePtr alert_doc_detection_time_node(void)
{
	xmlNodePtr node;
	time_t t;
	struct tm l_tm;
	char buff[32];

	node = xmlNewNode(NULL, "detection_time");

	time(&t);
	localtime_r(&t, &l_tm);

	/* format: "2001-12-31T12:00:00" */
	/* FIXME: use g_string_printf */
	snprintf(buff, sizeof(buff) - 1, "%04d-%02d-%02dT%02d:%02d:%02d", 1900 + l_tm.tm_year, 1 + l_tm.tm_mon, l_tm.tm_mday, l_tm.tm_hour, l_tm.tm_min, l_tm.tm_sec);
	buff[sizeof(buff) - 1] = '\0';
	xmlAddChild(node, xmlNewText(buff));

	return node;
}

static void get_ip_addr(char *ip_addr)
{
	int s;
	char mask[NI_MAXHOST];
	struct ifaddrs *ifaddr, *ifa;

	/* Set string to empty one */
	ip_addr[0] = '\0';

	/* Don't exit and just set ip is not available while failing */
	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		goto on_failure;
	}

	/* Search for the first interface with a mask different than 255.0.0.0
	   and extract its associated ip, stop the search on failure */
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;

		if (ifa->ifa_addr->sa_family != AF_INET && ifa->ifa_addr->sa_family != AF_INET6)
			continue;

		s = getnameinfo(ifa->ifa_netmask,
			(ifa->ifa_addr->sa_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
			mask, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		if (s != 0) {
			a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_ERROR, "getnameinfo() failed: %s", gai_strerror(s));
			break;
		}

		/* Skip the interface if it has a too large ip mask */
		if (strcmp(mask, "255.0.0.0") == 0)
			continue;

		s = getnameinfo(ifa->ifa_addr,
			(ifa->ifa_addr->sa_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
			ip_addr, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		if (s != 0) {
			a6o_log(ARMADITO_LOG_LIB, ARMADITO_LOG_LEVEL_ERROR, "getnameinfo() failed: %s\n", gai_strerror(s));
			break;
		}

		break;
	}

on_failure:
	/* Set ip_addr not available if not found */
	if (ip_addr[0] == '\0')
		snprintf(ip_addr, NI_MAXHOST - 1, "n/a");
	if (ifaddr != NULL)
		freeifaddrs(ifaddr);
	return;
}

#define PID_MAX 128
#define PROCESS_NAME_MAX 1024

static xmlNodePtr alert_doc_identification_node(void)
{
	xmlNodePtr node;
	char hostname[HOST_NAME_MAX + 1];
	char ip_addr[NI_MAXHOST];
	char pid[PID_MAX];
	char process_name[PROCESS_NAME_MAX];

	node = xmlNewNode(NULL, "identification");

	xmlNewChild(node, NULL, "user", getenv("USER"));
	gethostname(hostname, HOST_NAME_MAX + 1);
	xmlNewChild(node, NULL, "hostname", hostname);
	get_ip_addr(ip_addr);
	xmlNewChild(node, NULL, "ip", ip_addr);
	xmlNewChild(node, NULL, "os", "Linux");

	snprintf(pid, PID_MAX, "%d", getpid());
	xmlNewChild(node, NULL, "pid", pid);

	/* extern char *program_invocation_name; */
	/* extern char *program_invocation_short_name; */

	xmlNewChild(node, NULL, "process", program_invocation_short_name);

	return node;
}

static xmlDocPtr alert_doc_new(void)
{
	xmlDocPtr doc;
	xmlNodePtr root_node;

	LIBXML_TEST_VERSION;

	doc = xmlNewDoc("1.0");
	root_node = xmlNewNode(NULL, "alert");
	xmlDocSetRootElement(doc, root_node);

	return doc;
}

static void alert_doc_add_alert(xmlDocPtr doc, struct a6o_report *report)
{
	xmlNodePtr root_node, node;

	root_node = xmlDocGetRootElement(doc);
	xmlNewChild(root_node, NULL, "level", "2");

	node = xmlNewChild(root_node, NULL, "uri", report->path);
	xmlNewProp(node, "type", "path");

	xmlAddChild(root_node, alert_doc_detection_time_node());
	xmlAddChild(root_node, alert_doc_identification_node());
	xmlNewChild(root_node, NULL, "module", report->mod_name);
	xmlNewChild(root_node, NULL, "module_specific", report->mod_report);
}

static void alert_doc_save(xmlDocPtr doc, const char *filename)
{
	xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);
}

static void alert_doc_save_to_fd(xmlDocPtr doc, int fd)
{
	xmlSaveCtxtPtr xmlCtxt = xmlSaveToFd(fd, "UTF-8", XML_SAVE_FORMAT);

	if (xmlCtxt != NULL) {
		xmlSaveDoc(xmlCtxt, doc);
		xmlSaveClose(xmlCtxt);
	}
}

static void alert_doc_save_to_buffer(xmlDocPtr doc, xmlBufferPtr *pxml_buf)
{
	xmlSaveCtxtPtr xmlCtxt;

	*pxml_buf = xmlBufferCreate();
	xmlCtxt = xmlSaveToBuffer(*pxml_buf, "UTF-8", XML_SAVE_FORMAT);

	if (xmlCtxt != NULL) {
		xmlSaveDoc(xmlCtxt, doc);
		xmlSaveClose(xmlCtxt);
	}
}

static void alert_doc_free(xmlDocPtr doc)
{
	xmlFreeDoc(doc);
}
 
static struct alert *alert_new()
{
	struct alert *a;

	a = (struct alert *)malloc(sizeof(struct alert));
	assert(a != NULL);

	a->xml_doc = alert_doc_new();

	return a;
}

static void alert_add(struct alert *a, struct a6o_report *report)
{
	alert_doc_add_alert(a->xml_doc, report);
}

static void alert_send_via_file(struct alert *a, const char *alert_dir)
{
	int fd;
	char *alert_path;

	asprintf(&alert_path, "%s/alertXXXXXX", alert_dir);
	assert(alert_path != NULL);

	/* FIXME: signal error if cannot open file */
	fd = mkostemp(alert_path, O_WRONLY | O_CREAT | O_TRUNC);
	fchmod(fd, 0666);

	if (fd != -1) {
		alert_doc_save_to_fd(a->xml_doc, fd);
		close(fd);
	}

	free(alert_path);
}

static void alert_free(struct alert *a)
{
	alert_doc_free(a->xml_doc);

	free(a);
}

/*
 * module specific functions
 */

struct alert_data {
	char *alert_dir;
};

static enum a6o_mod_status alert_init(struct a6o_module *module)
{
	struct alert_data *al_data = g_new(struct alert_data, 1);

	al_data->alert_dir = NULL;

	module->data = al_data;

	return ARMADITO_MOD_OK;
}

static enum a6o_mod_status alert_conf_alert_dir(struct a6o_module *module, const char *key, struct a6o_conf_value *value)
{
	struct alert_data *al_data = (struct alert_data *)module->data;

	if (!a6o_conf_value_is_string(value))
		return ARMADITO_MOD_CONF_ERROR;

	al_data->alert_dir = strdup(a6o_conf_value_get_string(value));

	/* really necessary??? */
	os_mkdir_p(al_data->alert_dir);

	return ARMADITO_MOD_OK;
}

void alert_callback(struct a6o_report *report, void *callback_data)
{
	struct alert_data *al_data = (struct alert_data *)callback_data;
	struct alert *a;

	switch(report->status) {
	case ARMADITO_CLEAN:
	case ARMADITO_UNKNOWN_FILE_TYPE:
	case ARMADITO_EINVAL:
	case ARMADITO_IERROR:
	case ARMADITO_UNDECIDED:
	case ARMADITO_WHITE_LISTED:
		return;
	}

	a = alert_new();
	alert_add(a, report);
	alert_send_via_file(a, al_data->alert_dir);
	alert_free(a);

	report->action |= ARMADITO_ACTION_ALERT;
}

static struct a6o_conf_entry alert_conf_table[] = {
	{
		.key = "alert-dir",
		.type = CONF_TYPE_STRING,
		.conf_fun = alert_conf_alert_dir,
	},
	{
		.key = NULL,
		.type = 0,
		.conf_fun = NULL,
	},
};

struct a6o_module alert_module = {
	.init_fun = alert_init,
	.conf_table = alert_conf_table,
	.post_init_fun = NULL,
	.scan_fun = NULL,
	.close_fun = NULL,
	.supported_mime_types = NULL,
	.name = "alert",
	.size = sizeof(struct alert_data),
};

#include <libuhuru/core.h>

#include "os/dir.h"
#include "modulep.h"
#include "alert.h"

#undef ALERT_VIA_SSL
#define ALERT_VIA_FILE

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlsave.h>
#ifdef ALERT_VIA_SSL
#include <curl/curl.h>
#endif
#define _GNU_SOURCE
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

struct alert {
  xmlDocPtr xml_doc;
};

static xmlNodePtr alert_doc_gdh_node(void)
{
  xmlNodePtr node;
  time_t t;
  struct tm l_tm;
  char buff[32];

  node = xmlNewNode(NULL, "gdh");

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
  struct ifaddrs *ifaddr, *ifa;

  /* FIXME: do not exit on failure, but report error */
  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    exit(EXIT_FAILURE);
  }

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;

    if (ifa->ifa_addr->sa_family == AF_INET || ifa->ifa_addr->sa_family == AF_INET6) {
      int s;
      char mask[NI_MAXHOST];

      /* FIXME: do not exit on failure, but report error */
      s = getnameinfo(ifa->ifa_netmask,
		      (ifa->ifa_addr->sa_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
		      mask, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      if (s != 0) {
	uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_ERROR, "getnameinfo() failed: %s", gai_strerror(s));
	exit(EXIT_FAILURE);
      }

      if (strcmp(mask, "255.0.0.0") == 0)
	continue;

      /* FIXME: do not exit on failure, but report error */
      s = getnameinfo(ifa->ifa_addr,
		      (ifa->ifa_addr->sa_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
		      ip_addr, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      if (s != 0) {
	uhuru_log(UHURU_LOG_LIB, UHURU_LOG_LEVEL_ERROR, "getnameinfo() failed: %s\n", gai_strerror(s));
	exit(EXIT_FAILURE);
      }

      return;
    }
  }

  freeifaddrs(ifaddr);
}

static xmlNodePtr alert_doc_identification_node(void)
{
  xmlNodePtr node;
  char hostname[HOST_NAME_MAX + 1];
  char ip_addr[NI_MAXHOST];

  node = xmlNewNode(NULL, "identification");

  xmlNewChild(node, NULL, "user", getenv("USER"));
  gethostname(hostname, HOST_NAME_MAX + 1);
  xmlNewChild(node, NULL, "hostname", hostname);
  get_ip_addr(ip_addr);
  xmlNewChild(node, NULL, "ip", ip_addr);
  xmlNewChild(node, NULL, "os", "Linux");

  return node;
}

static xmlDocPtr alert_doc_new(void)
{
  xmlDocPtr doc;
  xmlNodePtr root_node;

  LIBXML_TEST_VERSION;

  doc = xmlNewDoc("1.0");
  root_node = xmlNewNode(NULL, "alert");
  xmlNewProp(root_node, "xmlns", "http://www.davfi-project.org/AlertSchema");
  xmlNewProp(root_node, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
  xmlNewProp(root_node, "xsi:schemaLocation", "http://www.davfi-project.org/AlertSchema AlertSchema.xsd ");
  xmlDocSetRootElement(doc, root_node);

  return doc;
}

static void alert_doc_add_alert(xmlDocPtr doc, struct uhuru_report *report)
{
  xmlNodePtr root_node, node;

  root_node = xmlDocGetRootElement(doc);
  xmlNewChild(root_node, NULL, "code", "a");
  xmlNewChild(root_node, NULL, "level", "2");

  node = xmlNewChild(root_node, NULL, "uri", report->path);
  xmlNewProp(node, "type", "path");

  xmlAddChild(root_node, alert_doc_gdh_node());

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

static void alert_add(struct alert *a, struct uhuru_report *report)
{
  alert_doc_add_alert(a->xml_doc, report);
}

#ifdef ALERT_VIA_UNIX_SOCKET
static int connect_socket(const char *path)
{
  int fd;
  struct sockaddr_un server_addr;

  fd = socket( AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("socket() failed");
    return -1;
  }

  server_addr.sun_family = AF_UNIX;
  strcpy(server_addr.sun_path, path);

  if (connect(fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) < 0) {
    close(fd);
    perror("connecting stream socket");
    return -1;
  }

  return fd;
}

#define ALERT_SOCKET_PATH "/var/tmp/davfi_alert.s"

static void alert_send_via_unix_sock(struct alert *a)
{
  int fd;
				
  if (a->xml_doc == NULL)
    return;

#if 0
  alert_doc_save(a->xml_doc, "-");
#endif

  fd = connect_socket(ALERT_SOCKET_PATH);
  if (fd != -1) {
    alert_doc_save_to_fd(a->xml_doc, fd);
    close(fd);
  }
}
#endif

#ifdef ALERT_VIA_SSL
#ifdef ALERT_VIA_SSL
#define UHURU_ALERT_URL "https://127.0.0.1:10083/"
/* #define UHURU_ALERT_URL "http://127.0.0.1:10083/" */
#endif

static size_t discard_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
   return size * nmemb;
}

static void alert_send_via_https(struct alert *a)
{
  static int error_count;
  xmlBufferPtr xml_buf;
  struct curl_httppost *formpost = NULL;
  struct curl_httppost *lastptr = NULL;
  CURL *curl;
  CURLcode res;

  if (a->xml_doc == NULL)
    return;

  curl_global_init(CURL_GLOBAL_ALL);

  curl = curl_easy_init();
  if (!curl)
    return;

  curl_easy_setopt(curl, CURLOPT_URL, UHURU_ALERT_URL);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libuhuru/1.0");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_data);
#ifdef ALERT_REPORT_SSL
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

  alert_doc_save_to_buffer(a->xml_doc, &xml_buf);
  curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, "xml", CURLFORM_PTRCONTENTS, xmlBufferContent(xml_buf), CURLFORM_END);
  curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

  res = curl_easy_perform(curl);

#define MAX_CURL_ERROR_PRINT 3
  if(res != CURLE_OK && error_count++ < MAX_CURL_ERROR_PRINT) {
    /* this mode of reporting alert is no longer available, so give up fixing error reporting */
  }

  curl_easy_cleanup(curl);
  curl_formfree(formpost);

  xmlBufferFree(xml_buf);
}
#endif


#ifdef ALERT_VIA_FILE

static void alert_send_via_file(struct alert *a, const char *alert_dir)
{
  int fd;
  char *alert_path;

  asprintf(&alert_path, "%s/alertXXXXXX", alert_dir) ;
  assert(alert_path != NULL);

  /* FIXME: signal error if cannot open file */
  fd = mkostemp(alert_path, O_WRONLY | O_CREAT | O_TRUNC);
  fchmod(fd, 0666);

  if (fd != -1) {
    alert_doc_save_to_fd(a->xml_doc, fd);
    close(fd);
  }
}

#endif

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

static enum uhuru_mod_status alert_init(struct uhuru_module *module)
{
  struct alert_data *al_data = g_new(struct alert_data, 1);

  al_data->alert_dir = NULL;

  module->data = al_data;

  return UHURU_MOD_OK;
}

static enum uhuru_mod_status alert_conf_alert_dir(struct uhuru_module *module, const char *directive, const char **argv)
{
  struct alert_data *al_data = (struct alert_data *)module->data;

  al_data->alert_dir = strdup(argv[0]);

  /* really necessary??? */
  os_mkdir_p(al_data->alert_dir);

  return UHURU_MOD_OK;
}

void alert_callback(struct uhuru_report *report, void *callback_data)
{
  struct alert_data *al_data = (struct alert_data *)callback_data;
  struct alert *a;

  switch(report->status) {
  case UHURU_CLEAN:
  case UHURU_UNKNOWN_FILE_TYPE:
  case UHURU_EINVAL:
  case UHURU_IERROR:
  case UHURU_UNDECIDED:
  case UHURU_WHITE_LISTED:
    return;
  }

  a = alert_new();
  alert_add(a, report);

#ifdef ALERT_VIA_SSL
  alert_send_via_https(a);
#endif
#ifdef ALERT_VIA_FILE
  alert_send_via_file(a, al_data->alert_dir);
#endif

  alert_free(a);

  report->action |= UHURU_ACTION_ALERT;
}

static struct uhuru_conf_entry alert_conf_table[] = {
  { 
    .directive = "alert-dir", 
    .conf_fun = alert_conf_alert_dir, 
  },
  { 
    .directive = NULL,
    .conf_fun = NULL, 
  },
};

struct uhuru_module alert_module = {
  .init_fun = alert_init,
  .conf_table = alert_conf_table,
  .post_init_fun = NULL,
  .scan_fun = NULL,
  .close_fun = NULL,
  .name = "alert",
  .size = sizeof(struct alert_data),
};

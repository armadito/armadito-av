#include <libumwsu/scan.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlsave.h>
#include <stdio.h>
#include <stdlib.h>
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

struct alert_data {
  xmlDocPtr xml_doc;
  GMutex doc_lock;
};

static void get_ip_addr(char *ip_addr)
{
  struct ifaddrs *ifaddr, *ifa;

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

      s = getnameinfo(ifa->ifa_netmask,
		      (ifa->ifa_addr->sa_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
		      mask, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      if (s != 0) {
	printf("getnameinfo() failed: %s\n", gai_strerror(s));
	exit(EXIT_FAILURE);
      }

      if (strcmp(mask, "255.0.0.0") == 0)
	continue;

      s = getnameinfo(ifa->ifa_addr,
		      (ifa->ifa_addr->sa_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
		      ip_addr, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      if (s != 0) {
	printf("getnameinfo() failed: %s\n", gai_strerror(s));
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
  snprintf(buff, sizeof(buff) - 1, "%04d-%02d-%02dT%02d:%02d:%02d", 1900 + l_tm.tm_year, l_tm.tm_mon, l_tm.tm_mday, l_tm.tm_hour, l_tm.tm_min, l_tm.tm_sec);
  buff[sizeof(buff) - 1] = '\0';
  xmlAddChild(node, xmlNewText(buff));

  return node;
}

static xmlDocPtr alert_doc_new(void)
{
  xmlDocPtr doc;
  xmlNodePtr root_node;

  LIBXML_TEST_VERSION;

  doc = xmlNewDoc("1.0");
  root_node = xmlNewNode(NULL, "alert_set");
  xmlNewProp(root_node, "xmlns", "http://www.davfi-project.org/AlertSchema");
  xmlNewProp(root_node, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
  xmlNewProp(root_node, "xsi:schemaLocation", "http://www.davfi-project.org/AlertSchema AlertSchema.xsd ");
  xmlDocSetRootElement(doc, root_node);

  xmlAddChild(root_node, alert_doc_identification_node());
  xmlAddChild(root_node, alert_doc_gdh_node());

  return doc;
}

static void alert_doc_add_alert(xmlDocPtr doc, struct umwsu_report *report)
{
  xmlNodePtr alert_node, node;

  alert_node = xmlNewChild(xmlDocGetRootElement(doc), NULL, "alert", NULL);
  xmlNewChild(alert_node, NULL, "code", "a");
  xmlNewChild(alert_node, NULL, "level", "2");
  node = xmlNewChild(alert_node, NULL, "uri", report->path);
  xmlNewProp(node, "type", "path");
  xmlNewChild(alert_node, NULL, "module", report->mod_name);
  xmlNewChild(alert_node, NULL, "module_specific", report->mod_report);
}

static void alert_doc_save(xmlDocPtr doc, const char *filename)
{
  xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);
}

static void alert_doc_free(xmlDocPtr doc)
{
  xmlFreeDoc(doc);
}
 
static struct alert_data *alert_data_new(void)
{
  struct alert_data *ad;

  ad = (struct alert_data *)malloc(sizeof(struct alert_data));
  assert(ad != NULL);

  ad->xml_doc = NULL;

  g_mutex_init(&ad->doc_lock);
}

static void alert_data_add(struct alert_data *ad, struct umwsu_report *report)
{
  if (report->status == UMWSU_CLEAN || report->status == UMWSU_UNKNOWN_FILE_TYPE)
    return;

  g_mutex_lock(&ad->doc_lock);

  if (ad->xml_doc == NULL)
    ad->xml_doc = alert_doc_new();

  alert_doc_add_alert(ad->xml_doc, report);

  g_mutex_unlock(&ad->doc_lock);
}

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

static void alert_data_send_and_free(struct alert_data *ad)
{
  int fd;
				
  fd = connect_socket(ALERT_SOCKET_PATH);
  if (fd != -1) {
    xmlSaveCtxtPtr xmlCtxt = xmlSaveToFd(fd, "UTF-8", XML_SAVE_FORMAT);

    if (xmlCtxt != NULL) {
      xmlSaveDoc(xmlCtxt, ad->xml_doc);
      xmlSaveClose(xmlCtxt);
    }

    close(fd);
  }

  xmlFreeDoc(ad->xml_doc);

  g_mutex_clear(&ad->doc_lock);

  free(ad);
}

void umwsu_alert_callback(struct umwsu *u, /* enum umwsu_event_type event_type, */ struct umwsu_report *report, void **callback_data)
{
  struct alert_data **ad = (struct alert_data **)callback_data;

#if 0
  switch(event_type) {
  case UMWSU_SCAN_START:
    *ad = alert_data_new();
    break;
  case UMWSU_SCAN_START:
    alert_data_add(*ad, report);
    break;
  case UMWSU_SCAN_END:
    alert_data_send_and_free(*ad);
    *ad = NULL;
    break;
  }
#endif

}


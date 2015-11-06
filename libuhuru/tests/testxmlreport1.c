#include <libuhuru/core.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <time.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif

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

      /* printf("\tmask: <%s>\n", mask); */

      s = getnameinfo(ifa->ifa_addr,
		      (ifa->ifa_addr->sa_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
		      ip_addr, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      if (s != 0) {
	printf("getnameinfo() failed: %s\n", gai_strerror(s));
	exit(EXIT_FAILURE);
      }

      /* printf("\taddress: <%s>\n", ip_addr); */

      return;
    }
  }

  freeifaddrs(ifaddr);
}

static xmlNodePtr document_identification_node(void)
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

static xmlNodePtr document_gdh_node(void)
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

xmlDocPtr document_new(void)
{
  xmlDocPtr doc;
  xmlNodePtr root_node;

  doc = xmlNewDoc("1.0");
  root_node = xmlNewNode(NULL, "alert_set");
  xmlNewProp(root_node, "xmlns", "http://www.davfi-project.org/AlertSchema");
  xmlNewProp(root_node, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
  xmlNewProp(root_node, "xsi:schemaLocation", "http://www.davfi-project.org/AlertSchema AlertSchema.xsd ");
  xmlDocSetRootElement(doc, root_node);

  xmlAddChild(root_node, document_identification_node());
  xmlAddChild(root_node, document_gdh_node());

  return doc;
}

void document_add_alert(xmlDocPtr doc, struct uhuru_report *report)
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

void document_save(xmlDocPtr doc, const char *filename)
{
  xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);
}

void document_free(xmlDocPtr doc)
{
  xmlFreeDoc(doc);
}
 
static void report_fill(struct uhuru_report *report, enum uhuru_file_status status, const char *path, const char *mod_name, const char *mod_report)
{
  report->status = status;
  report->path = (char *)path;
  report->mod_name = (char *)mod_name;
  report->mod_report = (char *)mod_report;
}

int main(int argc, char **argv)
{
  xmlDocPtr doc;
  struct uhuru_report report;

  LIBXML_TEST_VERSION;

  doc = document_new();

  report_fill(&report, UHURU_MALWARE, "/home/francois/zob", "module_pi", "virus de la grippe");
  document_add_alert(doc, &report);
  report_fill(&report, UHURU_SUSPICIOUS, "/home/francois/zobi", "module_e", "virus de la chtouille");
  document_add_alert(doc, &report);

  document_save(doc, argc > 1 ? argv[1] : "-");

  document_free(doc);

  xmlCleanupParser();

  return 0;
}

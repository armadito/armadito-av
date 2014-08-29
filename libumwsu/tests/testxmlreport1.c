#include <libumwsu/status.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include <stdlib.h>

static xmlDocPtr document_new(void)
{
  xmlDocPtr doc;
  xmlNodePtr root_node;

  doc = xmlNewDoc("1.0");
  root_node = xmlNewNode(NULL, "alert_set");
  xmlNewProp(root_node, "xmlns", "http://www.davfi-project.org/AlertSchema");
  xmlNewProp(root_node, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
  xmlNewProp(root_node, "xsi:schemaLocation", "http://www.davfi-project.org/AlertSchema AlertSchema.xsd ");
  xmlDocSetRootElement(doc, root_node);

  return doc;
}

static xmlNodePtr document_identification_node(void)
{
  xmlNodePtr node;

  node = xmlNewNode(NULL, "identification");

  xmlNewChild(node, NULL, "user", getenv("USER"));
  xmlNewChild(node, NULL, "os", "Linux");

  return node;
}

static void document_add_alert(xmlDocPtr doc, struct umwsu_report *report)
{
  xmlNodePtr alert_node, node;

  alert_node = xmlNewChild(xmlDocGetRootElement(doc), NULL, "alert", NULL);
  xmlNewChild(alert_node, NULL, "code", "a");
  xmlNewChild(alert_node, NULL, "level", "2");
  node = xmlNewChild(alert_node, NULL, "uri", report->path);
  xmlNewProp(node, "type", "path");
  xmlNewChild(alert_node, NULL, "gdh", "2001-12-31T12:00:00");
  xmlAddChild(alert_node, document_identification_node());
  xmlNewChild(alert_node, NULL, "module", report->mod_name);
  xmlNewChild(alert_node, NULL, "module_specific", report->mod_report);
}

static void document_save(xmlDocPtr doc, const char *filename)
{
  xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);
}

static void document_free(xmlDocPtr doc)
{
  xmlFreeDoc(doc);
}
 
static void report_fill(struct umwsu_report *report, enum umwsu_status status, const char *path, const char *mod_name, const char *mod_report)
{
  report->status = status;
  report->path = (char *)path;
  report->mod_name = (char *)mod_name;
  report->mod_report = (char *)mod_report;
}

int main(int argc, char **argv)
{
  xmlDocPtr doc;
  struct umwsu_report report;

  LIBXML_TEST_VERSION;

  doc = document_new();

  report_fill(&report, UMWSU_MALWARE, "/home/francois/zob", "module_pi", "virus de la grippe");

  document_add_alert(doc, &report);

  document_save(doc, argc > 1 ? argv[1] : "-");

  document_free(doc);

  xmlCleanupParser();

  return 0;
}

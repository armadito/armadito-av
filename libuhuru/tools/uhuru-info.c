#include <libuhuru/info.h>
#include "lib/infop.h"
#include "lib/uhurup.h"

#include <getopt.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlsave.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct info_options {
  int output_xml;
  int use_daemon;
};

static struct option long_options[] = {
  {"help",         no_argument,       0, 'h'},
  {"local",        no_argument,       0, 'l'},
  {"xml",          no_argument,       0, 'x'},
  {0, 0, 0, 0}
};

static void usage(void)
{
  fprintf(stderr, "usage: uhuru-info [options]\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Uhuru antivirus information\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  --help   -h              print help and quit\n");
  fprintf(stderr, "  --local  -l              do not use the scan daemon\n");
  fprintf(stderr, "  --xml    -x              output information as XML\n");
  fprintf(stderr, "\n");

  exit(1);
}

static void parse_options(int argc, char *argv[], struct info_options *opts)
{
  int c;

  opts->output_xml = 0;
  opts->use_daemon = 1;

  while (1) {
    int option_index = 0;

    c = getopt_long(argc, argv, "hlx", long_options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 'h':
      usage();
      break;
    case 'l':
      opts->use_daemon = 0;
      break;
    case 'x':
      opts->output_xml = 1;
      break;
    default:
      usage();
    }
  }
}

static xmlDocPtr info_doc_new(void)
{
  xmlDocPtr doc;
  xmlNodePtr root_node;

  LIBXML_TEST_VERSION;

  doc = xmlNewDoc("1.0");
  root_node = xmlNewNode(NULL, "uhuru-info");
#if 0
  xmlNewProp(root_node, "xmlns", "http://www.uhuru-am.com/UpdateInfoSchema");
  xmlNewProp(root_node, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
  xmlNewProp(root_node, "xsi:schemaLocation", "http://www.uhuru-am.com/UpdateInfoSchema UpdateInfoSchema.xsd ");
#endif
  xmlDocSetRootElement(doc, root_node);

  return doc;
}

static const char *update_status_str(enum uhuru_update_status status)
{
  switch(status) {
  case UHURU_UPDATE_NON_AVAILABLE:
    return "non available";
  case UHURU_UPDATE_OK:
    return "ok";
  case UHURU_UPDATE_LATE:
    return "late";
  case UHURU_UPDATE_CRITICAL:
    return "critical";
  }

  return "non available";
}

static void info_doc_add_module(xmlDocPtr doc, struct uhuru_module_info *info)
{
  xmlNodePtr root_node, module_node, base_node, date_node;
  struct uhuru_base_info **pinfo;
  char buffer[64];

  root_node = xmlDocGetRootElement(doc);

  module_node = xmlNewChild(root_node, NULL, "module", NULL);
  xmlNewProp(module_node, "name", info->name);

  xmlNewChild(module_node, NULL, "update-status", update_status_str(info->mod_status));

  date_node = xmlNewChild(module_node, NULL, "update-date", info->update_date);
  xmlNewProp(date_node, "type", "xs:dateTime");

  for(pinfo = info->base_infos; *pinfo != NULL; pinfo++) {
    base_node = xmlNewChild(module_node, NULL, "base", NULL);
    xmlNewProp(base_node, "name", (*pinfo)->name);

    date_node = xmlNewChild(base_node, NULL, "date", (*pinfo)->date);
    xmlNewProp(date_node, "type", "xs:dateTime");

    xmlNewChild(base_node, NULL, "version", (*pinfo)->version);
    sprintf(buffer, "%d", (*pinfo)->signature_count);
    xmlNewChild(base_node, NULL, "signature-count", buffer);
    xmlNewChild(base_node, NULL, "full-path", (*pinfo)->full_path);
  }
}

static void info_doc_add_global(xmlDocPtr doc, enum uhuru_update_status global_update_status)
{
  xmlNodePtr root_node = xmlDocGetRootElement(doc);

  xmlNewChild(root_node, NULL, "update-status", update_status_str(global_update_status));
}

static void info_doc_save_to_fd(xmlDocPtr doc, int fd)
{
  xmlSaveCtxtPtr xmlCtxt = xmlSaveToFd(fd, "UTF-8", XML_SAVE_FORMAT);

  if (xmlCtxt != NULL) {
    xmlSaveDoc(xmlCtxt, doc);
    xmlSaveClose(xmlCtxt);
  }
}

static void info_doc_free(xmlDocPtr doc)
{
  xmlFreeDoc(doc);
}

static void info_save_to_xml(struct uhuru_info *info)
{
  xmlDocPtr doc = info_doc_new();
  struct uhuru_module_info **m;

  info_doc_add_global(doc, info->global_status);

  if (info->module_infos != NULL) {
  for(m = info->module_infos; *m != NULL; m++)
    info_doc_add_module(doc, *m);
  }

  info_doc_save_to_fd(doc, STDOUT_FILENO);
  info_doc_free(doc);
}

static void do_info(struct info_options *opts)
{
  struct uhuru *u;
  struct uhuru_info *info;
  
  u = uhuru_open(opts->use_daemon);
  info = uhuru_info_new(u);

  if (opts->output_xml)
    info_save_to_xml(info);
  else
    fprintf(stderr, "only XML output implemented!\n");

  uhuru_info_free(info);

  uhuru_close(u);
}

int main(int argc, char **argv)
{
  struct info_options *opts = (struct info_options *)malloc(sizeof(struct info_options));

  parse_options(argc, argv, opts);

  do_info(opts);

  return 0;
}

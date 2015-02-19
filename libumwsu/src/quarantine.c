#include <libumwsu/scan.h>
#include <libumwsu/module.h>
#include "dir.h"
#include "modulep.h"
#include "quarantine.h"

#include <string.h>

#if 0
mettre en quarantaine:
- vérifier que le directory de quarantaine est en mode 0300 (wx)
=> peut être modifié par l utilisateur, donc pas terrible
=> utiliser un répertoire en 0333 appartenant à root
- déplacer le fichier dans le directory de quarantaine avec un nouveau nom aléatoire
- changer le mode du nouveau fichier à 00
#endif

static char *quarantine_dir;

static enum umwsu_mod_status mod_quarantine_conf(void *mod_data, const char *key, const char *value)
{
  if (!strcmp(key, "quarantine-dir")) {
    fprintf(stderr, "quarantine: got config %s -> %s\n", key, value);
    quarantine_dir = strdup(value);
    mkdir_p(quarantine_dir);
  }

  return UMWSU_MOD_OK;
}

struct umwsu_module umwsu_mod_quarantine = {
  .init = NULL,
  .conf = &mod_quarantine_conf,
  .scan = NULL,
  .close = NULL,
  .name = "quarantine",
  .mime_types = NULL,
  .data = NULL,
};

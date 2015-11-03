#include "infomodel.h"
#include "utils/ipc.h"

#include <iostream>
#include <QDebug>

BaseInfo::BaseInfo(const QString &name, const QString &date, const QString &version, unsigned int signatureCount, const QString &fullPath)
  : _name(name), _date(date), _version(version), _signatureCount(signatureCount), _fullPath(fullPath)
{
}

ModuleInfo::ModuleInfo(const QString &name, enum UpdateStatus status, const QString &updateDate)
  : _name(name), _status(status), _updateDate(updateDate)
{
}

InfoModel::InfoModel(enum UpdateStatus globalStatus)
  : _globalStatus(globalStatus)
{
}

static enum UpdateStatus statusFromStr(const char *status)
{
  if (!strcmp(status, "UHURU_UPDATE_OK"))
    return  UPDATE_OK;
  if (!strcmp(status, "UHURU_UPDATE_LATE"))
    return UPDATE_LATE;
  if (!strcmp(status, "UHURU_UPDATE_CRITICAL"))
    return UPDATE_CRITICAL;
  if (!strcmp(status, "UHURU_UPDATE_NON_AVAILABLE"))
    return UPDATE_NON_AVAILABLE;

  return -42;
}

static void ipc_handler_info_module(struct ipc_manager *manager, void *data)
{
  InfoModel *model = static_cast<InfoModel *>(data);

  struct ipc_handler_info_data *handler_data = (struct ipc_handler_info_data *)data;
  struct uhuru_module_info *mod_info = g_new0(struct uhuru_module_info, 1);
  int n_bases, i, argc;
  char *mod_name, *mod_status, *update_date;

  ipc_manager_get_arg_at(m, 0, IPC_STRING_T, &mod_name);
  ipc_manager_get_arg_at(m, 1, IPC_STRING_T, &mod_status);
  ipc_manager_get_arg_at(m, 2, IPC_STRING_T, &update_date);



  n_bases = (ipc_manager_get_argc(m) - 3) / 5;

  mod_info->base_infos = g_new0(struct uhuru_base_info *, n_bases + 1);

  argc = 3;

  for (i = 0; i < n_bases; i++, argc += 5) {
    struct uhuru_base_info *base_info = g_new(struct uhuru_base_info, 1);
    char *name, *date, *version, *full_path;

    ipc_manager_get_arg_at(m, argc+0, IPC_STRING_T, &name);
    base_info->name = os_strdup(name);
    ipc_manager_get_arg_at(m, argc+1, IPC_STRING_T, &date);
    base_info->date = os_strdup(date);
    ipc_manager_get_arg_at(m, argc+2, IPC_STRING_T, &version);
    base_info->version = os_strdup(version);
    ipc_manager_get_arg_at(m, argc+3, IPC_INT32_T, &base_info->signature_count);
    ipc_manager_get_arg_at(m, argc+4, IPC_STRING_T, &full_path);
    base_info->full_path = os_strdup(full_path);

    mod_info->base_infos[i] = base_info;
  }

  g_array_append_val(handler_data->g_module_infos, mod_info);
}

static void ipc_handler_info_end(struct ipc_manager *manager, void *data)
{
  InfoModel *model = static_cast<InfoModel *>(data);
}

void InfoModelThread::run()
{
  struct ipc_manager *manager = ipc_manager_new(DEFAULT_SOCKET_PATH);

  ipc_manager_add_handler(manager, IPC_MSG_ID_INFO_MODULE, ipc_handler_info_module, _model);
  ipc_manager_add_handler(manager, IPC_MSG_ID_INFO_END, ipc_handler_info_end, _model);

  ipc_manager_msg_send(manager, IPC_MSG_ID_INFO, IPC_NONE_T);

  while (ipc_manager_receive(manager) > 0)
    ;

  ipc_manager_free(manager);
}


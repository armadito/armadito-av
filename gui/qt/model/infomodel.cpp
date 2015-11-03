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

  return static_cast<enum UpdateStatus>(-42);
}

static void ipc_handler_info_module(struct ipc_manager *manager, void *data)
{
  InfoModel *model = static_cast<InfoModel *>(data);
  char *modName, *modStatus, *updateDate;

  ipc_manager_get_arg_at(manager, 0, IPC_STRING_T, &modName);
  ipc_manager_get_arg_at(manager, 1, IPC_STRING_T, &modStatus);
  ipc_manager_get_arg_at(manager, 2, IPC_STRING_T, &updateDate);

  ModuleInfo modInfo(modName, statusFromStr(modStatus), updateDate);

  for (int argc = 3; argc < ipc_manager_get_argc(manager); argc += 5) {
    char *name, *date, *version, *fullPath;
    unsigned int signatureCount;

    ipc_manager_get_arg_at(manager, argc+0, IPC_STRING_T, &name);
    ipc_manager_get_arg_at(manager, argc+1, IPC_STRING_T, &date);
    ipc_manager_get_arg_at(manager, argc+2, IPC_STRING_T, &version);
    ipc_manager_get_arg_at(manager, argc+3, IPC_INT32_T, &signatureCount);
    ipc_manager_get_arg_at(manager, argc+4, IPC_STRING_T, &fullPath);

    BaseInfo baseInfo(name, date, version, signatureCount, fullPath);
    modInfo.baseInfos().append(baseInfo);
  }

  model->moduleInfos().append(modInfo);
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


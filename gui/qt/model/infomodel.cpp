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

static void ipc_handler_info_module(struct ipc_manager *manager, void *data)
{
  InfoModel *i = static_cast<InfoModel *>(data);
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


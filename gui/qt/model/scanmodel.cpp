#include "scanmodel.h"
#include "utils/ipc.h"

#include <iostream>
#include <QDebug>

static void ipc_handler_scan_file(struct ipc_manager *manager, void *data)
{
  ScanModel *s = static_cast<ScanModel *>(data);
  char *path, *status, *mod_name, *mod_report, *action;
  int progress;

  ipc_manager_get_arg_at(manager, 0, IPC_STRING_T, &path);
  ipc_manager_get_arg_at(manager, 1, IPC_STRING_T, &status);
  ipc_manager_get_arg_at(manager, 2, IPC_STRING_T, &mod_name);
  ipc_manager_get_arg_at(manager, 3, IPC_STRING_T, &mod_report);
  ipc_manager_get_arg_at(manager, 4, IPC_STRING_T, &action);
  ipc_manager_get_arg_at(manager, 5, IPC_INT32_T, &progress);

  s->callback(path, status, mod_name, mod_report, action, progress);
}

void ScanModelThread::run()
{
  QByteArray ba = _model->path().toLocal8Bit();
  const char *c_path = ba.data();

  struct ipc_manager *manager = ipc_manager_new(_model->daemonFd());

  ipc_manager_add_handler(manager, IPC_MSG_ID_SCAN_FILE, ipc_handler_scan_file, _model);

  ipc_manager_msg_send(manager, IPC_MSG_ID_SCAN, IPC_STRING_T, c_path, IPC_NONE_T);

  while (ipc_manager_receive(manager) > 0)
    ;

  ipc_manager_free(manager);
}

void ScanModel::scan()
{
  ScanModelThread *scanThread = new ScanModelThread(this);

  QObject::connect(scanThread, SIGNAL(finished()), this, SLOT(scanThreadFinished()));

  scanThread->start();
}

void ScanModel::scanThreadFinished()
{
  emit scanComplete();
  _completed = true;
}

void ScanModel::callback(const char *path, const char *status, const char *mod_name, const char *mod_report, const char *action, int progress)
{
  _progress.set(progress);

  if (!*path)
    return;

  qDebug() << "DEBUG: ScanMode::callback - file path : " << path;

  emit scanning(QString(path));

  _scannedCount.increment();

  if (!strcmp(status, "UHURU_MALWARE")) {
    _malwareCount.increment();
    _reportModel.append(QString("Malicieux"), QString("aucune"), QString(path), QString(mod_report));
  } else if (!strcmp(status, "UHURU_SUSPICIOUS")) {
    _suspiciousCount.increment();
    _reportModel.append(QString("Suspect"), QString("aucune"), QString(path), QString(mod_report));
  } else if (!strcmp(status, "UHURU_EINVAL") 
	     || !strcmp(status, "UHURU_IERROR")
	     || !strcmp(status, "UHURU_UNKNOWN_FILE_TYPE")
	     || !strcmp(status, "UHURU_UNDECIDED")) {
    _unhandledCount.increment();
    _reportModel.append(tr("Not processed"), QString("aucune"), QString(path), "");
  } else if (!strcmp(status, "UHURU_WHITE_LISTED")
	     || !strcmp(status, "UHURU_CLEAN")) {
    _cleanCount.increment();
  }
}

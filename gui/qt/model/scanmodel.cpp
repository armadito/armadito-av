#include "scanmodel.h"

#include <QDirIterator>
#include <iostream>
#include <QDebug>

static void scanmodel_callback(struct uhuru_report *report, void *callback_data);

void ScanModelThread::run()
{
  QByteArray ba = _model->path().toLocal8Bit();
  const char *c_path = ba.data();
  //  enum uhuru_scan_flags flags = static_cast<enum uhuru_scan_flags>(0);
  //enum uhuru_scan_flags flags = static_cast<enum uhuru_scan_flags>(UHURU_SCAN_RECURSE | UHURU_SCAN_THREADED);
  enum uhuru_scan_flags flags = static_cast<enum uhuru_scan_flags>(UHURU_SCAN_RECURSE);

  struct uhuru_scan *scan = uhuru_scan_new(UHURU::instance(), 1, c_path, flags);

  uhuru_scan_add_callback(scan, scanmodel_callback, _model);

  uhuru_scan_run(scan);

  uhuru_scan_free(scan);
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

void ScanModel::callback(enum uhuru_file_status status, const char *path, const char *report)
{

  qDebug() << "DEBUG: ScanMode::callback - file path : " << path;

  emit scanning(QString(path));

  _scannedCount.increment();

  switch(status) {
  case UHURU_MALWARE:
    _malwareCount.increment();
    _reportModel.append(QString("Malicieux"), QString("aucune"), QString(path), QString(report));
    break;
  case UHURU_SUSPICIOUS:
    _suspiciousCount.increment();
    _reportModel.append(QString("Suspect"), QString("aucune"), QString(path), QString(report));
    break;
  case UHURU_EINVAL:
  case UHURU_IERROR:
  case UHURU_UNKNOWN_FILE_TYPE:
  case UHURU_UNDECIDED:
    _unhandledCount.increment();
    _reportModel.append(tr("Not processed"), QString("aucune"), QString(path), "");
    break;
  case UHURU_WHITE_LISTED:
  case UHURU_CLEAN:
    _cleanCount.increment();
    break;
  }
}

static void scanmodel_callback(struct uhuru_report *report, void *callback_data)
{
  ScanModel *s = static_cast<ScanModel *>(callback_data);

  s->callback(report->status, report->path, report->mod_report);
}


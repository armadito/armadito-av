#include "scanmodel.h"

#include <QDirIterator>
#include <iostream>

static void scanmodel_callback(struct umwsu_report *report, void *callback_data);

void ScanModelThread::run()
{
  _model->countFiles();

  QByteArray ba = _model->path().toLocal8Bit();
  const char *c_path = ba.data();
  //  enum umwsu_scan_flags flags = static_cast<enum umwsu_scan_flags>(0);
  //enum umwsu_scan_flags flags = static_cast<enum umwsu_scan_flags>(UMWSU_SCAN_RECURSE | UMWSU_SCAN_THREADED);
  enum umwsu_scan_flags flags = static_cast<enum umwsu_scan_flags>(UMWSU_SCAN_RECURSE);

  struct umwsu_scan *scan = umwsu_scan_new(UMWSU::instance(), c_path, flags);

  umwsu_scan_add_callback(scan, scanmodel_callback, _model);

  umwsu_scan_start(scan);

#if 0
  umwsu_scan_finish(scan);
#endif

  umwsu_scan_free(scan);
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

void ScanModel::countFiles()
{
  QDirIterator iter(_path, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
  int count = 0;

  for( ; iter.hasNext(); iter.next())
    count++;

  _totalCount.set(count);
}

void ScanModel::callback(enum umwsu_file_status status, const char *path, const char *report)
{

  emit scanning(QString(path));

  _scannedCount.increment();

  switch(status) {
  case UMWSU_MALWARE:
    _malwareCount.increment();
    _reportModel.append(QString("malicieux"), QString("aucune"), QString(path), QString(report));
    break;
  case UMWSU_SUSPICIOUS:
    _suspiciousCount.increment();
    _reportModel.append(QString("suspect"), QString("aucune"), QString(path), QString(report));
    break;
  case UMWSU_EINVAL:
  case UMWSU_IERROR:
  case UMWSU_UNKNOWN_FILE_TYPE:
  case UMWSU_UNDECIDED:
    _unhandledCount.increment();
    _reportModel.append(QString("non traite"), QString("aucune"), QString(path), "");
    break;
  case UMWSU_WHITE_LISTED:
  case UMWSU_CLEAN:
    _cleanCount.increment();
    _reportModel.append(QString("sain"), QString("aucune"), QString(path), "");
    break;
  }
}

static void scanmodel_callback(struct umwsu_report *report, void *callback_data)
{
  ScanModel *s = static_cast<ScanModel *>(callback_data);

  s->callback(report->status, report->path, report->mod_report);
}


#include "scanmodel.h"

#include <QDirIterator>

static void scanmodel_callback(struct umwsu_report *report, void *callback_data);

void ScanModel::scan()
{
  countFiles();

  QByteArray ba = _path.toLocal8Bit();
  const char *c_path = ba.data();
  //  enum umwsu_scan_flags flags = static_cast<enum umwsu_scan_flags>(0);
  enum umwsu_scan_flags flags = UMWSU_SCAN_RECURSE;

  struct umwsu_scan *scan = umwsu_scan_new(UMWSU::instance(), c_path, flags);

  umwsu_scan_add_callback(scan, scanmodel_callback, this);

  umwsu_scan_start(scan);
  umwsu_scan_finish(scan);
  umwsu_scan_free(scan);
}

void ScanModel::countFiles()
{
  QDirIterator iter(_path, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
  int count = 0;

  for( ; iter.hasNext(); iter.next())
    count++;

  _fileCount.set(count);
}

void ScanModel::callback(enum umwsu_status status)
{
  switch(status) {
  case UMWSU_CLEAN:
    _cleanCount.increment();
    break;
  case UMWSU_SUSPICIOUS:
    _suspiciousCount.increment();
    break;
  case UMWSU_MALWARE:
    _malwareCount.increment();
    break;
  case UMWSU_EINVAL:
  case UMWSU_IERROR:
  case UMWSU_UNKNOWN_FILE_TYPE:
  case UMWSU_UNDECIDED:
  case UMWSU_WHITE_LISTED:
    break;
  }

  _scannedCount.increment();
}

static void scanmodel_callback(struct umwsu_report *report, void *callback_data)
{
  ScanModel *s = static_cast<ScanModel *>(callback_data);

  s->callback(report->status);
}


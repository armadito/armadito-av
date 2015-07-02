#ifndef _SCANMODEL_H_
#define _SCANMODEL_H_

#include "counter.h"
#include "utils/uhuru.h"
#include "scanreportmodel.h"

#include <QObject>
#include <QThread>
#include <QDateTime>

class ScanModel : public QObject {
  Q_OBJECT

public:
  explicit ScanModel(const QString &path) : _path(path), _startDate(QDateTime::currentDateTime()), _completed(false) {}
  ~ScanModel() {}

  const QString &path() { return _path; }
  const QDateTime &startDate() { return _startDate; }
  const bool completed() { return _completed; }

  void scan();

  Counter *totalCount() { return &_totalCount; }
  Counter *scannedCount() { return &_scannedCount; }
  Counter *malwareCount() { return &_malwareCount; }
  Counter *suspiciousCount() { return &_suspiciousCount; }
  Counter *unhandledCount() { return &_unhandledCount; }
  Counter *cleanCount() { return &_cleanCount; }

  ScanReportModel *report() { return &_reportModel; }

  void countFiles();

  void callback(enum uhuru_file_status status, const char *path, const char *report);

signals:
  void scanning(const QString &path);
  void scanComplete();

private slots:
  void scanThreadFinished();

private:
  QString _path;
  QDateTime _startDate;
  bool _completed;

  Counter _totalCount;
  Counter _scannedCount;
  Counter _malwareCount;
  Counter _suspiciousCount;
  Counter _unhandledCount;
  Counter _cleanCount;

  ScanReportModel _reportModel;
};

class ScanModelThread: public QThread {
  Q_OBJECT

public:
  ScanModelThread(ScanModel *model) : _model(model) {}

protected:
  void run();

private:
  ScanModel *_model;
};

#endif

#include "counter.h"
#include "utils/umwsu.h"

#include <QObject>

class ScanModel : public QObject {
  Q_OBJECT

public:
  explicit ScanModel(const QString &path) : _path(path) {}
  ~ScanModel() {}

  void scan();

  Counter *fileCount() { return &_fileCount; }
  Counter *scannedCount() { return &_scannedCount; }
  Counter *malwareCount() { return &_malwareCount; }
  Counter *suspiciousCount() { return &_suspiciousCount; }
  Counter *cleanCount() { return &_cleanCount; }

  void callback(enum umwsu_status status);

private:
  void countFiles();

  QString _path;

  Counter _fileCount;
  Counter _scannedCount;
  Counter _malwareCount;
  Counter _suspiciousCount;
  Counter _cleanCount;
};

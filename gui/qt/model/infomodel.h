#ifndef _MODEL_INFOMODEL_H_
#define _MODEL_INFOMODEL_H_

#include <QObject>
#include <QVector>
#include <QThread>

enum UpdateStatus {
  UPDATE_OK,
  UPDATE_LATE,
  UPDATE_CRITICAL,
  UPDATE_NON_AVAILABLE,
  UPDATE_UNKNOWN,
};

class BaseInfo {

public:
  BaseInfo() : _name(""), _date(""), _version(""), _signatureCount(0), _fullPath("") {} 
  BaseInfo(const QString &name, const QString &date, const QString &version, unsigned int signatureCount, const QString &fullPath)
    : _name(name), _date(date), _version(version), _signatureCount(signatureCount), _fullPath(fullPath) {} 
  ~BaseInfo() {}

  const QString &name() const { return _name; }
  const QString &date() const { return _date; }
  const QString &version() const { return _version; }
  unsigned int signatureCount() const { return _signatureCount; }
  const QString &fullPath() const { return _fullPath; }

private:
  QString _name;
  /* UTC and ISO 8601 date */
  QString _date;
  QString _version;
  unsigned int _signatureCount;
  QString _fullPath;
};

class ModuleInfo {
public:
  ModuleInfo() : _name(""), _status(UPDATE_UNKNOWN), _updateDate("") {}
  ModuleInfo(const QString &name, enum UpdateStatus status, const QString &updateDate)
    : _name(name), _status(status), _updateDate(updateDate) {}
  ~ModuleInfo() {}

  const QString &name() const { return _name; }
  enum UpdateStatus status() const { return _status; }
  const QString &updateDate() const { return _updateDate; }
  QVector<BaseInfo> &baseInfos() { return _baseInfos; }
  const QVector<BaseInfo> &baseInfos() const { return _baseInfos; }

private:
  QString _name;
  enum UpdateStatus _status;
  /* UTC and ISO 8601 date time */
  QString _updateDate;
  QVector<BaseInfo> _baseInfos;
};

class InfoModel : public QObject {
  Q_OBJECT

public:
  explicit InfoModel(int daemonFd, enum UpdateStatus globalStatus = UPDATE_UNKNOWN)
    : _daemonFd(daemonFd), _globalStatus(globalStatus) {}

  ~InfoModel() {}

  QVector<ModuleInfo> &moduleInfos() { return _moduleInfos; }
  enum UpdateStatus &globalStatus() { return _globalStatus; }
  int daemonFd() { return _daemonFd; }

  void doUpdate();

  void debug();

signals:
  void updated();

private slots:
  void infoThreadFinished();

private:
  enum UpdateStatus _globalStatus;
  QVector<ModuleInfo> _moduleInfos;
  int _daemonFd;
};

class InfoModelThread: public QThread {
  Q_OBJECT

public:
  InfoModelThread(InfoModel *model) : _model(model) {}

  // const InfoModel &model() { return *_model; }

protected:
  void run();

private:
  InfoModel *_model;
};

#endif

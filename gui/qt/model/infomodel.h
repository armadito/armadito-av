#ifndef _MODEL_INFOMODEL_H_
#define _MODEL_INFOMODEL_H_

#include <QObject>
#include <QList>
#include <QThread>

enum UpdateStatus {
  UPDATE_OK,
  UPDATE_LATE,
  UPDATE_CRITICAL,
  UPDATE_NON_AVAILABLE,
};

class BaseInfo {

public:
  BaseInfo(const QString &name, const QString &date, const QString &version, unsigned int signatureCount, const QString &fullPath); 
  ~BaseInfo() {}

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
  ModuleInfo(const QString &name, enum UpdateStatus status, const QString &updateDate);
  ~ModuleInfo() {}

  const QString &name() { return _name; }
  enum UpdateStatus status() { return _status; }
  
private:
  QString _name;
  enum UpdateStatus _status;
  /* UTC and ISO 8601 date time */
  QString _updateDate;
  QList<BaseInfo> _baseInfos;
};

class InfoModel : public QObject {
  Q_OBJECT

public:
  explicit InfoModel(enum UpdateStatus globalStatus);
  ~InfoModel() {}

private:
  enum UpdateStatus _globalStatus;
  QList<ModuleInfo> _moduleInfos;
};

class InfoModelThread: public QThread {
  Q_OBJECT

public:
  InfoModelThread() {}

  const InfoModel &model() { return *_model; }

protected:
  void run();

private:
  InfoModel *_model;
};

#endif

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
};

class BaseInfo {

public:
  BaseInfo() : _name(""), _date(""), _version(""), _signatureCount(0), _fullPath("") {} 
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
  ModuleInfo() : _name(""), _status(UPDATE_OK), _updateDate("") {}
  ModuleInfo(const QString &name, enum UpdateStatus status, const QString &updateDate);
  ~ModuleInfo() {}

  const QString &name() { return _name; }
  enum UpdateStatus status() { return _status; }
  QVector<BaseInfo> &baseInfos() { return _baseInfos; }

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
  explicit InfoModel(enum UpdateStatus globalStatus);
  ~InfoModel() {}

  QVector<ModuleInfo> &moduleInfos() { return _moduleInfos; }

private:
  enum UpdateStatus _globalStatus;
  QVector<ModuleInfo> _moduleInfos;
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

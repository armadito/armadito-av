#ifndef _SCANREPORTMODEL_H_
#define _SCANREPORTMODEL_H_

#include <QAbstractTableModel>
#include <QVector>

struct report_entry {
  QString status;
  QString action;
  QString path;
  QString report;
};

class ScanReportModel : public QAbstractTableModel {
  Q_OBJECT

public:
  ScanReportModel(QObject *parent = 0);

  int rowCount(const QModelIndex &parent = QModelIndex()) const;

  int columnCount(const QModelIndex &parent = QModelIndex()) const;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  Qt::ItemFlags flags(const QModelIndex &index) const;

  void append(const QString &status, const QString &action, const QString &path, const QString &report);

private:

  QVector<struct report_entry> _reports;
};

#endif

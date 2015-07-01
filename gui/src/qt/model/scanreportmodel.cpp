#include "scanreportmodel.h"
#include <iostream>
#include <QDebug>
#include <QFileInfo>

ScanReportModel::ScanReportModel(QObject *parent)
  :QAbstractTableModel(parent)
{
}

int ScanReportModel::rowCount(const QModelIndex & /*parent*/) const
{
  return _reports.size();
}

int ScanReportModel::columnCount(const QModelIndex &parent) const 
{ 
  return 3; 
}

QVariant ScanReportModel::data(const QModelIndex &index, int role) const
{
  if (role == Qt::DisplayRole) {
    switch(index.column()) {
    case 0:
      return _reports.at(index.row()).status;
#if 0
    case 1:
      return _reports.at(index.row()).action;
#endif
    case 1:
      return _reports.at(index.row()).report;
    case 2:
      return _reports.at(index.row()).path;
    }
  }

  return QVariant();
}

QVariant ScanReportModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Horizontal) {
      switch (section) {
      case 0:
	return QString("Etat");
#if 0
      case 1:
	return QString("Action");
#endif
      case 1:
	return QString("Detail");
      case 2:
	return QString("Fichier");
      }
    }
  } 

#if 0
  if (role == Qt::SizeHintRole)
    qDebug() << "ask for SizeHint";
#endif

  return QVariant();
}

Qt::ItemFlags ScanReportModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::ItemIsEnabled;

  int row = index.row();
  int col = index.column();

#if 0
  qDebug() << QString("row %1, col %2").arg(row).arg(col);
#endif

  Qt::ItemFlags flags = QAbstractTableModel::flags(index);

  if (row == 2)
    flags |= Qt::ItemIsEditable;

  // return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
  return flags;
}

void ScanReportModel::append(const QString &status, const QString &action, const QString &path, const QString &report)
{
  int lastRow = rowCount();

  beginInsertRows(QModelIndex(), lastRow, lastRow);

  QFileInfo info(path);

  struct report_entry entry = { status, action, info.fileName(), report};

  _reports.append(entry);

  endInsertRows();

#if 0
  int row = _reports.size() - 1;
  QModelIndex topLeft = createIndex(row,0);
  QModelIndex bottomRight = createIndex(row,2);

  emit dataChanged(topLeft, bottomRight);
#endif
}


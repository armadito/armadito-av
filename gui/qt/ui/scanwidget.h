#ifndef SCANWIDGET_H
#define SCANWIDGET_H

#include "model/scanmodel.h"
#include "model/scanreportmodel.h"

#include <QWidget>
#include <QPushButton>
#include <QtGui>
#include <QtCore>

namespace Ui {
  class ScanWidget;
}

class ScanWidget : public QWidget
{
  Q_OBJECT

 public:
  explicit ScanWidget(ScanModel *model, QWidget *parent = 0);
  ~ScanWidget();

 public slots:
  void afterEndInsert();

 private slots:
  void enableCloseButton();
  void on_closeButton_clicked();

 private:
  void doConnect(ScanModel *model);
  void connectLineEdit(const char *lineEditName, Value *value);

  Ui::ScanWidget *ui;
  ScanModel *model;
  QLabel *ui_labelTitle; 
  QPushButton *ui_closeButton;
  QTableView *ui_reportView;

};

#endif

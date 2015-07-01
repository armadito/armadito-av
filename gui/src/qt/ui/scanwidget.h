#ifndef SCANWIDGET_H
#define SCANWIDGET_H

#include "model/scanmodel.h"
#include "model/scanreportmodel.h"

#include <QWidget>
#include <QPushButton>

namespace Ui {
  class ScanWidget;
}

class ScanWidget : public QWidget
{
  Q_OBJECT

 public:
  explicit ScanWidget(ScanModel *model, QWidget *parent = 0);
  ~ScanWidget();

 private slots:
  void enableCloseButton();
  void on_closeButton_clicked();

 private:
  void doConnect(ScanModel *model);
  void connectLineEdit(const char *lineEditName, Counter *counter);

  Ui::ScanWidget *ui;
  ScanModel *model;
  QPushButton *ui_closeButton;    
};

#endif

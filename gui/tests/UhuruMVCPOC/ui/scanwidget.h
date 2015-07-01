#ifndef SCANWIDGET_H
#define SCANWIDGET_H

#include "model/scanmodel.h"

#include <QWidget>

namespace Ui {
class ScanWidget;
}

class ScanWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScanWidget(QWidget *parent = 0);
    ~ScanWidget();

public slots:
    void on_scanButton_clicked()
    {
      model->scan();
    }

private:
    Ui::ScanWidget *ui;
    ScanModel *model;
};

#endif

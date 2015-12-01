#ifndef SCANWINDOW_H
#define SCANWINDOW_H

#include <QMainWindow>
#include "model/scanmodel.h"

class ScanWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ScanWindow(ScanModel *model, QWidget *parent = 0);
    ~ScanWindow() {}

private:
    void construct(ScanModel *model);
    QIcon *getIcon();
    ScanModel *_model;
};

#endif

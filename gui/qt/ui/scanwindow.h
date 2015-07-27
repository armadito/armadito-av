#ifndef SCANWINDOW_H
#define SCANWINDOW_H

#include <QMainWindow>
#include "model/scanmodel.h"

#if 0
namespace Ui {
class MainWindow;
}
#endif

class ScanWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ScanWindow(ScanModel *model, QWidget *parent = 0);
    explicit ScanWindow(const QString &path, QWidget *parent = 0);
    ~ScanWindow() {}

private:
    void construct(ScanModel *model);
    ScanModel *_model;
};

#endif

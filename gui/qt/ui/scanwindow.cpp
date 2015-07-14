#include "scanwindow.h"
#include "scanwidget.h"

//#include <QVBoxLayout>
#include <QApplication>
#include <QDesktopWidget>

ScanWindow::ScanWindow(ScanModel *model, QWidget *parent) :
    QMainWindow(parent)
{
  _model = model;

  ScanWidget *s = new ScanWidget(_model);

  setCentralWidget(s);

  resize(800, 500);

  move(QApplication::desktop()->screen()->rect().center() - rect().center());

  _model->scan();
}

ScanWindow::ScanWindow(const QString &path, QWidget *parent) :
  ScanWindow(new ScanModel(path), parent)
{
}


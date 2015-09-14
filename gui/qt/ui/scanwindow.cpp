#include "scanwindow.h"
#include "scanwidget.h"

#include <QApplication>
#include <QDesktopWidget>

void ScanWindow::construct(ScanModel *model)
{
  _model = model;

  ScanWidget *s = new ScanWidget(_model);

  setCentralWidget(s);

  resize(800, 500);

  move(QApplication::desktop()->screen()->rect().center() - rect().center());

  _model->scan();
}

ScanWindow::ScanWindow(ScanModel *model, QWidget *parent) :
    QMainWindow(parent)
{
  construct(model);
}

ScanWindow::ScanWindow(const QString &path, QWidget *parent) :
    QMainWindow(parent)
{
  construct(new ScanModel(path));
}


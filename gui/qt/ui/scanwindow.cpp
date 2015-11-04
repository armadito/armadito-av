#include "scanwindow.h"
#include "scanwidget.h"

#include <QApplication>
#include <QDesktopWidget>

QIcon *ScanWindow::getIcon()
{
  return new QIcon(":/icons/uhuru_grey.svg");
}


void ScanWindow::construct(ScanModel *model)
{
  _model = model;

  ScanWidget *s = new ScanWidget(_model);

  setWindowTitle(QString("Analyse antivirale"));
  setCentralWidget(s);
  setWindowIcon(*getIcon());

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


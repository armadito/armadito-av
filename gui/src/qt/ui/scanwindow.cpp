#include "scanwindow.h"
#include "scanwidget.h"

//#include <QVBoxLayout>
#include <QApplication>
#include <QDesktopWidget>

ScanWindow::ScanWindow(ScanModel *model, QWidget *parent) :
    QMainWindow(parent)
{
  ScanWidget *s = new ScanWidget(model);

  setCentralWidget(s);

  resize(800, 500);

  move(QApplication::desktop()->screen()->rect().center() - rect().center());
}


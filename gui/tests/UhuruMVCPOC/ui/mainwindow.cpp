#include "mainwindow.h"
#include "scanwidget.h"
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
  ScanWidget *s = new ScanWidget;

  setCentralWidget(s);
}

MainWindow::~MainWindow()
{
}

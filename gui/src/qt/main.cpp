#include "ui/systray.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  QApplication::setQuitOnLastWindowClosed(false);

  Systray *st = new Systray();

  return a.exec();
}

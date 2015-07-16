#include "utils/dbus.h"
#include "ui/scanwindow.h"
#include <iostream>

QString UhuruDBusService::ping(const QString &arg)
{
  return QString("ping(\"%1\") got called").arg(arg);
}

void UhuruDBusService::scan(const QString &arg)
{
  std::cerr << "UhuruDBusService::scan(\"" << arg.toStdString().c_str() << "\")\n";

  ScanWindow *w = new ScanWindow(arg);
  w->show();
  w->activateWindow();
  w->raise();
}

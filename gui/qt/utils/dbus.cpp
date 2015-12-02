#include "utils/dbus.h"
#include "ui/scanwindow.h"
#include "ui/app.h"

#include <iostream>
#include <QSystemTrayIcon>

QString UhuruDBusService::ping(const QString &arg)
{
  return QString("ping(\"%1\") got called").arg(arg);
}

void UhuruDBusService::scan(const QString &arg)
{
  std::cerr << "UhuruDBusService::scan(\"" << arg.toStdString().c_str() << "\")\n";

#if 0
  ScanWindow *w = new ScanWindow(arg);
  w->show();
  w->activateWindow();
  w->raise();
#endif
}

// test with:
// gdbus call --session --dest org.uhuru.AntivirusService --object-path / --method org.uhuru.Antivirus.notifyDetection "/tmp/foo" "BigTrojan"
void UhuruDBusService::notifyDetection(const QString &path, const QString &virusName)
{
  UhuruApplication::instance()->systray()->notify(tr("Uhuru has detected a threat"), 
						  tr("File: %1\nVirus:%2").arg(path).arg(virusName), 
						  QSystemTrayIcon::Warning);
}


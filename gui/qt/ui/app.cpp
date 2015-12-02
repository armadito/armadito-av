#include "app.h"

#include "uhuru-qt-config.h"
#include "ui/systray.h"
#include "utils/dbus.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QDebug>
#include <QtDBus/QtDBus>

UhuruApplication *UhuruApplication::_uniqueInstance = 0;

#define SERVICE_NAME "org.uhuru.AntivirusService"

// to debug DBus launch with QDBUS_DEBUG=1 


int UhuruApplication::initDBus()
{
  if (!QDBusConnection::sessionBus().isConnected()) {
    fprintf(stderr, "Cannot connect to the D-Bus session bus.\n");
    return 1;
  }

  if (!QDBusConnection::sessionBus().registerService(SERVICE_NAME)) {
    fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));        
    return 1;
  }

  UhuruDBusService *service = new UhuruDBusService();
  if (!QDBusConnection::sessionBus().registerObject("/", service, QDBusConnection::ExportAllSlots)) {
    fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));        
    return 1;
  }

  return 0;
}

int UhuruApplication::initI18n()
{
  QLocale loc = QLocale::system();

  QTranslator qtTr;

  if (qtTr.load(QString("qt_") + loc.name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
    installTranslator(&qtTr);

  QTranslator *uhuruTr = new QTranslator();
  
  if (uhuruTr->load(QString(PACKAGE "_") + loc.name(), TRANSLATIONSDIR)) {
    installTranslator(uhuruTr);
    qDebug() << "loaded translator for " << QString(PACKAGE "_") + loc.name() << " in " TRANSLATIONSDIR;
  } else {
    qDebug() << "cannot load translator for " << QString(PACKAGE "_") + loc.name() << " in " TRANSLATIONSDIR;
    return 1;
  }

  return 0;
}

UhuruApplication:: UhuruApplication(int argc, char *argv[]) 
  : QApplication(argc, argv) 
{
  QApplication::setQuitOnLastWindowClosed(false);

  if (initDBus())
    throw UhuruApplicationInitializationFailure();

  if (initI18n())
    throw UhuruApplicationInitializationFailure();

  _systray = new Systray();
}

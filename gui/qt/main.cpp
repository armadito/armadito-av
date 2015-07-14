#include "uhuru-qt-config.h"
#include "ui/systray.h"
#include "utils/dbus.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QDebug>
#include <QtDBus/QtDBus>

#define SERVICE_NAME "org.uhuru.ScanService"

static int initDBus()
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

static int initI18n(QApplication &app)
{
  QLocale loc = QLocale::system();

  QTranslator qtTr;

  if (qtTr.load(QString("qt_") + loc.name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
    app.installTranslator(&qtTr);

  QTranslator uhuruTr;
  
  if (uhuruTr.load(QString(PACKAGE "_") + loc.name(), TRANSLATIONSDIR)) {
    app.installTranslator(&uhuruTr);
    qDebug() << "loaded translator for " << QString(PACKAGE "_") + loc.name() << " in " TRANSLATIONSDIR;
  } else {
    qDebug() << "cannot load translator for " << QString(PACKAGE "_") + loc.name() << " in " TRANSLATIONSDIR;
    return 1;
  }

  return 0;
}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  if (initDBus())
    return 1;

  if (initI18n(app))
    return 1;

  QApplication::setQuitOnLastWindowClosed(false);

  Systray *st = new Systray();

  return app.exec();
}

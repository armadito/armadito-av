#include "ui/systray.h"
#include "uhuru-qt-config.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QDebug>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  QLocale loc = QLocale::system();

  QTranslator qtTr;

  if (qtTr.load(QString("qt_") + loc.name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
    a.installTranslator(&qtTr);

  QTranslator uhuruTr;

  if (uhuruTr.load(QString(PACKAGE "_") + loc.name(), TRANSLATIONSDIR)) {
    a.installTranslator(&uhuruTr);
    qDebug() << "loaded translator for " << QString(PACKAGE "_") + loc.name() << " in " TRANSLATIONSDIR;
  } else
    qDebug() << "cannot load translator for " << QString(PACKAGE "_") + loc.name() << " in " TRANSLATIONSDIR;

  QApplication::setQuitOnLastWindowClosed(false);

  Systray *st = new Systray();

  return a.exec();
}

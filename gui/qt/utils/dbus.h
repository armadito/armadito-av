#ifndef _UTILS_DBUS_H_
#define _UTILS_DBUS_H_

#include <QtCore/QObject>

class UhuruDBusService: public QObject {
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.uhuru.ScanApplication")

 public slots:
  Q_SCRIPTABLE QString ping(const QString &arg);
  Q_SCRIPTABLE void scan(const QString &arg);
};

#endif

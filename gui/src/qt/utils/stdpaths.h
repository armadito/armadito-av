#ifndef _STDPATHS_H_
#define _STDPATHS_H_

#include <QString>

#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

class StdPaths {
 public:
  static QString desktopLocation()
  {
#if QT_VERSION >= 0x050000
    return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
#else
    return QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
#endif
  }

  static QString documentsLocation()
  {
#if QT_VERSION >= 0x050000
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#else
    return QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#endif
  }
};

#endif

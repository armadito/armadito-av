#ifndef APP_H
#define APP_H

#include "ui/systray.h"

#include <QApplication>
#include <exception>

class UhuruApplicationInitializationFailure: public std::exception {
public:
  virtual const char* what() const throw()
  {
    return "failure in application initialization";
  }
};

class UhuruApplication : public QApplication {
    Q_OBJECT

 public:
  static UhuruApplication *instance()
  {
    return _uniqueInstance;
  }

  static void init(int argc, char *argv[])
  {
    _uniqueInstance = new UhuruApplication(argc, argv);
  }

  Systray *systray() { return _systray; }

 private:
  static UhuruApplication *_uniqueInstance;

  UhuruApplication(int argc, char *argv[]);

  UhuruApplication(UhuruApplication const&);            // Don't Implement
  void operator=(UhuruApplication const&);   // Don't implement

  int initDBus();
  int initI18n();

  Systray *_systray;
};

#endif

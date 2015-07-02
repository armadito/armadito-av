#ifndef _UTILS_UHURU_H_
#define _UTILS_UHURU_H_

#include <libuhuru/scan.h>
#include <QThread>

class UHURUOpenThread;

class UHURU {
  friend class UHURUOpenThread;

 public:
  static struct uhuru *instance()
  {
    return _uniqueInstance->getHandle();
  }

 private:
  static UHURU *_uniqueInstance;

  struct uhuru *getHandle();

  UHURU();

  ~UHURU()
    {
      uhuru_close(_uhuruHandle);
    }

  UHURU(UHURU const&);            // Don't Implement
  void operator=(UHURU const&);   // Don't implement

  bool _openThreadTerminated;
  UHURUOpenThread *_openThread;
  struct uhuru *_uhuruHandle;
};

class UHURUOpenThread: public QThread {
  Q_OBJECT

 public:
  UHURUOpenThread(UHURU *uhuru) : _uhuru(uhuru) {}

protected:
  void run();

private:
  UHURU *_uhuru;
};

#endif

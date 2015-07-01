#ifndef _UTILS_UMWSU_H_
#define _UTILS_UMWSU_H_

#include <libumwsu/scan.h>
#include <QThread>

class UMWSUOpenThread;

class UMWSU {
  friend class UMWSUOpenThread;

 public:
  static struct umwsu *instance()
  {
    return _uniqueInstance->getHandle();
  }

 private:
  static UMWSU *_uniqueInstance;

  struct umwsu *getHandle();

  UMWSU();

  ~UMWSU()
    {
      umwsu_close(_umwsuHandle);
    }

  UMWSU(UMWSU const&);            // Don't Implement
  void operator=(UMWSU const&);   // Don't implement

  bool _openThreadTerminated;
  UMWSUOpenThread *_openThread;
  struct umwsu *_umwsuHandle;
};

class UMWSUOpenThread: public QThread {
  Q_OBJECT

 public:
  UMWSUOpenThread(UMWSU *umwsu) : _umwsu(umwsu) {}

protected:
  void run();

private:
  UMWSU *_umwsu;
};

#endif

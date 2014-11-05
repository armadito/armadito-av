#ifndef _UTILS_UMWSU_H_
#define _UTILS_UMWSU_H_

#include <libumwsu/scan.h>

class UMWSU {
 public:
  static struct umwsu *instance()
  {
    return get()._umwsu_handle;
  }

 private:
  static UMWSU &get()
  {
    static UMWSU e;

    return e;
  }

  UMWSU()
    {
      _umwsu_handle = umwsu_open();
      umwsu_print(_umwsu_handle);
    }

  ~UMWSU()
    {
      umwsu_close(_umwsu_handle);
    }

  UMWSU(UMWSU const&);        // Don't Implement

  void operator=(UMWSU const&);     // Don't implement

  struct umwsu *_umwsu_handle;
};

#endif

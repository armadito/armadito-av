#include "utils/uhuru.h"

UHURU *UHURU::_uniqueInstance = new UHURU();

UHURU::UHURU() : _openThreadTerminated(false), _openThread(new UHURUOpenThread(this))
{
  _openThread->start();
}

struct uhuru *UHURU::getHandle()
{
  if (!_openThreadTerminated) {
    _openThread->wait();
    _openThreadTerminated = true;
  }

  return _uhuruHandle;
}

void UHURUOpenThread::run()
{
  _uhuru->_uhuruHandle = uhuru_open(1);
#if 0
  uhuru_print(_uhuru->_uhuruHandle);
#endif
}


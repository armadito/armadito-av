#include "utils/umwsu.h"

UMWSU *UMWSU::_uniqueInstance = new UMWSU();

UMWSU::UMWSU() : _openThreadTerminated(false), _openThread(new UMWSUOpenThread(this))
{
  _openThread->start();
}

struct umwsu *UMWSU::getHandle()
{
  if (!_openThreadTerminated) {
    _openThread->wait();
    _openThreadTerminated = true;
  }

  return _umwsuHandle;
}

void UMWSUOpenThread::run()
{
  _umwsu->_umwsuHandle = umwsu_open(0);
  umwsu_print(_umwsu->_umwsuHandle);
}


#ifndef _UTILS_DAEMON_H_
#define _UTILS_DAEMON_H_

#define DEFAULT_SOCKET_PATH "@/tmp/.uhuru/daemon"

#include <QtCore/QObject>

class DaemonConnection: public QObject {
  Q_OBJECT
  
public:

  DaemonConnection(const char *socketPath = DEFAULT_SOCKET_PATH) : _socketPath(socketPath), _ioFd(-1) {}

  int connect(int maxRetry = 10);

private:
  const char *_socketPath;
  int _ioFd;
};


#endif

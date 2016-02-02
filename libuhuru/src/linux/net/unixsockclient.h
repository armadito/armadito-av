#ifndef _UNIXSOCKCLIENT_H_
#define _UNIXSOCKCLIENT_H_

int unix_client_connect(const char *socket_path, int max_retry);

#endif

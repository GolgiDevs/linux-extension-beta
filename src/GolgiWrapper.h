#ifndef __GOLGI_GOLGI_WRAPPER_H__
#define __GOLGI_GOLGI_WRAPPER_H__

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

void GolgiSetup(const char *instanceId,
                bool useEncryption);
int GolgiSelect(int nfds,
                fd_set *readFds,
                fd_set *writeFds,
                fd_set *exceptFds,
                struct timeval *tv);

#endif


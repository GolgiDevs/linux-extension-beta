#include "GolgiNetLinux.h"
#include "GolgiWrapper.h"

GolgiNetLinux *netLinux;

void GolgiSetup(const char *instanceId,
                bool useEncryption)
{
    if(useEncryption){
        DBG.println("Initialising SSL library");
        SSL_library_init();
        SSL_load_error_strings();
    }
    
    GolgiAPIImpl *runtime = new GolgiAPIImpl(instanceId);
    netLinux = new GolgiNetLinux(useEncryption);
    GolgiTransport *transport = new GolgiTransport(netLinux,
                                                   GOLGI_APPKEY,
                                                   GOLGI_DEVKEY);
    runtime->setSouthboundInterface(transport->getSouthboundInterface());
    transport->setNorthboundInterface(runtime->getNorthboundInterface());
}

int GolgiSelect(int nfds,
                fd_set *readFds,
                fd_set *writeFds,
                fd_set *exceptFds,
                struct timeval *tv)
{
    // This is a wrapper of the select function. We will add the GolgiNetLinux 
    // sockfd to the read and (if required) the write list. We will then make 
    // a standard select call. When the call returns we check for the 
    // GolgiNetLinux socket and process appropriately. Next we remove the 
    // GolgiNetLinux socket from the read/write lists and return to the user 
    // so that they can process their own sockets
 
    int sockfd = netLinux->getSockfd();

    // add the file descriptors as needed
    if(netLinux->connected() || 
        netLinux->SSLRequireRead()){

        FD_SET(sockfd,
               readFds);
    }
    if((netLinux->writePending() && netLinux->connected()) || 
        netLinux->waiting() || 
        netLinux->SSLRequireWrite()){

        FD_SET(sockfd,
               writeFds);
    }

    // update nfds
    if(sockfd >= nfds){
        nfds = sockfd + 1;
    }

    // call the select
    int n = ::select(nfds,
                     readFds,
                     writeFds,
                     exceptFds,
                     tv);

    // check if we have reads/writes available
    if(FD_ISSET(sockfd,
                readFds)){
        netLinux->readAvailable();
        FD_CLR(sockfd,
               readFds);
        n--;
    }
    if(FD_ISSET(sockfd,
                writeFds)){
        netLinux->writeAvailable();
        FD_CLR(sockfd,
               writeFds);
        n--;
    }

    GolgiService();

    return n;
}


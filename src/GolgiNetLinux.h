#ifndef __GOLGI_GOLGINETLINUX_H__
#define __GOLGI_GOLGINETLINUX_H__
#include <stdint.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "libgolgi.h"

class GolgiNetLinux : public GolgiNetInterface {
public:
    // functions to be implemented for Golgi functionality
    bool connect(const char *url, int32_t port);
    bool connected(void);
    bool waiting(void);
    void stop(void);
    int32_t available(void);
    int32_t read(void);
    int32_t read(void *dst, int32_t len);
    void write(const char *buf);
    
    void readAvailable(void);
    void writeAvailable(void);

    bool writePending(void);
    bool SSLRequireRead(void);
    bool SSLRequireWrite(void);

    int getSockfd(void);

    GolgiNetLinux(bool useEncryption);
    ~GolgiNetLinux();
    // functions required by the GolgiNetLinux

private:
    // private variables
    int sockfd;
    bool isConnected;
    bool isWaiting;
    bool isEncrypted;
    bool isSSLRequireRead;
    bool isSSLRequireWrite;
    bool isSSLWaiting;

    const SSL_METHOD *method;
    SSL_CTX *ctx;
    SSL *ssl;
    BIO *bio;

    int readBufferLength;
    int writeBufferLength;

    int readBufferPosition;
    int writeBufferPosition;

    unsigned char *readBuffer;
    unsigned char *writeBuffer;

    void ssl_connect(void);
    void ssl_connect_continue(void);

    void removeFromReadBuffer(int len);
    void removeFromWriteBuffer(int len);
    void addToReadBuffer(unsigned char *src,int len);
    void addToWriteBuffer(unsigned char * src,int len);
    
    void expandReadBuffer(int len);
    void expandWriteBuffer(int len);
};

#endif


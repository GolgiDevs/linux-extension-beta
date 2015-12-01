#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

#include "GolgiNetLinux.h"

bool GolgiNetLinux::connect(const char *url,int port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;
    char sport[6];

    if(isConnected || isWaiting || (isSSLWaiting && isEncrypted)){
        return isConnected;
    }

    if(isEncrypted){
        snprintf(sport,6,"%d",443);
    }
    else{
        snprintf(sport,6,"%d",port);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* TCP socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    s = getaddrinfo(url, sport, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
     * Try each address until we successfully bind(2).
     * If socket(2) (or bind(2)) fails, we (close the socket
     * and) try the next address. */
    for(rp = result; rp != NULL; rp = rp->ai_next) {
        
        sockfd = socket(rp->ai_family, 
                        rp->ai_socktype,
                        rp->ai_protocol);
        if (sockfd == -1)
            continue;
        
        int flags = fcntl(sockfd,F_GETFL,NULL);
        flags |= O_NONBLOCK;
        fcntl(sockfd,F_SETFL,flags);

        if(::connect(sockfd,rp->ai_addr,rp->ai_addrlen) != -1){
            if(isEncrypted){
                isSSLWaiting = true;
                ssl_connect();
            }
            else{
                isConnected = true;
            }
            break;  /* Success */
        }
        else{
            int err = errno;
            if(err == EINPROGRESS){ // this indicates that the connect could not complete immediately
                DBG.println("Connect could not complete immediately");
                isWaiting = true;
                break;
            }
        }

        close(sockfd);
    }

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not connect\n");
        isConnected = false;
    }

    freeaddrinfo(result);           /* No longer needed */

    return isConnected;
}

bool GolgiNetLinux::connected(void)
{
    return isConnected;
}

bool GolgiNetLinux::waiting(void)
{
    return isWaiting;
}

void GolgiNetLinux::stop(void)
{
    close(sockfd);
    sockfd = -1;
    isConnected = false;
}

int32_t GolgiNetLinux::available(void)
{
    return readBufferPosition;
}

int32_t GolgiNetLinux::read(void)
{
    int32_t c = (int32_t)readBuffer[0];
    removeFromReadBuffer(1);
    return c;
}

int32_t GolgiNetLinux::read(void *dst, int32_t len)
{
    int rem = available();
    
    if(rem > len){
        rem = len;
    }

    memcpy(dst,readBuffer,rem);
    removeFromReadBuffer(rem);
    return rem;
}

void GolgiNetLinux::write(const char *buf)
{
    DBG.println("Write called");
    addToWriteBuffer((unsigned char *)buf,strlen(buf));
}

void GolgiNetLinux::readAvailable(void) // called when the socket is ready for reading
{
    DBG.println("Read is available attempting to read");
    if(isSSLRequireRead){
        ssl_connect_continue();
        return;
    }
    if(readBufferLength-readBufferPosition == 0){
        expandReadBuffer(0);
    }

    int n;

    if(isEncrypted){
        n = SSL_read(ssl,
                     readBuffer + readBufferPosition,
                     readBufferLength - readBufferPosition);
    }
    else{
        n = ::read(sockfd,
                   readBuffer + readBufferPosition,
                   readBufferLength - readBufferPosition);
    }
    if(n > 0) readBufferPosition += n;
    if(n == 0) stop();
}

void GolgiNetLinux::writeAvailable(void) // called when the socket is ready for writing
{
    if(isSSLRequireWrite){
        ssl_connect_continue();
        return;
    }
    if(isWaiting){
        int so_err;
        socklen_t len;
        len = sizeof(so_err);
        int rc = getsockopt(sockfd,
                            SOL_SOCKET,
                            SO_ERROR,
                            &so_err,
                            &len);
        if(rc == 0 && so_err == 0){
            isWaiting = false;
            if(isEncrypted){
                ssl_connect();
                return;
            }
            else{
                isConnected = true;
            }
        }
        else{
            DBG.print("Error on connection: ");
            DBG.println(strerror(so_err));
            isWaiting = false;
            return;
        }
    }
    DBG.println("Write is available attempting to write");
    
    int n;

    if(isEncrypted){
        n = SSL_write(ssl,
                      writeBuffer,
                      writeBufferPosition);
    }
    else{
        n = ::write(sockfd,
                    writeBuffer,
                    writeBufferPosition);
    }

    removeFromWriteBuffer(n);
}

bool GolgiNetLinux::writePending(void)
{
    if(writeBufferPosition > 0) return true;
    return false;
}

bool GolgiNetLinux::SSLRequireRead(void)
{
    return isSSLRequireRead;
}

bool GolgiNetLinux::SSLRequireWrite(void)
{
    return isSSLRequireWrite;
}

int GolgiNetLinux::getSockfd(void)
{
    return sockfd;
}

int verify_callback(int preverify, X509_STORE_CTX *x509_ctx)
{
    if(preverify == 0){
        DBG.println("Verification failed - this is likely an issue with SSL_CTX_load_verify_locations call in GolgiNetLinux.cpp - please check");
        DBG.println("Exiting ... ");
        exit(0);
    }

    return preverify;
}

GolgiNetLinux::GolgiNetLinux(bool useEncryption)
{
    sockfd = -1;
    isConnected = false;
    isWaiting = false;
    isEncrypted = useEncryption;
    isSSLRequireRead = false;
    isSSLRequireWrite = false;
    isSSLWaiting = false;

    // initialise the buffer lengths and positions
    readBufferLength = 1024;
    writeBufferLength = 1024;
    readBufferPosition = 0;
    writeBufferPosition = 0;

    // initialise the read and write buffers
    readBuffer = new unsigned char[readBufferLength];
    writeBuffer = new unsigned char[writeBufferLength];
    bzero(readBuffer,readBufferLength);
    bzero(writeBuffer,readBufferLength);

    if(isEncrypted){
        method = SSLv23_client_method();
        if((ctx = SSL_CTX_new(method)) == NULL){
            DBG.println("Failed to create an SSL context");
            abort();
        }
        SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2|SSL_OP_NO_SSLv3);
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);
        SSL_CTX_set_verify_depth(ctx, 4);
        int err;
        if(SSL_CTX_load_verify_locations(ctx,(access("/etc/ssl/certs/ca-bundle.crt",F_OK) != -1)?"/etc/ssl/certs/ca-bundle.crt":NULL,"/etc/ssl/certs/") != 1){
            err = errno;
            DBG.print("Failed to load cert verification files: ");
            DBG.println(strerror(err));
            DBG.println("Exiting the program - you can edit the SSL_CTX_load_verify_locations(SSL_CTX *ctx,const char *ca_file, const char *ca_path) call in GolgiNetLinux.cpp for your system");
            exit(-1);
        }
    }
}

GolgiNetLinux::~GolgiNetLinux()
{
    DBG.println("GolgiNetLinux desctructor called");
    delete[] readBuffer;
    delete[] writeBuffer;
}

void GolgiNetLinux::ssl_connect(void)
{
    DBG.println("Calling ssl_connect");
    bio = BIO_new_fd(sockfd,BIO_NOCLOSE);
    ssl = SSL_new(ctx);
    SSL_set_bio(ssl,bio,bio);
    ssl_connect_continue();
}

void GolgiNetLinux::ssl_connect_continue(void)
{   
    int rc;
    int ssl_error;

    DBG.println("Calling ssl_connect_continue");
    // reset the require read/write as we may have multiple calls to 
    // ssl_connect_continue
    isSSLRequireRead = false;
    isSSLRequireWrite = false;

    rc = SSL_connect(ssl);

    if(rc == -1){
        ssl_error = SSL_get_error(ssl,rc);
        if(ssl_error == SSL_ERROR_WANT_READ){
            isSSLRequireRead = true;
        }
        else if(ssl_error == SSL_ERROR_WANT_WRITE){
            isSSLRequireWrite = true;
        }
        else{
            DBG.print("An error occurred with ssl_connect: ");
            DBG.println(strerror(ssl_error));
        }
    }
    else if(rc == 1){
        isConnected = true;
        isSSLWaiting = false;
        DBG.println("SSL Connection complete");
    }
    else{
        DBG.println("Unhandled return value from SSL_connect");
    }
}

void GolgiNetLinux::removeFromReadBuffer(int len)
{
    // remove the item from the buffer
    DBG.println("Remove from read buffer");
    unsigned char *tmp = new unsigned char[readBufferLength];
    bzero(tmp,readBufferLength);
    memcpy(tmp,readBuffer+len,readBufferPosition-len);
    readBufferPosition -= len;
    delete[] readBuffer;
    readBuffer = tmp;
}

void GolgiNetLinux::removeFromWriteBuffer(int len)
{
    DBG.println("Remove from write buffer");
    // remove the item from the buffer
    unsigned char *tmp = new unsigned char[writeBufferLength];
    bzero(tmp,writeBufferLength);
    memcpy(tmp,writeBuffer+len,writeBufferPosition-len);
    writeBufferPosition -= len;
    delete[] writeBuffer;
    writeBuffer = tmp;
}

void GolgiNetLinux::addToReadBuffer(unsigned char *src, int len)
{
    // check that there is sufficient space in the readBuffer
    if(len > (readBufferLength - readBufferPosition)){
        expandReadBuffer(len - readBufferLength + readBufferPosition);
    }

    // add the new data to the read buffer
    memcpy(readBuffer+readBufferPosition,src,len);
    readBufferPosition+=len;
}

void GolgiNetLinux::addToWriteBuffer(unsigned char *src, int len)
{
    // check that there is sufficient space in the writeBuffer
    if(len > (writeBufferLength - writeBufferPosition)){
        expandWriteBuffer(len - writeBufferLength + writeBufferPosition);
    }

    // add the new data to the write buffer
    memcpy(writeBuffer+writeBufferPosition,src,len);
    writeBufferPosition+=len;
}

void GolgiNetLinux::expandReadBuffer(int len)
{
    DBG.println("Expanding read buffer");
    if(len > readBufferLength){
        readBufferLength += len;
    }
    else{
        readBufferLength *= 2;
    }
    unsigned char *tmp = new unsigned char[readBufferLength];
    bzero(tmp,readBufferLength);
    memcpy(tmp,readBuffer,readBufferPosition);
    delete[] readBuffer;
    readBuffer = tmp;
}

void GolgiNetLinux::expandWriteBuffer(int len)
{
    DBG.println("Expanding write buffer");
    if(len > writeBufferLength){
        writeBufferLength += len;
    }
    else{
        writeBufferLength *= 2;
    }
    writeBufferLength *= 2;
    unsigned char *tmp = new unsigned char[writeBufferLength];
    bzero(tmp,writeBufferLength);
    memcpy(tmp,writeBuffer,writeBufferPosition);
    delete[] writeBuffer;
    writeBuffer = tmp;
}



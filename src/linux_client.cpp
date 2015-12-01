#include "libgolgi.h"
#include "GolgiWrapper.h"

int main(int argc,
         char **argv)
{
    fd_set readFds;
    fd_set writeFds;
    fd_set exceptFds;
    struct timeval tv;
    int nfds = 1;

    FD_ZERO(&readFds);
    FD_ZERO(&writeFds);
    FD_ZERO(&exceptFds);

    DBG.println("Setting up as alice");

    // Call GolgiSetup to create a connection to Golgi servers and register 
    // with an instance id. The function call takes two arguments. The 
    // instance id you wish to register with as a character array - note this 
    // should be unique across your application/API. And a boolean indicating 
    // if you wish to use SSL/TLS encryption or not - pass true to use 
    // encryption and false to not.
    //
    // GolgiSetup(char *instance_id,
    //            bool useSSLEncrypton);
    GolgiSetup("alice",
               true);

    // calls to register request receivers should happen here. An example of 
    // registering a request receiver for an API with API Domain "hilo" and 
    // method name "setLed" is shown below
    //
    // (new HiloSetLedHandler())->registerReceiver();
    //


    // A while loop calling GolgiSelect which wraps a system select call. You 
    // should use the GolgiSelect loop exactly how you would use a system 
    // select call. The GolgiSelect wrapping handles all of the socket 
    // interactions required to enable the Golgi service.
    while(1){
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        GolgiSelect(nfds,
                    &readFds,
                    &writeFds,
                    &exceptFds,
                    &tv);
    }
    return 0;
}

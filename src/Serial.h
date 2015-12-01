#ifndef __GOLGI_SERIAL_CLASS_H__
#define __GOLGI_SERIAL_CLASS_H__
#include <stdio.h>
#include "String.h"

class SerialClass{
public:
    SerialClass();
    void println(String str);
    void print(String str);
    void println(const char *c);
    void print(const char *c);
    void println(int d);
    void print(int d);

private:
    FILE *outfd;
};

extern SerialClass Serial;

#endif


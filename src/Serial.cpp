#include <stdio.h>

#include "Serial.h"

SerialClass::SerialClass()
{
    outfd = stdout;
}

void SerialClass::print(String str)
{
    fprintf(outfd,
            "%s",
            str.c_str());
}

void SerialClass::println(String str)
{
    fprintf(outfd,
            "%s\n",
            str.c_str());
}

void SerialClass::println(const char *ch)
{
    fprintf(outfd,
            "%s\n",
            ch);
}

void SerialClass::print(const char *ch)
{
    fprintf(outfd,
            "%s",
            ch);
}

void SerialClass::println(int d)
{
    fprintf(outfd,
            "%d\n",
            d);
}

void SerialClass::print(int d)
{
    fprintf(outfd,
            "%d",
            d);
}

SerialClass Serial;


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "String.h"

String::~String()
{
    delete[] str;
}

String::String()
{
    max_string_length = 1024;
    str = new char[max_string_length];
    bzero(str,max_string_length);
    len = 0;
}

String::String(int i)
{
    max_string_length = 1024;
    str = new char[max_string_length];
    sprintf(str,"%d",i);
    len = strlen(str);
}

String::String(int32_t d,int i)
{
    max_string_length = 1024;
    str = new char[max_string_length];
    sprintf(str,"0x%x",d);
    len = strlen(str);
}

String::String(const char *c)
{
    max_string_length = 1024;
    len = strlen(c);
    if(len > max_string_length) max_string_length = len;
    str = new char[max_string_length+1];
    bzero(str,max_string_length+1);
    memcpy(str,c,strlen(c));
}

const char *String::c_str(void) const
{
    return (const char *)str;
}

int String::length(void) const
{
    return len;
}



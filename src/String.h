#ifndef __GOLGI_STRING_H__
#define __GOLGI_STRING_H__

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define HEX 1

class String {
public:
    ~String();
    String();
    String(int i);
    String(int32_t d,int i);
    String(const char *c);
    const char *c_str(void) const;
    int length(void) const;
    
    String operator+(const String& s)
    {
        int tlen = this->length() + s.length();
        char *tmp;
        tmp = (char *)malloc(sizeof(char)*(tlen+1));
        bzero(tmp,tlen+1);
        memcpy(tmp,this->c_str(),this->length());
        memcpy(tmp+this->length(),s.c_str(),s.length());

        String string(tmp);
        free(tmp);
        return string;
    }

    String operator+(const char *s)
    {
        int tlen = this->length() + strlen(s);
        char *tmp;
        tmp = (char *)malloc(sizeof(char)*(tlen+1));
        bzero(tmp,tlen+1);
        memcpy(tmp,this->c_str(),this->length());
        memcpy(tmp+this->length(),s,strlen(s));

        String string(tmp);
        free(tmp);
        return string;
    }


private:
    char *str;
    int len;
    int max_string_length;
};

#endif



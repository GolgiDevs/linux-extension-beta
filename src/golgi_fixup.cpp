#include <time.h>
#include <math.h>
#include <stdlib.h>

#include "golgi_fixup.h"

long millis(void)
{
    long            ms; // Milliseconds
    time_t          s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds

    return s*1000 + ms;
}

char random(char s, char e)
{
    return s + rand() % (e - s);
}


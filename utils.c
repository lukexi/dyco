#include "utils.h"

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>

void Fatal(const char *format, ...)
{
    va_list ap;

    fprintf(stderr, "ERROR: ");

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    exit(1);
}

void Graph(char* sym, int N) {
    for (int i = 0; i < N; ++i) {
        printf("%s", sym);
    }
    printf("\n");
}

float GetTime(void)
{
    static struct timeval InitialTime = {0, 0};
    if (InitialTime.tv_sec == 0)
    {
        gettimeofday(&InitialTime, NULL);
    }
    struct timeval Now;
    gettimeofday(&Now, NULL);

    struct timeval TimeSinceStart = { Now.tv_sec - InitialTime.tv_sec, Now.tv_usec - InitialTime.tv_usec };

    return (TimeSinceStart.tv_sec + (TimeSinceStart.tv_usec / 1000000.0));
}

fps MakeFPS(char* Name) {
    struct timeval Now;
    gettimeofday(&Now, NULL);
    return (fps){
        .Frames = 0,
        .CurrentSecond = (int)Now.tv_sec,
        .Name = Name
    };
}

void TickFPS(fps* FPS) {
    struct timeval Now;
    gettimeofday(&Now, NULL);

    FPS->Frames++;

    int NowSecond = Now.tv_sec;
    if (NowSecond > FPS->CurrentSecond) {
        printf("%40s: %i FPS\n", FPS->Name, FPS->Frames);
        FPS->Frames = 0;
        FPS->CurrentSecond = NowSecond;
    }
}

int NextPowerOfTwo(int x) {
    x--;
    x |= x >> 1; // handle 2 bit numbers
    x |= x >> 2; // handle 4 bit numbers
    x |= x >> 4; // handle 8 bit numbers
    x |= x >> 8; // handle 16 bit numbers
    x |= x >> 16; // handle 32 bit numbers
    x++;
    return x;
}

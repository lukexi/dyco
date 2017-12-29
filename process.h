#ifndef PROCESS_H
#define PROCESS_H
#include <stdlib.h>

int CallProcess(char* const* Args, const char* StdIn,
    char* OutBuffer, size_t OutBufferSize, size_t* OutLength,
    char* ErrBuffer, size_t ErrBufferSize, size_t* ErrLength);

#endif // PROCESS_H

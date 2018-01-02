#ifndef COMPILE_H
#define COMPILE_H
#include <stdlib.h>
#include <stdbool.h>

bool IsCFileName(char* Source);

int CompileSource(
    char* FileName,
    char* Source,
    char* ErrBuffer, size_t ErrBufferSize, size_t* ErrLength);

#endif // COMPILE_H

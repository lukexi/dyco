#ifndef COMPILE_H
#define COMPILE_H
#include <stdlib.h>
#include <stdbool.h>

bool IsCFileName(const char* Source);

int CompileSource(
    const char* FileName,
    const char* Source,
    char* ErrBuffer, size_t ErrBufferSize, size_t* ErrLength);

#endif // COMPILE_H

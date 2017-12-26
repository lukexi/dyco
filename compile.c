#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "process.h"

#define MACOS_DYLIB_FLAGS "-dynamiclib", "-undefined", "dynamic_lookup",
#define LINUX_DYLIB_FLAGS "-fPIC", "-shared",

#define DEBUG_FLAGS "-fno-omit-frame-pointer",  "-g",
#define PERF_FLAGS "-O3",

#define COMMON_FLAGS MACOS_DYLIB_FLAGS DEBUG_FLAGS PERF_FLAGS

bool IsCFileName(const char* Source) {
    const int Len = strlen(Source);
    return Source[Len-2] == '.' && Source[Len-1] == 'c';
}

int CompileSource(
    const char* FileName,
    const char* Source,
    char* ErrBuffer, size_t ErrBufferSize, size_t* ErrLength)
{

    const char* StdInArgs[] = {
        "clang",
        "-o", FileName,
        COMMON_FLAGS
        "-x", "c", "-", // treat as C file, read from stdin
        NULL
    };
    const char* FileArgs[] = {
        "clang",
        "-o", FileName,
        COMMON_FLAGS
        Source,
        NULL
    };
    const char** Args = IsCFileName(Source) ? FileArgs : StdInArgs;

    char OutBuffer[1024]; size_t OutLength;

    int ExitCode = CallProcess(Args, Source,
        OutBuffer, sizeof(OutBuffer), &OutLength,
        ErrBuffer, ErrBufferSize, ErrLength);

    if (OutLength) printf("STDOUT: %s\n", OutBuffer);

    return ExitCode;
}

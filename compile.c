#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "process.h"

#define MACOS_DYLIB_FLAGS "-dynamiclib", "-undefined", "dynamic_lookup",
#define LINUX_DYLIB_FLAGS "-fPIC", "-shared",

int CompileSource(
    const char* FileName,
    const char* Source,
    char* ErrBuffer, size_t ErrBufferSize, size_t* ErrLength)
{
    char OutBuffer[1024];
    size_t OutLength;

    // char FileNameSO[256];
    // snprintf(FileNameSO, sizeof(FileNameSO), "%s.so", FileName);
    const char* Args[] = {"clang",
        "-o", FileName,
        "-O3",
        MACOS_DYLIB_FLAGS
        // "-fno-omit-frame-pointer",  "-g", // Debug support
        "-x", "c", // treat as C file
        "-",       // read from stdin
        NULL};
    int ExitCode = CallProcess(Args, Source,
        OutBuffer, sizeof(OutBuffer), &OutLength,
        ErrBuffer, ErrBufferSize, ErrLength);
    if (OutLength) printf("STDOUT: %s\n", OutBuffer);
    // if (ErrLength) printf("STDERR: %s\n", ErrBuffer);
    return ExitCode;
}

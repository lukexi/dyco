#include <dlfcn.h>
#include <stdio.h>
#include "compile.h"

// FIXME: shouldn't wrap this up so tightly, as we'd like to be
// able to return the library to get multiple symbols from it
// along with freeing it

void* DynamicFunction(const char* FunctionName, const char* FunctionSource)
{
    char FileNameSO[256];
    snprintf(FileNameSO, sizeof(FileNameSO), "%s.so", FunctionName);

    char ErrBuffer[1024];
    size_t ErrLength;
    int ExitCode = CompileSource(
        FileNameSO, FunctionSource,
        ErrBuffer, sizeof(ErrBuffer), &ErrLength);
    if (ErrLength) printf("STDERR: %s\n", ErrBuffer);
    if (ExitCode != 0) {
        printf("Compilation failed with exit code: %i", ExitCode);
        return NULL;
    }

    void* LibraryHandle  = dlopen(FileNameSO, RTLD_LAZY);
    void* FunctionHandle = dlsym(LibraryHandle, "Foo");

    return FunctionHandle;
}

#ifndef DYNAMIC_H
#define DYNAMIC_H

#include <sys/stat.h>
#include <stdbool.h>
#include <dlfcn.h>

void* DynamicFunction(
    const char* FunctionName,
    const char* FunctionSource);

typedef struct library library;

typedef void (*loader_func)(library* Library, void* UserData);

struct library {
    void*  LibHandle;
    time_t LastModTime;
    char*  SourceFilename;
    char   CompilationLog[2048];
    size_t CompilationLogLength;

    loader_func LoaderFunc;
    void* LoaderUserData;
};

// Provide an optional callback to call when the library is updated
library* CreateLibrary(char* SourceFilename, loader_func LoaderFunc, void* LoaderUserData);

void* GetLibrarySymbol(library* Library, char* SymbolName);

// Returns whether the library was updated.
// (if you use this instead of the callback, reload symbols when this is true)
bool UpdateLibrary(library* Library);

void FreeLibrary(library* Library);

#endif // DYNAMIC_H

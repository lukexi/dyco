#ifndef DYNAMIC_H
#define DYNAMIC_H

#include <sys/stat.h>
#include <stdbool.h>
#include <dlfcn.h>

typedef struct library library;

typedef void (*loader_func)(library* Library, void* UserData);

struct library {
    void*  LibHandle;
    time_t LastModTime;
    char*  Name;
    char*  Source;
    char   CompilationLog[2048];
    size_t CompilationLogLength;
    bool   SourceUpdated;

    loader_func LoaderFunc;
    void* LoaderUserData;
};

// Creates a library from a source file or source code string.
// Provide an optional callback to call when the library is updated
library* CreateLibrary(char* Name, char* Source, loader_func LoaderFunc, void* LoaderUserData);

void* GetLibrarySymbol(library* Library, char* SymbolName);

// Set a new source for the library and compile it.
// (This can be a file or a source code string)
void UpdateLibrarySource(library* Library, char* Source);

// Check if the library needs to be updated from a file.
// Returns whether the library was updated.
// (if you use this instead of the callback, reload symbols when this is true)
bool UpdateLibraryFile(library* Library);

void FreeLibrary(library* Library);

// Shortcut to grabbing a single function from source.
// Uses the function name as the library name,
// so you can call it multiple times
// to update a function from new source.
void* DynamicFunction(
    char* FunctionName,
    char* FunctionSource);

#endif // DYNAMIC_H

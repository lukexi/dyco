#ifndef DYNAMIC_H
#define DYNAMIC_H

#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <dlfcn.h>

typedef struct library library;

typedef void (*loader_func)(library* Library, void* UserData);

struct library {
    void*  LibHandle;
    time_t LastModTime;
    char*  Name;
    char*  Source;
    char   CompilationLog[16384];
    size_t CompilationLogLength;
    bool   SourceUpdated;
    bool   LibraryNeedsReload;
    bool   LibraryCompiledSuccessfully;

    char*  Dependencies[1024];
    time_t DependenciesModTimes[1024];
    size_t DependenciesLength;

    loader_func LoaderFunc;
    void* LoaderUserData;
};

// Creates a library from a source file or source code string.
// Provide an optional callback to call when the library is updated
library* CreateLibrary(char* Name, char* Source, loader_func LoaderFunc, void* LoaderUserData);

// Get a symbol from the library that will be
// valid until the next time the library is reloaded.
void* GetLibrarySymbol(library* Library, char* SymbolName);

// Set a new source for the library and compile it.
// (This can be a file or a source code string)
void UpdateLibrarySource(library* Library, char* Source);

// Check if the library needs to be updated from a file.
// Returns whether the library was updated.
// (if you use this instead of the callback, reload symbols when this is true)
bool UpdateLibraryFile(library* Library);

// Unloads the library. All symbols will become invalid.
void FreeLibrary(library* Library);

// Allow separate recompilation and reloading,
// when you'd like them to happen on separate threads, for example.
// (these are alternative to the "Update" functions,
// which recompile and reload in one call.)
// You can call these as quickly as you want, since they won't
// do anything until necessary, and will return true when something
// was done.
bool RecompileLibrary(library* Library);
bool ReloadLibrary(library* Library);

// Shortcut to grabbing a single function from source.
// Uses the function name as the library name,
// so you can call it multiple times
// to update a function from new source.
void* DynamicFunction(
    char* FunctionName,
    char* FunctionSource);

time_t GetFileModTime(char* FileName);

#endif // DYNAMIC_H

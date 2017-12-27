#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "compile.h"
#include "dynamic.h"

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

    if (ErrLength) printf("Compilation error: %s\n", ErrBuffer);

    if (ExitCode != 0) {
        printf("Compilation failed with exit code: %i", ExitCode);
        return NULL;
    }

    void* LibraryHandle  = dlopen(FileNameSO, RTLD_LAZY);
    void* FunctionHandle = dlsym(LibraryHandle, FunctionName);

    return FunctionHandle;
}


library* CreateLibrary(char* SourceFilename, loader_func LoaderFunc, void* LoaderUserData) {
    library* Library = calloc(1, sizeof(library));
    Library->SourceFilename = strdup(SourceFilename);
    Library->LoaderFunc     = LoaderFunc;
    Library->LoaderUserData = LoaderUserData;
    UpdateLibrary(Library);
    return Library;
}

void FreeLibrary(library* Library) {
    if (Library->LibHandle) {
        dlclose(Library->LibHandle);
    }
    free(Library->SourceFilename);
    free(Library);
}

void* GetLibrarySymbol(library* Library, char* SymbolName) {
    return dlsym(Library->LibHandle, SymbolName);
}

bool UpdateLibrary(library* Library) {
    if (!Library) return false;

    // Verify the source file name is a c file name,
    // since we CompileSource also supports source code.
    // FIXME figure out how/if we want to handle that.
    if (IsCFileName(Library->SourceFilename)) {
        struct stat attr = { 0 };
        stat(Library->SourceFilename, &attr);
        time_t NewModTime = attr.st_mtime;
        if (NewModTime == Library->LastModTime) return false;

        Library->LastModTime = NewModTime;
    }

    // Get a name for the library derived from the source
    char LibraryFilename[256];
    snprintf(LibraryFilename,
        sizeof(LibraryFilename),
        "%s.so", Library->SourceFilename);

    // Clear the compilation log
    memset(Library->CompilationLog, 0, sizeof(Library->CompilationLog));

    // Compile the source into a library
    int ExitCode = CompileSource(
        LibraryFilename, Library->SourceFilename,
        Library->CompilationLog, sizeof(Library->CompilationLog),
        &Library->CompilationLogLength);

    if (Library->CompilationLogLength) {
        printf("Compilation log: %s\n", Library->CompilationLog);
    }

    if (ExitCode != 0) {
        printf("Compilation failed with exit code: %i", ExitCode);
        return false;
    }

    // Open the new library
    if (Library->LibHandle) {
        int Result = dlclose(Library->LibHandle);
        if (Result) printf("dlclose error: %i\n", Result);
        Library->LibHandle = NULL;
    }

    void* NewLibraryHandle = dlopen(LibraryFilename, RTLD_LAZY);

    if (NewLibraryHandle) {
        Library->LibHandle = NewLibraryHandle;

        if (Library->LoaderFunc) {
            Library->LoaderFunc(Library, Library->LoaderUserData);
        }

        return true;
    }
    return true;
}

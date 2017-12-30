#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "compile.h"
#include "dynamic.h"

library* CreateLibrary(char* Name, char* Source, loader_func LoaderFunc, void* LoaderUserData) {
    library* Library = calloc(1, sizeof(library));
    Library->Name           = strdup(Name);
    Library->Source         = strdup(Source);
    Library->LoaderFunc     = LoaderFunc;
    Library->LoaderUserData = LoaderUserData;
    Library->SourceUpdated  = true;
    UpdateLibraryFile(Library);
    return Library;
}

void FreeLibrary(library* Library) {
    if (!Library) return;
    if (Library->LibHandle) {
        dlclose(Library->LibHandle);
    }
    free(Library->Name);
    free(Library->Source);
    free(Library);
}

void* GetLibrarySymbol(library* Library, char* SymbolName) {
    if (!Library) return NULL;
    return dlsym(Library->LibHandle, SymbolName);
}

void UpdateLibrarySource(library* Library, char* Source) {
    if (!Library) return;
    free(Library->Source);
    Library->Source = strdup(Source);
    Library->SourceUpdated = true;
    UpdateLibraryFile(Library);
}

bool ReloadLibrary(library* Library) {
    if (!Library) return false;
    if (!Library->LibraryNeedsReload) return false;

    char LibraryFilename[256];
    snprintf(LibraryFilename, sizeof(LibraryFilename), "%s.so", Library->Name);

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
    }
    Library->LibraryNeedsReload = false;

    return true;
}

bool UpdateLibraryFile(library* Library) {
    if (!Library) return false;

    if (RecompileLibrary(Library)) {
        ReloadLibrary(Library);
        return true;
    }

    return false;
}

time_t GetFileModTime(char* FileName) {
    struct stat attr = { 0 };
    stat(FileName, &attr);
    return attr.st_mtime;
}

bool RecompileLibrary(library* Library) {
    if (!Library) return false;

    // If the source is a C file, check if it's been updated.
    if (IsCFileName(Library->Source)) {
        time_t NewModTime = GetFileModTime(Library->Source);
        if (NewModTime == Library->LastModTime) return false;

        Library->LastModTime = NewModTime;
    } else if (!Library->SourceUpdated) {
        return false;
    }
    Library->SourceUpdated = false;

    // Append .so to the library name
    char LibraryFilename[256];
    snprintf(LibraryFilename,
        sizeof(LibraryFilename),
        "%s.so", Library->Name);

    // Clear the compilation log
    memset(Library->CompilationLog, 0, sizeof(Library->CompilationLog));

    // Compile the source into a library
    int ExitCode = CompileSource(
        LibraryFilename, Library->Source,
        Library->CompilationLog, sizeof(Library->CompilationLog),
        &Library->CompilationLogLength);

    if (Library->CompilationLogLength) {
        printf("Compilation log: %s\n", Library->CompilationLog);
    }

    if (ExitCode != 0) {
        printf("Compilation failed with exit code: %i\n", ExitCode);
        return false;
    }

    Library->LibraryNeedsReload = true;

    return true;
}


// Shortcut to grabbing a single function.
void* DynamicFunction(char* FunctionName, char* FunctionSource) {

    char FileNameSO[256];
    snprintf(FileNameSO, sizeof(FileNameSO), "%s.so", FunctionName);

    // Check if the library is already open, and close it if so
    void* LibraryHandle = dlopen(FileNameSO, RTLD_NOLOAD);
    if (LibraryHandle) {
        // Must call dlclose twice, since dlopen with RTLD_NOLOAD
        // still increments the reference count
        int Result = dlclose(LibraryHandle);
        if (Result) printf("dlclose error: %i\n", Result);
        Result = dlclose(LibraryHandle);
        if (Result) printf("dlclose error: %i\n", Result);
    }

    library* Library = CreateLibrary(FunctionName, FunctionSource,
        NULL, NULL);

    void* FunctionHandle = GetLibrarySymbol(Library, FunctionName);

    // Prevent FreeLibrary from freeing the lib.
    Library->LibHandle = NULL;
    FreeLibrary(Library);

    return FunctionHandle;
}

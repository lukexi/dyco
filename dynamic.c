#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "compile.h"
#include "process.h"
#include "dynamic.h"

library* CreateLibrary(
    char* Name,
    char* Source,
    loader_func LoaderFunc,
    void* LoaderUserData)
{
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

    Library->LibraryNeedsReload = false;

    // Close the old library
    char LibraryFilename[256];
    snprintf(LibraryFilename, sizeof(LibraryFilename), "%s.so", Library->Name);

    if (Library->LibHandle) {
        int Result = dlclose(Library->LibHandle);
        if (Result) printf("dlclose error: %i\n", Result);
        Library->LibHandle = NULL;
    }

    // Open the new library
    void* NewLibraryHandle = NULL;

    if (Library->LibraryCompiledSuccessfully) {
        NewLibraryHandle = dlopen(LibraryFilename, RTLD_LAZY | RTLD_GLOBAL);
    }

    Library->LibHandle = NewLibraryHandle;

    if (Library->LoaderFunc) {
        Library->LoaderFunc(Library, Library->LoaderUserData);
    }


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


void UpdateDependencies(library* Library)
{
    char* StdInArgs[] = {
        "clang",
        "-MM",
        "-x", "c", "-", // treat as C file, read from stdin
        NULL
    };
    char* FileArgs[] = {
        "clang",
        "-MM",
        Library->Source,
        NULL
    };
    bool SourceIsFile = IsCFileName(Library->Source);
    char** Args = SourceIsFile ? FileArgs : StdInArgs;
    char* StdIn = SourceIsFile ? NULL : Library->Source;

    // NOTE: this is modified below by strtok_r
    char OutBuffer[16384]; size_t OutBufferSize = sizeof(OutBuffer); size_t OutLength;
    char ErrBuffer[16384]; size_t ErrBufferSize = sizeof(ErrBuffer); size_t ErrLength;

    int ExitCode = CallProcess(Args, StdIn,
        OutBuffer, OutBufferSize, &OutLength,
        ErrBuffer, ErrBufferSize, &ErrLength);

    for (int I = 0; I < Library->DependenciesLength; I++) {
        free(Library->Dependencies[I]);
    }
    Library->DependenciesLength = 0;

    if (ErrLength) {
        printf("Error %i getting dependencies: %s\n", ExitCode, ErrBuffer);
        return;
    }
    if (OutLength) {

        const char* Delimiters = ": \n\\";
        char* Remaining = OutBuffer;
        char* Token;

        // Discard the first item (it is the .o filename)
        Token = strtok_r(Remaining, Delimiters, &Remaining);
        while ((Token = strtok_r(Remaining, Delimiters, &Remaining))) {
            // Ignore empty strings and absolute paths,
            // which we assume to be system headers that will not change.
            // TODO: reevaluate this; maybe we want to check for e.g. /usr
            // rather than ignoring all absolute paths?
            if (strlen(Token) > 0 && Token[0] != '/') {
                // printf("Found token: %s\n", Token);

                int DepIndex   = Library->DependenciesLength;
                char* FileName = strdup(Token);
                Library->Dependencies[DepIndex]         = FileName;
                Library->DependenciesModTimes[DepIndex] = GetFileModTime(FileName);
                Library->DependenciesLength++;
            }
        }
    }
}

bool CheckDependenciesChanged(library* Library) {
    // printf("Checking %zu files\n", Library->DependenciesLength);
    for (int I = 0; I < Library->DependenciesLength; I++) {
        time_t NewDepModTime = GetFileModTime(Library->Dependencies[I]);
        if (NewDepModTime > Library->DependenciesModTimes[I]) {
            printf("%s changed.\n", Library->Dependencies[I]);
            // printf("%s updated at %li, %s updated at %li\n",
            //     Library->Dependencies[I], DepModTime,
            //     Library->Name, Library->LastModTime);
            return true;
        }
    }
    return false;
}

bool RecompileLibrary(library* Library) {
    if (!Library) return false;

    if (CheckDependenciesChanged(Library)) {
        // If yes, continue to recompilation.
        // If no, continue checking conditions.
    } else if (IsCFileName(Library->Source)) {
        // If the source is a C file, check if it's been updated.
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

    // Save the header dependencies of the source code
    // TODO
    // UpdateDependencies(Library);

    // Compile the source into a library
    int ExitCode = CompileSource(
        LibraryFilename, Library->Source,
        Library->CompilationLog, sizeof(Library->CompilationLog),
        &Library->CompilationLogLength);

    if (Library->CompilationLogLength) {
        printf("Compilation log: %s\n", Library->CompilationLog);
    }

    Library->LibraryNeedsReload = true;
    Library->LibraryCompiledSuccessfully = ExitCode == 0;

    if (ExitCode != 0) {
        printf("Compilation failed with exit code: %i\n", ExitCode);
        return true;
    }

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

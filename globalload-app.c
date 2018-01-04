#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "dynamic.h"
#include "globalload-testfile.h"

// int (*Frob) (void);

// void FrobLoader(library* Library, void* UserData) {
//     void* FunctionHandle = GetLibrarySymbol(Library, "Frob");

//     if (FunctionHandle) {
//         Frob = FunctionHandle;
//     } else {
//         printf("Couldn't find symbol 'Frob' :(\n");
//     }
// }

int main() {

    library* TestLib = CreateLibrary("globalload-testfile", "globalload-testfile.c",
        // FrobLoader, NULL);
        NULL, NULL);

    int I = 1000;
    while (I--) {
        usleep(500000);

        if (RecompileLibrary(TestLib)) {
            // void* LibraryHandle = dlopen("globalload-testfile.so", RTLD_NOLOAD);
            // if (LibraryHandle) {
            //     printf("Closing\n");
            //     // Must call dlclose twice, since dlopen with RTLD_NOLOAD
            //     // still increments the reference count
            //     int Result = dlclose(LibraryHandle);
            //     if (Result) printf("dlclose error: %i\n", Result);
            //     Result = dlclose(LibraryHandle);
            //     if (Result) printf("dlclose error: %i\n", Result);
            //     printf("Hellaclosed\n");
            // }
            ReloadLibrary(TestLib);
        }

        Frob();
    }

    return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "dynamic.h"

int (*Foo) (void);

void FooLoader(library* Library, void* UserData) {
    void* FunctionHandle = GetLibrarySymbol(Library, "Foo");

    if (FunctionHandle) {
        Foo = FunctionHandle;
    } else {
        printf("Couldn't find symbol 'Foo' :(\n");
    }
}

int main() {

    library* TestLib = CreateLibrary("testfile", "testfile.c", FooLoader, NULL);

    int I = 1000;
    while (I--) {
        usleep(500000);
        UpdateLibraryFile(TestLib);
        Foo();
    }

    return 0;
}

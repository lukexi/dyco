#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "dynamic.h"

int (*Foo) (void);

void FooLoader(library* Library, void* UserData) {
    Foo = GetLibrarySymbol(Library, "Foo");
    if (Foo == NULL) {
        printf("Couldn't find symbol 'Foo' :(\n");
    }
}

int main() {

    library* TestLib = CreateLibrary("file-testfile", "file-testfile.c", FooLoader, NULL);

    int I = 1000;
    while (I--) {
        usleep(500000);
        UpdateLibraryFile(TestLib);
        if (Foo) Foo();
    }

    return 0;
}

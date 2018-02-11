#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "dynamic.h"

int (*Foo) (void);

int main() {

    library* TestLib = CreateLibrary("file-testfile", "file-testfile.c");

    int I = 1000;
    while (I--) {
        usleep(500000);
        if (UpdateLibraryFile(TestLib)) {
            Foo = GetLibrarySymbol(TestLib, "Foo");
            if (Foo == NULL) {
                printf("Couldn't find symbol 'Foo' :(\n");
            }
        }
        if (Foo) Foo();
    }

    return 0;
}

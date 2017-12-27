#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "dynamic.h"

int (*Foo) (void);

void FooLoader(library* Library, void* UserData) {
    printf("Executing FooLoader. %p\n", Library->LibHandle);
    void* FunctionHandle = dlsym(Library->LibHandle, "Foo");

    if (FunctionHandle) {
        Foo = FunctionHandle;
    } else {
        printf("Couldn't find symbol 'Foo' :(\n");
    }
}

int main() {

    library* TestLib = CreateLibrary("testfile.c", FooLoader, NULL);

    int I = 1000;
    while (I--) {
        usleep(500000);
        UpdateLibrary(TestLib);
        Foo();
    }

    return 0;
}

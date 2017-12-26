#include "compile.h"
#include <stdio.h>
#include <dlfcn.h>

const char* Program =
"#include <stdio.h>\n"
"int Foo(void) {\n"
"    printf(\"Hello from Foo!\\n\");\n"
"    return 123;\n"
"}\n";

int (*Foo) (void);

int main() {
    const char* LibName = "SayHi.so";

    char ErrBuffer[1024];
    size_t ErrLength;
    int CompileExitCode =
        CompileSource(LibName, Program,
        ErrBuffer, sizeof(ErrBuffer), &ErrLength);
    if (ErrLength) printf("STDERR: %s\n", ErrBuffer);

    void* LibraryHandle = dlopen(LibName, RTLD_LAZY);
    Foo = dlsym(LibraryHandle, "Foo");
    if (!Foo) {
        printf("Failed to find foo\n");
        exit(1);
    }

    printf("Result: %i", Foo());

    return 0;
}

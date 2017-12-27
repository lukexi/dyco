#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "dynamic.h"

const char* FooSource =
"#include <stdio.h>\n"
"int Foo(void) {\n"
"    printf(\"Hello from Foo!\\n\");\n"
"    return 123;\n"
"}\n";

int (*Foo) (void);

int main() {

    int (*Foo) (void);
    Foo = DynamicFunction("Foo", FooSource);
    // Foo = DynamicFunction("Foo", "testfile.c");
    printf("Result: %i\n", Foo());

    return 0;
}

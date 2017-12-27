#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "dynamic.h"

char* FooSource =
"#include <stdio.h>\n"
"int Foo(void) {\n"
"    printf(\"Hello from Foo!\\n\");\n"
"    return 123;\n"
"}\n";

char* FooSource2 =
"#include <stdio.h>\n"
"int Foo(void) {\n"
"    printf(\"Hello from Foo 2!\\n\");\n"
"    return 321;\n"
"}\n";

int (*Foo) (void);

int main() {

    int (*Foo) (void);
    Foo = DynamicFunction("Foo", FooSource);
    // Foo = DynamicFunction("Foo", "testfile.c");
    printf("Result: %i\n", Foo());

    Foo = DynamicFunction("Foo", FooSource2);
    printf("Result: %i\n", Foo());

    return 0;
}

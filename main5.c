#include "process.h"
#include <stdio.h>

char* Program =
"#include <stdio.h>\n"
"int main(int argc, char* argv[]) {\n"
"    printf(\"Hi ...\\n\");\n"
"    return 0;\n"
"}\n";

int main() {
    char OutBuffer[1024];
    char ErrBuffer[1024];
    size_t ErrLength;
    size_t OutLength;


    const char* Args[] = {"clang", "-o", "foo.app", "-x", "c", "-", NULL};
    printf("Running %s\n", Args[0]);
    CallProcess(Args, Program,
        OutBuffer, sizeof(OutBuffer), &OutLength,
        ErrBuffer, sizeof(ErrBuffer), &ErrLength);
    if (OutLength) printf("STDOUT: %s\n", OutBuffer);
    if (ErrLength) printf("STDERR: %s\n", ErrBuffer);


    const char* Args2[] = {"./foo.app"};
    printf("Running %s\n", Args2[0]);
    CallProcess(Args2, NULL,
        OutBuffer, sizeof(OutBuffer), &OutLength,
        ErrBuffer, sizeof(ErrBuffer), &ErrLength);
    if (OutLength) printf("STDOUT: %s\n", OutBuffer);
    if (ErrLength) printf("STDERR: %s\n", ErrBuffer);


    return 0;
}

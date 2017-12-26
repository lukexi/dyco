#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

char* Program =
// "#include <stdio.h>\n"
"int main(int argc, char* argv[]) {\n"
"    printf(\"HALLOWORLD\\n\");\n"
"    return 0;\n"
"}\n";

int main(int argc, char* argv[])
{
    printf("Awaiting\n");

    FILE* Process = popen("clang -o foo.app -x c -", "r+");
    // FILE* Process = popen("clang -o foo.app -x c - 2>&1", "r+");
    // FILE* Process = popen("cat test.c | clang -o foo.app -x c - 2>&1", "r+");
    if (Process == NULL) {
        printf("Failed to open process.\n");
        exit(1);
    }

    // int FD = fileno(Process);
    // fcntl(FD, F_SETFL, O_NONBLOCK);

    fwrite(Program, strlen(Program), 1, Process);
    // fputc(EOF, Process);
    printf("Done.\n");

    // printf("Awaiting output...\n");
    // char Buffer[1024];
    // int Result = fread(Buffer, 1024, 1, Process);
    // printf("%i Buffer: %s\n", Result, Buffer);

    printf("Waiting...\n");
    pclose(Process);



    printf("Exiting.\n");
    return 0;
}

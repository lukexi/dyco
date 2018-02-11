#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "dynamic.h"
#include "globalload-testfile.h"

int main() {

    library* TestLib = CreateLibrary("globalload-testfile", "globalload-testfile.c");
    LoadAll();

    int I = 1000;
    while (I--) {
        usleep(500000);

        if (RecompileLibrary(TestLib)) {
            ReloadLibrary(TestLib);
            LoadAll();
        }

        if (Frob) Frob();
        if (Warb) Warb(1.23456);
    }

    return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "gl.h"
#include "dynamic.h"

int main(int argc, char const *argv[]) {

    library* CVRender = CreateLibrary(
        "cv-render",
        "cv-render.c", NULL, NULL);

    SDL_Window* Window = CreateWindow("CV", 10,10, 1024,768);

    void* (*TickRender)(SDL_Window* Window);
    TickRender = GetLibrarySymbol(CVRender, "TickRender");
    while (true) {

        RecompileLibrary(CVRender);

        if (CVRender->LibraryNeedsReload) {
            void (*Cleanup)(void) = GetLibrarySymbol(CVRender, "Cleanup");
            if (Cleanup) Cleanup();
            ReloadLibrary(CVRender);
            TickRender = GetLibrarySymbol(CVRender, "TickRender");
        }

        if (TickRender) TickRender(Window);
    }


    return 0;
}

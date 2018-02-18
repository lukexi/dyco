#include "gl.h"
#include "dynamic.h"
#include "nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"

int main(int argc, char const *argv[])
{
    library* RenderLib = CreateLibrary2("contours-render", "contours-render.c");

    SDL_Window* Window = CreateWindow("Contours", 10,10, 1024,768);
    NVGcontext* NVG    = nvgCreateGL3(NVG_ANTIALIAS);

    void (*Tick)(SDL_Window* Window, NVGcontext* NVG);

    while (true) {

        RecompileLibrary(RenderLib);

        if (RenderLib->LibraryNeedsReload) {
            void (*Cleanup)(NVGcontext* NVG) = GetLibrarySymbol(RenderLib, "Cleanup");
            if (Cleanup) Cleanup(NVG);
            ReloadLibrary(RenderLib);

            void (*Setup)(NVGcontext* NVG);
            Setup = GetLibrarySymbol(RenderLib, "Setup");
            Tick  = GetLibrarySymbol(RenderLib, "Tick");
            if (Setup) Setup(NVG);
        }

        if (Tick) Tick(Window, NVG);
    }

    return 0;
}

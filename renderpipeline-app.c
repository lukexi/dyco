#include "dynamic.h"
#include "gl.h"

int main(int argc, char const *argv[])
{
    SDL_Window* Window = CreateWindow("Render Pipeline", 10,10, 1024,768);

    library* RenderPipeline = CreateLibrary(
        "renderpipeline-main",
        "renderpipeline-main.c", NULL, NULL);
    void (*TickRenderPipeline)(SDL_Window* Window);
    void (*Cleanup)(void);

    TickRenderPipeline = GetLibrarySymbol(RenderPipeline, "TickRenderPipeline");
    Cleanup            = GetLibrarySymbol(RenderPipeline, "Cleanup");
    while (true) {

        if (RecompileLibrary(RenderPipeline)) {
            if (Cleanup) Cleanup();
            ReloadLibrary(RenderPipeline);
            TickRenderPipeline = GetLibrarySymbol(RenderPipeline, "TickRenderPipeline");
            Cleanup            = GetLibrarySymbol(RenderPipeline, "Cleanup");
        }

        if (TickRenderPipeline) TickRenderPipeline(Window);
    }

    return 0;
}

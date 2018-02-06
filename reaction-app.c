#include "dynamic.h"
#include "gl.h"

int main(int argc, char const *argv[])
{
    SDL_Window* Window = CreateWindow("Reaction Diffusion", 10,10, 1024,768);

    library* Reaction = CreateLibrary(
        "reaction-main",
        "reaction-main.c", NULL, NULL);
    void (*TickReaction)(SDL_Window* Window);
    void (*Cleanup)(void);

    TickReaction = GetLibrarySymbol(Reaction, "TickReaction");
    Cleanup      = GetLibrarySymbol(Reaction, "Cleanup");
    while (true) {

        if (RecompileLibrary(Reaction)) {
            if (Cleanup) Cleanup();
            ReloadLibrary(Reaction);
            TickReaction = GetLibrarySymbol(Reaction, "TickReaction");
            Cleanup      = GetLibrarySymbol(Reaction, "Cleanup");
        }

        if (TickReaction) TickReaction(Window);
    }

    return 0;
}

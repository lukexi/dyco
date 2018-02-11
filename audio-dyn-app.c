#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "gl.h"
#include "audio-interface.h"
#include "dynamic.h"
#include "audio-jack.h"

int AudioCallback(jack_nframes_t NumFrames, void *UserData) {

    audio_state *AudioState = (audio_state*)UserData;
    if (!AudioState) return 0;

    if (AudioState->UGen->LibraryNeedsReload || !AudioState->TickUGen) {
        void (*Cleanup)(audio_state*) = GetLibrarySymbol(AudioState->UGen, "Cleanup");
        if (Cleanup) Cleanup(AudioState);
        ReloadLibrary(AudioState->UGen);
        AudioState->TickUGen = GetLibrarySymbol(AudioState->UGen, "TickUGen");
    }
    const jack_nframes_t SampleRate = jack_get_sample_rate(AudioState->Jack->Client);

    float* OutLeft  = (float*)jack_port_get_buffer(AudioState->Jack->OutL, NumFrames);
    float* OutRight = (float*)jack_port_get_buffer(AudioState->Jack->OutR, NumFrames);
    memset(OutLeft,  0, sizeof(float) * NumFrames);
    memset(OutRight, 0, sizeof(float) * NumFrames);

    if (AudioState->TickUGen) AudioState->TickUGen(AudioState,
        NumFrames, SampleRate, OutLeft, OutRight);
    return 0;
}

int main(int argc, char const *argv[]) {
    audio_state* AudioState = calloc(1, sizeof(audio_state));

    AudioState->UGen = CreateLibrary("audio-dyn-audio", "audio-dyn-audio.c");
    AudioState->TickUGen = GetLibrarySymbol(AudioState->UGen, "TickUGen");

    AudioState->Jack = StartJack("Dynamic Audio", AudioCallback, AudioState);
    if (!AudioState->Jack) { free(AudioState); return -1; }

    library* Render = CreateLibrary("audio-dyn-render", "audio-dyn-render.c");

    SDL_Window* Window = CreateWindow("Dynamic Audio", 10,10, 1024,768);

    void* (*TickRender)(SDL_Window* Window, audio_state* AudioState);
    TickRender = GetLibrarySymbol(Render, "TickRender");

    while (true) {
        RecompileLibrary(AudioState->UGen);
        RecompileLibrary(Render);

        if (Render->LibraryNeedsReload) {
            void (*Cleanup)(void) = GetLibrarySymbol(Render, "Cleanup");
            if (Cleanup) Cleanup();
            ReloadLibrary(Render);
            TickRender = GetLibrarySymbol(Render, "TickRender");
        }

        if (TickRender) TickRender(Window, AudioState);
    }


    return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "gl.h"
#include "audio-interface.h"
#include "dynamic.h"
#include "shader.h"
#include "quad.h"
#include "audio-jack.h"

int AudioCallback(jack_nframes_t NumFrames, void *UserData) {

    audio_state *AudioState = (audio_state*)UserData;
    if (!AudioState) return 0;

    if (ReloadLibrary(AudioState->UGen) || !AudioState->TickUGen) {
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

    CreateRingBuffer(&AudioState->AudioTapRed, sizeof(audio_block), 64);
    CreateRingBuffer(&AudioState->AudioTapGrn, sizeof(audio_block), 64);
    CreateRingBuffer(&AudioState->AudioTapBlu, sizeof(audio_block), 64);

    AudioState->UGen = CreateLibrary("audio-wavetable", "audio-wavetable.c");
    AudioState->TickUGen = GetLibrarySymbol(AudioState->UGen, "TickUGen");

    AudioState->Jack = StartJack("Wavetable", AudioCallback, AudioState);
    if (!AudioState->Jack) {
        free(AudioState);
        return -1;
    }

    library* AudioRender = CreateLibrary("audio-render", "audio-render.c");

    SDL_Window* Window = CreateWindow("Wavetable", 10,10, 1024,768);

    void* (*TickRender)(SDL_Window* Window, audio_state* AudioState, void* State);
    TickRender = GetLibrarySymbol(AudioRender, "TickRender");
    void* State = NULL;
    while (true) {
        RecompileLibrary(AudioState->UGen);
        RecompileLibrary(AudioRender);

        if (AudioRender->LibraryNeedsReload) {
            void (*Cleanup)(void) = GetLibrarySymbol(AudioRender, "Cleanup");
            if (Cleanup) Cleanup();
            ReloadLibrary(AudioRender);
            TickRender = GetLibrarySymbol(AudioRender, "TickRender");
        }

        if (TickRender) State = TickRender(Window, AudioState, State);
    }


    return 0;
}

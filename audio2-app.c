#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <memory.h>
#include "dynamic.h"
#include "audio-jack.h"
#include "audio2-interface.h"

library* UGen;
int (*TickUGen)(uint32_t NumFrames, uint32_t SampleRate, float* OutL, float* OutR);

int AudioCallback(jack_nframes_t NumFrames, jack* Jack) {

    float* OutLeft  = (float*)jack_port_get_buffer(Jack->OutL, NumFrames);
    float* OutRight = (float*)jack_port_get_buffer(Jack->OutR, NumFrames);

    // Silence the output by default
    memset(OutLeft,  0, sizeof(float) * NumFrames);
    memset(OutRight, 0, sizeof(float) * NumFrames);
    const jack_nframes_t SampleRate = jack_get_sample_rate(Jack->Client);
    if (ReloadLibrary(UGen) || !TickUGen) TickUGen = GetLibrarySymbol(UGen, "TickUGen");
    if (TickUGen) TickUGen(NumFrames, SampleRate, OutLeft, OutRight);
    return 0;
}

int main(int argc, char const *argv[]) {

    UGen = CreateLibrary(
        "audio2-oscs",
        "audio2-oscs.c", NULL, NULL);
    TickUGen = GetLibrarySymbol(UGen, "TickUGen");

    jack* Jack = StartJack("Modular Wavetable", (JackProcessCallback)AudioCallback, NULL);
    if (!Jack) {
        return -1;
    }

    while (true) {
        RecompileLibrary(UGen);
    }

    return 0;
}

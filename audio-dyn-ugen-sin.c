#include "audio-dyn-interface.h"
#include "audio-lib.c"
#include <stdio.h>

typedef struct {
    double Phase;
} oscillator;


float Oscillator(oscillator* State, int SampleRate, float* Wavetable, float Freq) {
    const float T = 1/(float)SampleRate;

    State->Phase += T * Freq * 512;

    // No interpolation
    // const int Index = ((int)State->Phase) % 512;
    // const float Amplitude = Wavetable[Index];

    // Linear interpolation
    double Integral;
    const float Fractional = modf(State->Phase, &Integral);
    const int Index0 = ((int)State->Phase + 0) % 512;
    const int Index1 = ((int)State->Phase + 1) % 512;
    const float Amplitude = ((1-Fractional) * Wavetable[Index0]) +
                            (Fractional * Wavetable[Index1]);

    return Amplitude;
}

void Cleanup(audio_unit* Unit) {
    if (!Unit) return;
    // printf("Cleaning up\n");
    free(Unit->State);
    Unit->State = NULL;
}

void TickUGen(
    audio_unit* Unit,
    uint32_t NumFrames,
    uint32_t SampleRate) {
    if (!Unit) return;

    if (!Unit->State) {
        // printf("Initializing\n");
        Unit->State = calloc(1, sizeof(oscillator));
        InitWavetables();
    }
    oscillator* State = (oscillator*)Unit->State;

    for (int I = 0; I < NumFrames; I++) {
        Unit->Output[I] = Oscillator(State,
            SampleRate,
            SquWave,
            GetInput(Unit->Inputs[0], I));
    }
}

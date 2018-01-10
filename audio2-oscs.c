#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include "audio-lib.c"

#include "audio2-interface.h"



typedef struct {
    double Phase;
    float Out[BLOCK_SIZE];
} oscillator;

typedef struct ugen ugen;

struct ugen {
    size_t StateSize; // A ugen implements StateSize to return the size of the struct it uses for state
    void*  State;     // A ugen implements Initialize to take a (StateSize-sized) pointer to memory it can use and fill it appropriately

    void(*Tick)(ugen State);

    float** Inputs;
    float** Outputs;
};

oscillator Osc1;
oscillator Osc2;
oscillator Osc3;

typedef struct {

} oscillator;

void Initialize() {
    static bool Initialized = false;
    if (Initialized) return;
    InitWavetables();



    Initialized = true;
}

float Osc(oscillator* Osc, int SampleRate, float Freq, float* Wavetable) {
    const float Step = 1/(float)SampleRate;
    Osc->Phase += Step * Freq;
    const int Index = ((int)Osc->Phase * 512) % 512;
    return Wavetable[Index];
}

int TickUGen(uint32_t NumFrames, uint32_t SampleRate, float* OutL, float* OutR) {
    Initialize();

    for (int N = 0; N < NumFrames; N++) {

        const float Freq1 = Osc(&Osc3, SampleRate, 2,     SinWave) * 440 + 440;
        const float Freq  = Osc(&Osc2, SampleRate, Freq1, SinWave) * 440 + 440;
        const float Out   = Osc(&Osc1, SampleRate, Freq,  SinWave);

        *OutL++ = Out;
        *OutR++ = Out;
    }
    return 0;
}

#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include "audio-lib.c"

float SinWave[512];
float SawWave[512];
float SquWave[512];
float TriWave[512];

void InitWavetables() {
    for (int I = 0; I < 512; I++) {
        float PhaseR = (float)I / (float)512 * TAU;
        SinWave[I] = sin(PhaseR);

        const int NumPartials = 30;

        for (int K = 1; K < NumPartials; K++) SawWave[I] += pow(-1, K) * sin(K * PhaseR) / (float)K;
        SawWave[I] *= 2/M_PI;

        for (int K = 1; K < NumPartials; K++) SquWave[I] += sin(PhaseR*(2*K-1)) / (float)(2*K-1);
        SquWave[I] *= 4/M_PI;

        for (int K = 0; K < NumPartials; K++) TriWave[I] += pow(-1, K) * pow(2*K+1, -2) * sin(PhaseR*(2*K+1));
    }
}

bool Initialized = false;
void Initialize() {
    Initialized = true;
    InitWavetables();
}

typedef struct {
    double Phase;
} oscillator;

oscillator Osc1;
oscillator Osc2;

float TickOsc(oscillator* Osc, int SampleRate, float Freq, float* Wavetable) {
    const float Step = 1/(float)SampleRate;
    Osc->Phase += Step * Freq * 512;
    const int Index = ((int)Osc->Phase) % 512;
    return Wavetable[Index];
}

int TickUGen(uint32_t NumFrames, uint32_t SampleRate, float* OutL, float* OutR) {
    if (!Initialized) Initialize();
    // printf("Hi\n");

    const float Freq1 = 440;
    for (int N = 0; N < NumFrames; N++) {
        const float Out = TickOsc(&Osc1, SampleRate, 440, SinWave);
        *OutL++ = Out;
        *OutR++ = Out;
    }
    return 0;
}

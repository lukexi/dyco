#include "audio-interface.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

bool Initialized;
float SinWave[512];
float SawWave[512];

const double TAU = M_PI * 2;

void Initialize() {
    for (int I = 0; I < 512; I++) {
        float PhaseR = (float)I / (float)512 * TAU;
        SinWave[I] = sin(PhaseR);

        for (int K = 1; K < 30; K++) {
            SawWave[I] += pow(-1, K) * sin(K * PhaseR) / (float)K;
        }
        SawWave[I] *= 2/M_PI;
    }

    Initialized = true;
}

float RandomFloat() {
    return (float)rand() / (float)RAND_MAX;
}

float RandomRange(float Low, float High) {
    const float Range = High-Low;
    return Low + Range * RandomFloat();
}

long GlobalPhase = 0;

int TickUGen(jack_nframes_t NumFrames, void *Arg) {
    if (!Initialized) Initialize();

    audio_state *AudioState = (audio_state*)Arg;
    jack_default_audio_sample_t *OutputBufferLeft  = jack_port_get_buffer(AudioState->OutputPortLeft,  NumFrames);
    jack_default_audio_sample_t *OutputBufferRight = jack_port_get_buffer(AudioState->OutputPortRight, NumFrames);
    float *OutLeft  = (float*)OutputBufferLeft;
    float *OutRight = (float*)OutputBufferRight;

    const jack_nframes_t SampleRate = jack_get_sample_rate(AudioState->Client);
    static int R = 1;
    for (int SampleIndex = 0; SampleIndex < NumFrames; SampleIndex++) {
        float Mix = sin((float)(GlobalPhase%SampleRate) / SampleRate * 4 * 2*M_PI) * 0.5 + 0.5;

        float Output1 = SawWave[ (int)(GlobalPhase*R) % 512 ];
        float Output2 = SinWave[ (int)(GlobalPhase*R) % 512 ];

        float Output = (Output1 * Mix) + (Output2 * (1-Mix));

        Output *= 0.1;
        *OutLeft++  = Output;
        *OutRight++ = Output;

        GlobalPhase++;


        if ((GlobalPhase%SampleRate) == 0) R = rand() % 5 + 1;
    }

    return 0;
}

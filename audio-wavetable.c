#include "audio-interface.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

bool Initialized;
float SinWave[512];
float SawWave[512];
float SquWave[512];

const double TAU = M_PI * 2;

void Initialize() {
    for (int I = 0; I < 512; I++) {
        float PhaseR = (float)I / (float)512 * TAU;
        SinWave[I] = sin(PhaseR);

        const int NumPartials = 30;
        for (int K = 1; K < NumPartials; K++) {
            SawWave[I] += pow(-1, K) * sin(K * PhaseR) / (float)K;
        }
        SawWave[I] *= 2/M_PI;

        for (int K = 1; K < NumPartials; K++) {
            SquWave[I] += sin(PhaseR*(2*K-1)) / (float)(2*K-1);
        }
        SquWave[I] *= 4/M_PI;
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
    float* OutLeft  = (float*)jack_port_get_buffer(AudioState->OutputPortLeft,  NumFrames);
    float* OutRight = (float*)jack_port_get_buffer(AudioState->OutputPortRight, NumFrames);

    const jack_nframes_t SampleRate = jack_get_sample_rate(AudioState->Client);
    static int Seq = 1;
    for (int SampleIndex = 0; SampleIndex < NumFrames; SampleIndex++) {
        float Mix = sin((float)(GlobalPhase%SampleRate) / SampleRate * 1 * TAU) * 0.5 + 0.5;

        float Output1 = SawWave[ (int)(GlobalPhase*Seq) % 512 ];
        float Output2 = SquWave[ (int)(GlobalPhase*Seq) % 512 ];

        Output1 += SawWave[ (int)(GlobalPhase*Seq*1.5) % 512 ];
        Output2 += SquWave[ (int)(GlobalPhase*Seq*1.5) % 512 ];

        float OutputL = (Output1 * Mix) + (Output2 * (1-Mix));
        float OutputR = (Output1 * (1-Mix)) + (Output2 * Mix);

        OutputL *= 0.1;
        OutputR *= 0.1;
        *OutLeft++  = OutputL;
        *OutRight++ = OutputR;

        GlobalPhase++;


        if ((GlobalPhase%(SampleRate/8)) == 0) Seq = rand() % 7 + 1;
    }

    return 0;
}

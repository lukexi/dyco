#include "audio-interface.h"
#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

bool Initialized;
float SinWave[512];
float SawWave[512];
float SquWave[512];
float TriWave[512];

const double TAU = M_PI * 2;

void Initialize() {
    for (int I = 0; I < 512; I++) {
        float PhaseR = (float)I / (float)512 * TAU;
        SinWave[I] = sin(PhaseR);

        const int NumPartials = 10;
        for (int K = 1; K < NumPartials; K++) {
            SawWave[I] += pow(-1, K) * sin(K * PhaseR) / (float)K;
        }
        SawWave[I] *= 2/M_PI;

        for (int K = 1; K < NumPartials; K++) {
            SquWave[I] += sin(PhaseR*(2*K-1)) / (float)(2*K-1);
        }
        SquWave[I] *= 4/M_PI;

        for (int K = 0; K < NumPartials; K++) {
            TriWave[I] += pow(-1, K) * pow(2*K+1, -2) * sin(PhaseR*(2*K+1));
        }
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
    float* OutLeftS  = (float*)jack_port_get_buffer(AudioState->OutputPortLeft,  NumFrames);
    float* OutRightS = (float*)jack_port_get_buffer(AudioState->OutputPortRight, NumFrames);
    float* OutLeft = OutLeftS;
    float* OutRight = OutRightS;

    const jack_nframes_t SampleRate = jack_get_sample_rate(AudioState->Client);
    static int Seq = 1;


    for (int SampleIndex = 0; SampleIndex < NumFrames; SampleIndex++) {
        float Mix = sin((float)(GlobalPhase%SampleRate) / SampleRate * 1 * TAU) * 0.5 + 0.5;
        float Wave1 = SawWave[ (int)(GlobalPhase*Seq*1.001) % 512 ];
        float Wave2 = SquWave[ (int)(GlobalPhase*Seq*0.999) % 512 ];
        // Mix=0;

        Wave1 += SawWave[ (int)(GlobalPhase*Seq*1.5) % 512 ];
        Wave2 += SquWave[ (int)(GlobalPhase*Seq*1.5*1.001) % 512 ];

        float OutputL = Wave1*Mix + Wave2*(1-Mix);
        float OutputR = Wave1*(1-Mix) + Wave2*Mix;

        OutputL *= 0.1;
        OutputR *= 0.1;
        *OutLeft++  = OutputL;
        *OutRight++ = OutputR;

        GlobalPhase++;


        if ((GlobalPhase%(SampleRate/1)) == 0) Seq = rand() % 7 + 1;
    }

    const size_t BlockSize = sizeof(float) * NumFrames;
    audio_block TapL;
    audio_block TapR;
    TapL.Length = NumFrames;
    TapR.Length = NumFrames;
    TapL.Samples = malloc(BlockSize);
    TapR.Samples = malloc(BlockSize);
    memcpy(TapL.Samples, OutLeftS, BlockSize);
    memcpy(TapR.Samples, OutRightS, BlockSize);
    WriteRingBuffer(&AudioState->AudioTapL, &TapL, 1);
    WriteRingBuffer(&AudioState->AudioTapR, &TapR, 1);

    return 0;
}

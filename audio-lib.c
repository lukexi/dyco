#include "audio-lib.h"
#include <stdlib.h>

float TransposeRatio(float Semitones) {
    return pow(2, Semitones/12);
}

float MIDIToFreq(float MIDINote) {
    const float A440Note = 69;
    return 440 * TransposeRatio(MIDINote - A440Note);
}

float RandomFloat() {
    return (float)rand() / (float)RAND_MAX;
}

float RandomRange(float Low, float High) {
    const float Range = High-Low;
    return Low + Range * RandomFloat();
}

float Clamp(float x, float lowerlimit, float upperlimit) {
  if (x < lowerlimit)
    x = lowerlimit;
  if (x > upperlimit)
    x = upperlimit;
  return x;
}

float Lerp(float From, float To, float X) {
    return From + ((To - From) * Clamp(X, 0, 1));
}

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

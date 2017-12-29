#include "audio-interface.h"
#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

float MajorScale[] = {0,2,4,5,7,9,11};
#define RAND_FLOAT ((float)rand() / (float)RAND_MAX)
#define RAND_INT(Lo, Hi) (Lo + (rand() % (Hi - Lo)))
#define RAND_RANGE(Lo, Hi) (Lo+RAND_FLOAT*(Hi-Lo))

#define ARRAY_SIZE(Array) (sizeof(Array) / sizeof(*Array))
#define ARRAY_END(Array) (Array + ARRAY_SIZE(Array))
#define RANDOM_ITEM(Array) (Array[(int)floor(RAND_FLOAT * ARRAY_SIZE(Array))])

const double TAU = M_PI * 2;

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



bool Initialized;
float SinWave[512];
float SawWave[512];
float SquWave[512];
float TriWave[512];

typedef struct {
    double Phase;
    float Freq;
    float OldFreq;
    float NewFreq;
    float Slew;
    float SlewRate;
} oscillator;

oscillator Osc[4];
oscillator LFO;

void SetFreq(oscillator* Oscillator, float Freq, float SlewRate) {
    Oscillator->OldFreq = Oscillator->Freq;
    Oscillator->NewFreq = Freq;
    Oscillator->Slew = 0;
    Oscillator->SlewRate = SlewRate;
    if (SlewRate <= 0) Oscillator->Freq = Freq;
}

float TickOscillator(oscillator* Oscillator, int SampleRate, float* Wavetable) {
    const float T = 1/(float)SampleRate;

    if (Oscillator->SlewRate > 0) {
        Oscillator->Freq = Lerp(Oscillator->OldFreq, Oscillator->NewFreq, Oscillator->Slew);
        Oscillator->Slew += T*Oscillator->SlewRate;
    }

    Oscillator->Phase += T * Oscillator->Freq * 512;

    const int Index = ((int)Oscillator->Phase) % 512;

    return Wavetable[Index];
}






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

        for (int K = 0; K < NumPartials; K++) {
            TriWave[I] += pow(-1, K) * pow(2*K+1, -2) * sin(PhaseR*(2*K+1));
        }
    }
    float BaseFreq = MIDIToFreq(60);

    SetFreq(&Osc[0], BaseFreq, 60);
    SetFreq(&Osc[1], BaseFreq, 60);
    SetFreq(&Osc[2], BaseFreq, 60);
    SetFreq(&Osc[3], BaseFreq, 60);

    Initialized = true;
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


    SetFreq(&LFO, 0.1, 0);
    for (int SampleIndex = 0; SampleIndex < NumFrames; SampleIndex++) {

        float Mix = TickOscillator(&LFO, SampleRate, SinWave) * 0.5 + 0.5;

        float Wave1 = TickOscillator(&Osc[0], SampleRate, SawWave);
        float Wave2 = TickOscillator(&Osc[1], SampleRate, SquWave);
        Wave1 += TickOscillator(&Osc[2], SampleRate, SinWave);
        Wave2 += TickOscillator(&Osc[3], SampleRate, TriWave);

        float OutputL = Wave1*Mix + Wave2*(1-Mix);
        float OutputR = Wave1*(1-Mix) + Wave2*Mix;

        OutputL *= 0.1;
        OutputR *= 0.1;
        *OutLeft++  = OutputL;
        *OutRight++ = OutputR;

        GlobalPhase++;

        int SeqDur = (SampleRate/2);
        if ((GlobalPhase%SeqDur) == 0) {

            int BaseNote = RANDOM_ITEM(MajorScale)
                + RAND_INT(0,3) * 12 + 40; // rand() % 24 + 30;
            float BaseFreq = MIDIToFreq(BaseNote);
            // BaseFreq = floor(BaseFreq);

            SetFreq(&Osc[0], BaseFreq*TransposeRatio(1), 50);
            SetFreq(&Osc[1], BaseFreq*TransposeRatio(4), 50);
            SetFreq(&Osc[2], BaseFreq*TransposeRatio(8), 50);
            SetFreq(&Osc[3], BaseFreq*TransposeRatio(11), 50);
        }
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

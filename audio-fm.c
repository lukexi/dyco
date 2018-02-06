#include "audio-interface.h"
#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "utils.h"
#include "audio-lib.c"
#include "audio-filter.c"
#include "audio-ugens.c"

// VOICE

typedef struct {
    env_ar Env;
    oscillator Osc;
    float OscFB;
} operator;

typedef struct {
    float Freq;
    trigger Trigger;
    operator Op1;
    operator Op2;
    operator Op3;
    operator Op4;
} voice;

float Operator(operator* State, int SR, float Trig, float Attack, float Decay, float Feedback, float Freq) {
    const float Osc = Oscillator(&State->Osc, SR, SinWave, Freq + Feedback * State->OscFB);
    State->OscFB = Osc;
    return
        Osc
        * EnvAR(&State->Env, SR, Attack, Decay, Trig);
}

float Voice(voice* State, int SR, float TrigRate) {

    const float Trig = Trigger(&State->Trigger, SR, TrigRate);
    if (Trig > 0) {
        State->Freq =
            MIDIToFreq(
                40 + (RAND_INT(0,3) * 12) + RANDOM_ITEM(MinorScale)
                );
    }

    const float Freq = State->Freq;
    float Op1 = Operator(&State->Op1, SR, Trig, 1/TrigRate/4.0, 1/TrigRate/2.0, 0, Freq);
    float Op2 = Operator(&State->Op2, SR, Trig, 1/TrigRate/4.0, 1/TrigRate/2.0, 0, Freq + Op1 * 1);
    float Op3 = Operator(&State->Op3, SR, Trig, 1/TrigRate/4.0, 1/TrigRate/2.0, 0, Freq + Op2 * 1);
    float Op4 = Operator(&State->Op4, SR, Trig, 0.01,           1/TrigRate,     0, Freq + Op3 * 1000);

    return Op4;
}

void InitTap(audio_block* Tap, int NumFrames) {
    Tap->Samples = malloc(sizeof(float) * NumFrames);
    Tap->Freqs   = malloc(sizeof(float) * NumFrames);
    Tap->Length = NumFrames;
}

int TickUGen(audio_state *AudioState,
    uint32_t NumFrames,
    uint32_t SampleRate,
    float* OutL,
    float* OutR)
{

    static bool Initialized = false;
    if (!Initialized) {
        InitWavetables();

        Initialized = true;
    }

    audio_block TapRed;
    InitTap(&TapRed, NumFrames);
    float* TapRedIn     = TapRed.Samples;

    audio_block TapGrn;
    InitTap(&TapGrn, NumFrames);
    float* TapGrnIn     = TapGrn.Samples;

    audio_block TapBlu;
    InitTap(&TapBlu, NumFrames);
    float* TapBluIn     = TapBlu.Samples;

    static voice Voices[3];

    for (int SampleIndex = 0; SampleIndex < NumFrames; SampleIndex++) {

        float Wave1 = Voice(&Voices[0], SampleRate, 2);
        float Wave2 = Voice(&Voices[1], SampleRate, 1);
        float Wave3 = Voice(&Voices[2], SampleRate, 8);

        *TapRedIn++ = Wave1;
        *TapGrnIn++ = Wave2;
        *TapBluIn++ = Wave3;

        float Out = (Wave1 + Wave2 + Wave3) * 0.3;
        *OutL++ = Out;
        *OutR++ = Out;
    }

    WriteRingBuffer(&AudioState->AudioTapRed, &TapRed, 1);
    WriteRingBuffer(&AudioState->AudioTapGrn, &TapGrn, 1);
    WriteRingBuffer(&AudioState->AudioTapBlu, &TapBlu, 1);

    return 0;
}

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
    env_ar Env1;
    trigger Trigger;
    slew_gen SlewGen;
    oscillator Osc1;
    oscillator LFO1;
    lowpass_filter Filter;
} voice;

float Voice(voice* State, int SR, float* Wavetable) {

    const float Trig = Trigger(&State->Trigger, SR, 3);
    const float SlewRate = 40;
    const float Slew = SlewGen(&State->SlewGen, SR, SlewRate,
        Trig,
        MIDIToFreq(RANDOM_ITEM(MajorScale) + RAND_INT(0,3) * 12 + 50));

    const float Env1 = EnvAR(&State->Env1, SR, 0.01, 1.0/1.0, Trig);

    const float Osc1 = Oscillator(&State->Osc1, SR, Wavetable, Slew);
    const float LFO1 = Oscillator(&State->LFO1, SR, SinWave, 1);
    const float Out  = Lowpass(&State->Filter, SR,
        (LFO1*0.5+0.5)*3000+100, // Freq
        0.8,                     // Res
        Osc1);                   // In
    return Out * Env1;
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
    static voice Voices[3];
    static oscillator LFO;

    static bool Initialized = false;
    if (!Initialized) {
        InitWavetables();
        const float BaseFreq = MIDIToFreq(60);

        SlewGen(&Voices[0].SlewGen, SampleRate, 0, 1, BaseFreq);
        SlewGen(&Voices[1].SlewGen, SampleRate, 0, 1, BaseFreq);
        SlewGen(&Voices[2].SlewGen, SampleRate, 0, 1, BaseFreq);

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

    for (int SampleIndex = 0; SampleIndex < NumFrames; SampleIndex++) {

        float Mix = Oscillator(&LFO, SampleRate, SinWave, 0.1) * 0.5 + 0.5;

        float Wave1 = Voice(&Voices[0], SampleRate, SinWave);
        float Wave2 = Voice(&Voices[1], SampleRate, SawWave);
        float Wave3 = Voice(&Voices[2], SampleRate, SquWave);

        *TapRedIn++ = Wave1;
        *TapGrnIn++ = Wave2;
        *TapBluIn++ = Wave3;

        float OutputL = Wave1*Mix + Wave2*(1-Mix) + Wave3;
        float OutputR = Wave1*(1-Mix) + Wave2*Mix + Wave3;

        *OutL++  = OutputL * 0.3;
        *OutR++ = OutputR * 0.3;
    }

    WriteRingBuffer(&AudioState->AudioTapRed, &TapRed, 1);
    WriteRingBuffer(&AudioState->AudioTapGrn, &TapGrn, 1);
    WriteRingBuffer(&AudioState->AudioTapBlu, &TapBlu, 1);

    return 0;
}

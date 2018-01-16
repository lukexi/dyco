#include "audio-interface.h"
#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "audio-lib.c"
#include "audio-filter.c"

// TRIGGER
typedef struct {
    float Phase;
} trigger;

float TickTrigger(trigger* Trigger, int SampleRate, float Freq) {
    const float T = 1/(float)SampleRate;
    const float NewPhase = Trigger->Phase + T * Freq;
    const float Output = (int)Trigger->Phase == (int)NewPhase ? 0 : 1;
    Trigger->Phase = NewPhase;
    return Output;
}

// SLEW GENERATOR
typedef struct {
    float OldValue;
    float NewValue;
    float SlewPhase;
} slew_gen;

void SetSlewGen(slew_gen* SlewGen, float Value) {
    SlewGen->OldValue = Lerp(SlewGen->OldValue, SlewGen->NewValue, SlewGen->SlewPhase);
    SlewGen->NewValue = Value;
    SlewGen->SlewPhase = 0;
}

float TickSlewGen(slew_gen* SlewGen, int SampleRate, float SlewRate) {
    const float T = 1/(float)SampleRate;
    SlewGen->SlewPhase += T * SlewRate;

    if (SlewRate <= 0) return SlewGen->NewValue;
    return Lerp(SlewGen->OldValue, SlewGen->NewValue, SlewGen->SlewPhase);
}


// OSCILLATOR
typedef struct {
    double Phase;
} oscillator;

float TickOscillator(oscillator* Oscillator, int SampleRate, float* Wavetable, float Freq) {
    const float T = 1/(float)SampleRate;

    Oscillator->Phase += T * Freq * 512;

    // No interpolation
    // const int Index = ((int)Oscillator->Phase) % 512;
    // const float Amplitude = Wavetable[Index];

    // Linear interpolation
    double Integral;
    const float Fractional = modf(Oscillator->Phase, &Integral);
    const int Index0 = ((int)Oscillator->Phase + 0) % 512;
    const int Index1 = ((int)Oscillator->Phase + 1) % 512;
    const float Amplitude = ((1-Fractional) * Wavetable[Index0]) +
                            (Fractional * Wavetable[Index1]);

    return Amplitude;
}

// VOICE
typedef struct {
    trigger Trigger;
    slew_gen SlewGen;
    oscillator Osc1;
    oscillator LFO1;
    filter_rlop Filter;
} voice;

float TickVoice(voice* Voice, int SampleRate, float* Wavetable) {
    if (TickTrigger(&Voice->Trigger, SampleRate, 2) > 0) {
        SetSlewGen(&Voice->SlewGen,
            MIDIToFreq(RANDOM_ITEM(MajorScale) + RAND_INT(0,3) * 12 + 50));
    }

    const float SlewRate = 4;
    const float Slew = TickSlewGen(&Voice->SlewGen, SampleRate, SlewRate);
    const float Osc1 = TickOscillator(&Voice->Osc1, SampleRate, Wavetable, Slew);
    const float LFO1 = TickOscillator(&Voice->LFO1, SampleRate, SinWave, 1);
    const float Out  = TickFilter(&Voice->Filter, SampleRate,
        (LFO1*0.5+0.5)*3000+100, // Freq
        0.8, // Res
        Osc1); // In
    return Out;
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
        float BaseFreq = MIDIToFreq(60);

        SetSlewGen(&Voices[0].SlewGen, BaseFreq);
        SetSlewGen(&Voices[1].SlewGen, BaseFreq);
        SetSlewGen(&Voices[2].SlewGen, BaseFreq);

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

        float Mix = TickOscillator(&LFO, SampleRate, SinWave, 0.1) * 0.5 + 0.5;

        float Wave1 = TickVoice(&Voices[0], SampleRate, SinWave);
        float Wave2 = TickVoice(&Voices[1], SampleRate, SawWave);
        float Wave3 = TickVoice(&Voices[2], SampleRate, SquWave);

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

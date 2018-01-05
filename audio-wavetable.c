#include "audio-interface.h"
#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "audio-lib.c"

bool Initialized;

float SinWave[512];
float SawWave[512];
float SquWave[512];
float TriWave[512];

void InitWavetables() {
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
}

typedef struct {
    double Phase;
    float Freq;
    float OldFreq;
    float NewFreq;
    float Slew;
    float SlewRate;
} oscillator;

oscillator Osc[3];
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

    // Linear interpolation
    double Integral;
    const float Fractional = modf(Oscillator->Phase, &Integral);
    const int Index0 = ((int)Oscillator->Phase + 0) % 512;
    const int Index1 = ((int)Oscillator->Phase + 1) % 512;
    const float Amplitude = ((1-Fractional) * Wavetable[Index0]) +
                            (Fractional * Wavetable[Index1]);

    // No interpolation
    // const int Index = ((int)Oscillator->Phase) % 512;
    // const float Amplitude = Wavetable[Index];

    return Amplitude;
}

void Initialize() {
    InitWavetables();
    float BaseFreq = MIDIToFreq(60);

    SetFreq(&Osc[0], BaseFreq, 0);
    SetFreq(&Osc[1], BaseFreq, 0);
    SetFreq(&Osc[2], BaseFreq, 0);

    Initialized = true;
}


void InitTap(audio_block* Tap, int NumFrames) {
    Tap->Samples = malloc(sizeof(float) * NumFrames);
    Tap->Freqs   = malloc(sizeof(float) * NumFrames);
    Tap->Length = NumFrames;
}

long GlobalPhase = 0;

int TickUGen(jack_nframes_t NumFrames, void *Arg) {
    if (!Initialized) Initialize();

    audio_state *AudioState = (audio_state*)Arg;
    float* OutLeft  = (float*)jack_port_get_buffer(AudioState->Jack->OutL, NumFrames);
    float* OutRight = (float*)jack_port_get_buffer(AudioState->Jack->OutR, NumFrames);

    const jack_nframes_t SampleRate = jack_get_sample_rate(AudioState->Jack->Client);


    audio_block TapRed;
    InitTap(&TapRed, NumFrames);
    float* TapRedIn     = TapRed.Samples;
    float* TapRedFreqIn = TapRed.Freqs;

    audio_block TapGrn;
    InitTap(&TapGrn, NumFrames);
    float* TapGrnIn     = TapGrn.Samples;
    float* TapGrnFreqIn = TapGrn.Freqs;

    audio_block TapBlu;
    InitTap(&TapBlu, NumFrames);
    float* TapBluIn     = TapBlu.Samples;
    float* TapBluFreqIn = TapBlu.Freqs;


    SetFreq(&LFO, 0.1, 0);
    for (int SampleIndex = 0; SampleIndex < NumFrames; SampleIndex++) {

        float Mix = TickOscillator(&LFO, SampleRate, SinWave) * 0.5 + 0.5;

        float Wave1 = TickOscillator(&Osc[0], SampleRate, SinWave);
        float Wave2 = TickOscillator(&Osc[1], SampleRate, SinWave);
        float Wave3 = TickOscillator(&Osc[2], SampleRate, SinWave);

        *TapRedIn++ = Wave1; *TapRedFreqIn++ = Osc[0].Freq;
        *TapGrnIn++ = Wave2; *TapGrnFreqIn++ = Osc[1].Freq;
        *TapBluIn++ = Wave3; *TapBluFreqIn++ = Osc[2].Freq;

        float OutputL = Wave1*Mix + Wave2*(1-Mix) + Wave3;
        float OutputR = Wave1*(1-Mix) + Wave2*Mix + Wave3;

        *OutLeft++  = OutputL * 0.3;
        *OutRight++ = OutputR * 0.3;

        GlobalPhase++;

        int SeqDur = (SampleRate/4);
        if ((GlobalPhase%SeqDur) == 0) {

            float Freq1 = MIDIToFreq(RANDOM_ITEM(MajorScale) + RAND_INT(0,3) * 12 + 40);
            float Freq2 = MIDIToFreq(RANDOM_ITEM(MajorScale) + RAND_INT(0,3) * 12 + 40);
            float Freq3 = MIDIToFreq(RANDOM_ITEM(MajorScale) + RAND_INT(0,3) * 12 + 40);

            SetFreq(&Osc[0], Freq1, 30);
            SetFreq(&Osc[1], Freq2, 30);
            SetFreq(&Osc[2], Freq3, 30);
        }
    }

    WriteRingBuffer(&AudioState->AudioTapRed, &TapRed, 1);
    WriteRingBuffer(&AudioState->AudioTapGrn, &TapGrn, 1);
    WriteRingBuffer(&AudioState->AudioTapBlu, &TapBlu, 1);

    return 0;
}

#include <math.h>
#include "utils.h"

// ENV
typedef struct {
    double Phase;
} env_ar;

float EnvAR(env_ar* State, int SampleRate, float AttackTime, float ReleaseTime, float Trig) {
    if (Trig > 0) {
        State->Phase = 0;
    }
    const float T = 1/(float)SampleRate;
    State->Phase += T;

    const float Phase = State->Phase;
    const float Linear = (Phase < AttackTime) ?
        Phase / AttackTime :
        MAX(0, 1 - (Phase - AttackTime) / ReleaseTime);
    return powf(Linear, 1.1);
}

// TRIGGER
typedef struct {
    double Phase;
} trigger;

float Trigger(trigger* State, int SampleRate, float Freq) {
    const float T = 1/(float)SampleRate;
    const float NewPhase = State->Phase + T * Freq;
    const float Output = (int)State->Phase == (int)NewPhase ? 0 : 1;
    State->Phase = NewPhase;
    // printf("State->Phase %f\n", State->Phase);
    return Output;
}

// SLEW GENERATOR
typedef struct {
    double Phase;
    float OldValue;
    float NewValue;
} slew_gen;

float SlewGen(slew_gen* State, int SampleRate, float SlewRate, float Trig, float Value) {
    if (Trig > 0) {
        State->OldValue = Lerp(State->OldValue, State->NewValue, State->Phase);
        State->NewValue = Value;
        State->Phase = 0;
    }

    const float T = 1/(float)SampleRate;
    State->Phase += T * SlewRate;

    if (SlewRate <= 0) return State->NewValue;
    return Lerp(State->OldValue, State->NewValue, State->Phase);
}


// OSCILLATOR
typedef struct {
    double Phase;
} oscillator;

float Oscillator(oscillator* State, int SampleRate, float* Wavetable, float Freq) {
    const float T = 1/(float)SampleRate;

    State->Phase += T * Freq * 512;

    // No interpolation
    // const int Index = ((int)State->Phase) % 512;
    // const float Amplitude = Wavetable[Index];

    // Linear interpolation
    double Integral;
    const float Fractional = modf(State->Phase, &Integral);
    const int Index0 = ((int)State->Phase + 0) % 512;
    const int Index1 = ((int)State->Phase + 1) % 512;
    const float Amplitude = ((1-Fractional) * Wavetable[Index0]) +
                            (Fractional * Wavetable[Index1]);

    return Amplitude;
}

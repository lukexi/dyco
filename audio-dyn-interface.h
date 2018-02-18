#ifndef AUDIO_INTERFACE_H
#define AUDIO_INTERFACE_H

#include "jack.h"
#include "audio-oscilloscope.h"
#include "ringbuffer.h"
#include "dynamic.h"

typedef struct {
    float Samples[512];
    size_t Length;
} audio_block;

typedef struct dsp_unit dsp_unit;

typedef void (*UGenTickFunc) (
    dsp_unit* Unit,
    uint32_t NumFrames,
    uint32_t SampleRate);

typedef struct {
    dsp_unit* Unit;
    float Constant;
} dsp_input;

struct dsp_unit {
    UGenTickFunc TickFunction;
    library* Library;
    dsp_input Inputs[8];
    float Output[512];
    long TickID;
    void* State;
    scope Scope;
    ringbuffer ScopeBuffer;
};

typedef struct {
    jack* Jack;
    library* UGen;
    int (*TickUGen)(
        void* AudioState,
        uint32_t NumFrames,
        uint32_t SampleRate,
        float* OutL, float* OutR
        );
    dsp_unit* OutputUnit;
} audio_state;

float GetInput(dsp_input Input, uint32_t Frame);

typedef struct {
    dsp_unit* Units[128];
    size_t Count;
} dsp_units;

#endif // AUDIO_INTERFACE_H

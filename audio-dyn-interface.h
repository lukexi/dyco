#ifndef AUDIO_INTERFACE_H
#define AUDIO_INTERFACE_H

#include "audio-jack.h"
#include "audio-oscilloscope.h"
#include "ringbuffer.h"
#include "dynamic.h"

typedef struct {
    float Samples[512];
    size_t Length;
} audio_block;

typedef struct audio_unit audio_unit;

typedef void (*UGenTickFunc) (
    audio_unit* Unit,
    uint32_t NumFrames,
    uint32_t SampleRate);

typedef struct {
    audio_unit* Unit;
    float Constant;
} audio_input;

struct audio_unit {
    UGenTickFunc TickFunction;
    library* Library;
    audio_input Inputs[8];
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
    audio_unit* OutputUnit;
} audio_state;

float GetInput(audio_input Input, uint32_t Frame);

#endif // AUDIO_INTERFACE_H

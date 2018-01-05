#ifndef AUDIO_INTERFACE_H
#define AUDIO_INTERFACE_H

#include "audio-jack.h"
#include "ringbuffer.h"
#include "dynamic.h"

typedef struct {
    float* Samples;
    float* Freqs;
    size_t Length;
} audio_block;

typedef struct {
    jack* Jack;
    library* UGen;
    int (*TickUGen)(jack_nframes_t NumFrames, void *Arg);
    ringbuffer AudioTapRed;
    ringbuffer AudioTapGrn;
    ringbuffer AudioTapBlu;
} audio_state;

#endif // AUDIO_INTERFACE_H

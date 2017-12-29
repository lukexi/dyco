#ifndef AUDIO_INTERFACE_H
#define AUDIO_INTERFACE_H

#include <jack/jack.h>
#include "ringbuffer.h"
#include "dynamic.h"

typedef struct {
    float* Samples;
    size_t Length;
} audio_block;

typedef struct {
    jack_port_t *OutputPortLeft;
    jack_port_t *OutputPortRight;
    jack_client_t *Client;
    library* UGen;
    int (*TickUGen)(jack_nframes_t NumFrames, void *Arg);
    ringbuffer AudioTapL;
    ringbuffer AudioTapR;
} audio_state;

#endif // AUDIO_INTERFACE_H

#ifndef AUDIO_INTERFACE_H
#define AUDIO_INTERFACE_H

#include "ringbuffer.h"
#include <jack/jack.h>
#include "dynamic.h"

typedef struct {
    jack_port_t *OutputPortLeft;
    jack_port_t *OutputPortRight;
    jack_client_t *Client;
    library* UGen;
    int (*TickUGen)(jack_nframes_t NumFrames, void *Arg);
} audio_state;

#endif // AUDIO_INTERFACE_H

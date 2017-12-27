#ifndef AUDIO_INTERFACE_H
#define AUDIO_INTERFACE_H

#include "ringbuffer.h"
#include <jack/jack.h>

typedef struct {
    jack_port_t *OutputPortLeft;
    jack_port_t *OutputPortRight;
    jack_client_t *Client;
} audio_state;

#endif // AUDIO_INTERFACE_H

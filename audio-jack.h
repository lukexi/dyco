#ifndef AUDIO_JACK_H
#define AUDIO_JACK_H

#include <jack/jack.h>

typedef struct {
    jack_client_t *Client;
    jack_port_t *OutL;
    jack_port_t *OutR;
} jack;

jack* StartJack(char* Name, JackProcessCallback ProcessCallback, void *ProcessArg);

#endif // AUDIO_JACK_H

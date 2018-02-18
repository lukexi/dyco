#ifndef JACK_H
#define JACK_H

#include <jack/jack.h>

typedef struct {
    jack_client_t *Client;
    jack_port_t *OutL;
    jack_port_t *OutR;
} jack;

jack* StartJack(char* Name, JackProcessCallback ProcessCallback, void *ProcessArg);

// Frees and NULLs the pointer.
void StopJack(jack** JackPtr);

#endif // JACK_H

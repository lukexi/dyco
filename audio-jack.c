#include "audio-jack.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

bool ConnectJack(
    jack_client_t *Client,
    jack_port_t *OutL,
    jack_port_t *OutR
    );

jack* StartJack(char* Name, JackProcessCallback ProcessCallback, void *ProcessArg) {

    jack* Jack = calloc(1, sizeof(jack));

    const char *ServerName = NULL;
    jack_options_t Options = JackNullOption;
    jack_status_t Status;

    Jack->Client = jack_client_open(Name, Options, &Status, ServerName);
    if (Jack->Client == NULL) {
        fprintf(stderr, "jack_client_open() failed, status = 0x%2.0x\n", Status);
        if (Status & JackServerFailed) {
            fprintf(stderr, "Unable to connect to JACK server\n");
        }
        free(Jack);
        return NULL;
    }
    if (Status & JackServerStarted) {
        fprintf(stderr, "JACK server started\n");
    }


    // jack_on_shutdown(client, jack_shutdown, 0);

    Jack->OutL = jack_port_register(Jack->Client,  "output_left",
                      JACK_DEFAULT_AUDIO_TYPE,
                      JackPortIsOutput|JackPortIsTerminal, 0);
    Jack->OutR = jack_port_register(Jack->Client, "output_right",
                      JACK_DEFAULT_AUDIO_TYPE,
                      JackPortIsOutput|JackPortIsTerminal, 0);

    if (Jack->OutL == NULL || Jack->OutR == NULL) {
        fprintf(stderr, "no more JACK ports available\n");
        free(Jack);
        return NULL;
    }

    // If no argument, pass ourselves
    if (ProcessArg == NULL) ProcessArg = Jack;
    jack_set_process_callback(Jack->Client, ProcessCallback, ProcessArg);

    if (jack_activate(Jack->Client)) {
        fprintf(stderr, "cannot activate client");
        free(Jack);
        return NULL;
    }

    bool Connected = ConnectJack(Jack->Client, Jack->OutL, Jack->OutR);
    if (!Connected) {
        jack_deactivate(Jack->Client);
        free(Jack);
        return NULL;
    }
    return Jack;
}

bool ConnectJack(
    jack_client_t *Client,
    jack_port_t *OutL,
    jack_port_t *OutR
    )
{
    // Connect our left and right ports to JACK's ports
    // ("Input" here meaning we are "Inputting to JACK")
    const char** Ports = jack_get_ports(Client,
        NULL, NULL, JackPortIsPhysical|JackPortIsInput);
    if (Ports == NULL) {
        fprintf(stderr, "no physical playback ports\n");
        return false;
    }

    if (jack_connect(Client, jack_port_name(OutL),  Ports[0]) ||
        jack_connect(Client, jack_port_name(OutR), Ports[1]))
    {
        fprintf(stderr, "cannot connect output ports\n");
        free(Ports);
        return false;
    }

    free(Ports);
    return true;
}

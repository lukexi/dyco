#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "audio-interface.h"
#include "dynamic.h"

int AudioCallback(jack_nframes_t NumFrames, void *Arg);
bool ConnectJack(audio_state* AudioState);

bool StartJack(audio_state* AudioState) {

    const char *ServerName = NULL;
    jack_options_t Options = JackNullOption;
    jack_status_t Status;

    AudioState->Client = jack_client_open("Wavetable", Options, &Status, ServerName);
    if (AudioState->Client == NULL) {
        fprintf(stderr, "jack_client_open() failed, status = 0x%2.0x\n", Status);
        if (Status & JackServerFailed) {
            fprintf(stderr, "Unable to connect to JACK server\n");
        }
        return false;
    }
    if (Status & JackServerStarted) {
        fprintf(stderr, "JACK server started\n");
    }


    // jack_on_shutdown(client, jack_shutdown, 0);

    printf("engine sample rate: %" PRIu32 "\n",
        jack_get_sample_rate(AudioState->Client));

    AudioState->OutputPortLeft = jack_port_register(AudioState->Client,  "output_left",
                      JACK_DEFAULT_AUDIO_TYPE,
                      JackPortIsOutput|JackPortIsTerminal, 0);
    AudioState->OutputPortRight = jack_port_register(AudioState->Client, "output_right",
                      JACK_DEFAULT_AUDIO_TYPE,
                      JackPortIsOutput|JackPortIsTerminal, 0);

    if (AudioState->OutputPortLeft == NULL || AudioState->OutputPortRight == NULL) {
        fprintf(stderr, "no more JACK ports available\n");
        return false;
    }

    jack_set_process_callback(AudioState->Client, AudioCallback, (void*)AudioState);

    if (jack_activate(AudioState->Client)) {
        fprintf(stderr, "cannot activate client");
        return false;
    }

    bool Connected = ConnectJack(AudioState);
    return Connected;
}

bool ConnectJack(audio_state* AudioState) {
    // Connect our left and right ports to JACK's ports
    // ("Input" here meaning we are "Inputting to JACK")
    const char** Ports = jack_get_ports(AudioState->Client,
        NULL, NULL, JackPortIsPhysical|JackPortIsInput);
    if (Ports == NULL) {
        fprintf(stderr, "no physical playback ports\n");
        return false;
    }

    if (   jack_connect(AudioState->Client, jack_port_name(AudioState->OutputPortLeft),  Ports[0])
        || jack_connect(AudioState->Client, jack_port_name(AudioState->OutputPortRight), Ports[1])) {
        fprintf(stderr, "cannot connect output ports\n");
        free(Ports);
        return false;
    }

    free(Ports);
    return true;
}

int AudioCallback(jack_nframes_t NumFrames, void *UserData) {

    audio_state *AudioState = (audio_state*)UserData;
    if (!AudioState) return 0;

    if (!AudioState->TickUGen || ReloadLibrary(AudioState->UGen)) {
        AudioState->TickUGen = GetLibrarySymbol(AudioState->UGen, "TickUGen");
    }

    AudioState->TickUGen(NumFrames, UserData);
    return 0;
}

int main(int argc, char const *argv[]) {
    audio_state* AudioState = calloc(1, sizeof(audio_state));

    AudioState->UGen = CreateLibrary(
        "audio-wavetable",
        "audio-wavetable.c", NULL, NULL);
    AudioState->TickUGen = GetLibrarySymbol(AudioState->UGen, "TickUGen");

    bool JackStarted = StartJack(AudioState);
    if (!JackStarted) {
        free(AudioState);
        return -1;
    }

    while (true) {
        RecompileLibrary(AudioState->UGen);
    }


    return 0;
}

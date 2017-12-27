#include "audio-interface.h"
#include <math.h>
#include <stdlib.h>

float RandomFloat() {
    return (float)rand() / (float)RAND_MAX;
}

float RandomRange(float Low, float High) {
    const float Range = High-Low;
    return Low + Range * RandomFloat();
}

int TickUGen(
    jack_nframes_t NumFrames, void *Arg) {

    audio_state *AudioState = (audio_state*)Arg;

    jack_nframes_t SampleRate = jack_get_sample_rate(AudioState->Client);

    jack_default_audio_sample_t *OutputBufferLeft  = jack_port_get_buffer(AudioState->OutputPortLeft,  NumFrames);
    jack_default_audio_sample_t *OutputBufferRight = jack_port_get_buffer(AudioState->OutputPortRight, NumFrames);

    float *OutLeft  = (float*)OutputBufferLeft;
    float *OutRight = (float*)OutputBufferRight;
    for (int SampleIndex = 0; SampleIndex < NumFrames; SampleIndex++) {
        *OutLeft++  = RandomRange(-1,1);
        *OutRight++ = RandomRange(-1,1);
    }

    return 0;
}

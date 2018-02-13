#include "audio-dyn-interface.h"
#include "utils.h"

void TickUGen(
    audio_unit* Unit,
    uint32_t NumFrames,
    uint32_t SampleRate) {
    for (int F = 0; F < NumFrames; F++) {
        Unit->Output[F] = 0;
        for (int I = 0; I < ARRAY_LEN(Unit->Inputs); I++) {
            Unit->Output[F] += GetInput(Unit->Inputs[I], F)*0.5;
        }
    }
}

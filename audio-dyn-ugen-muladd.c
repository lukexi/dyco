#include "audio-dyn-interface.h"
#include "utils.h"

void TickUGen(
    dsp_unit* Unit,
    uint32_t NumFrames,
    uint32_t SampleRate) {
    for (int F = 0; F < NumFrames; F++) {
        Unit->Output[F] =
              GetInput(Unit->Inputs[0], F)
            * GetInput(Unit->Inputs[1], F)
            + GetInput(Unit->Inputs[2], F);
    }
}

#include "audio-dyn-interface.h"
#include "utils.h"

typedef struct {
    double Phase;
} clock_gen;

void TickUGen(
    dsp_unit* Unit,
    uint32_t NumFrames,
    uint32_t SampleRate) {

    if (!Unit->State) {
        Unit->State = calloc(1, sizeof(clock_gen));
    }
    clock_gen* State = (clock_gen*)Unit->State;

    for (int F = 0; F < NumFrames; F++) {

        const float Freq = GetInput(Unit->Inputs[0], F);
        const float T = 1.0/(float)SampleRate;
        const float NewPhase = State->Phase + T * Freq;
        const float Output = (int)State->Phase == (int)NewPhase ? 0 : 1;
        State->Phase = NewPhase;
        Unit->Output[F] = Output;
    }
}

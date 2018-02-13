
#include "audio-dyn-interface.h"
#include "utils.h"

typedef struct {
    double Phase;
} env_ar;

void TickUGen(
    audio_unit* Unit,
    uint32_t NumFrames,
    uint32_t SampleRate) {

    if (!Unit) return;

    if (!Unit->State) {
        Unit->State = calloc(1, sizeof(env_ar));
    }
    env_ar* State = (env_ar*)Unit->State;

    for (int F = 0; F < NumFrames; F++) {

        float Trigger     = GetInput(Unit->Inputs[0], F);
        float AttackTime  = GetInput(Unit->Inputs[1], F);
        float ReleaseTime = GetInput(Unit->Inputs[2], F);

        if (Trigger > 0) {
            State->Phase = 0;
        }
        const float T = 1/(float)SampleRate;
        State->Phase += T;

        const float Phase = State->Phase;
        const float Linear = (Phase < AttackTime) ?
            Phase / AttackTime :
            MAX(0, 1 - (Phase - AttackTime) / ReleaseTime);

        Unit->Output[F] = powf(Linear, 1.1);;
    }
}

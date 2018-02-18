#include "audio-dyn-interface.h"
#include "audio-lib.c"
#include <stdio.h>
#include <assert.h>
#include "utils.h"

dsp_unit* CreateUnit(char* Name, char* FileName);

void FreeUnit(dsp_unit* Unit, dsp_units* FreeList) {
    if (Unit == NULL) return;
    // Keep track of which pointers have already been freed, so we
    // don't double free units connected in e.g. a diamond shape
    for (int I = 0; I < FreeList->Count; I++) {
        if (FreeList->Units[I] == Unit) return;
    }
    FreeList->Units[FreeList->Count++] = Unit;

    printf("Freeing %s\n", Unit->Library->Name);

    FreeLibrary(Unit->Library);
    Unit->Library = NULL;

    for (int I = 0; I < ARRAY_LEN(Unit->Inputs); I++) {
        FreeUnit(Unit->Inputs[I].Unit, FreeList);
        Unit->Inputs[I].Unit = NULL;
    }

    free(Unit);
}

void Cleanup(audio_state* AudioState) {
    dsp_units FreeList;
    FreeList.Count = 0;
    printf("Cleaning up audio graph...\n");
    FreeUnit(AudioState->OutputUnit, &FreeList);
    AudioState->OutputUnit = NULL;
}

void Initialize(audio_state* AudioState) {
    static bool Initialized = false;
    // printf("Initialized? %i\n", Initialized);
    if (Initialized) return;
    Initialized = true;

    dsp_unit* Clock = CreateUnit("clock1", "audio-dyn-ugen-clock.c");
    Clock->Inputs[0].Constant = 1;

    dsp_unit* Env = CreateUnit("env1", "audio-dyn-ugen-env.c");
    Env->Inputs[0].Unit = Clock;
    Env->Inputs[1].Constant = 0.1;
    Env->Inputs[2].Constant = 1.0;

    dsp_unit* LFO = CreateUnit("lfo", "audio-dyn-ugen-sin.c");
    LFO->Inputs[0].Constant = 0.1;
    dsp_unit* LFOMulAdd = CreateUnit("muladdlfo", "audio-dyn-ugen-muladd.c");
    LFOMulAdd->Inputs[0].Unit = LFO;
    LFOMulAdd->Inputs[1].Constant = 500;
    LFOMulAdd->Inputs[2].Constant = 500;

    dsp_unit* Sin3 = CreateUnit("sin3", "audio-dyn-ugen-sin.c");
    Sin3->Inputs[0].Constant = 440;

    dsp_unit* MulAdd1 = CreateUnit("muladd1", "audio-dyn-ugen-muladd.c");

    MulAdd1->Inputs[0].Unit = Sin3;    // input
    MulAdd1->Inputs[1].Unit = LFOMulAdd; // mul
    MulAdd1->Inputs[2].Constant = 440; // add

    dsp_unit* MulAdd2 = CreateUnit("muladd2", "audio-dyn-ugen-muladd.c");

    MulAdd2->Inputs[0].Unit = Sin3;    // input
    MulAdd2->Inputs[1].Unit = LFOMulAdd; // mul
    MulAdd2->Inputs[2].Constant = 880; // add

    dsp_unit* Sin1 = CreateUnit("sin1", "audio-dyn-ugen-sin.c");
    dsp_unit* Sin2 = CreateUnit("sin2", "audio-dyn-ugen-sin.c");

    Sin1->Inputs[0].Unit = MulAdd1; // Freq
    Sin2->Inputs[0].Unit = MulAdd2; // Freq

    dsp_unit* Mix1 = CreateUnit("mix1", "audio-dyn-ugen-mix.c");
    Mix1->Inputs[0].Unit = Sin1;
    Mix1->Inputs[1].Unit = Sin2;

    dsp_unit* FinalMulAdd = CreateUnit("muladdlfo", "audio-dyn-ugen-muladd.c");
    FinalMulAdd->Inputs[0].Unit = Mix1;
    FinalMulAdd->Inputs[1].Unit = Env;
    FinalMulAdd->Inputs[2].Constant = 0;

    AudioState->OutputUnit = FinalMulAdd;
}

float GetInput(dsp_input Input, uint32_t Frame) {
    if (Input.Unit) return Input.Unit->Output[Frame];
    return Input.Constant;
}

dsp_unit* CreateUnit(char* Name, char* FileName) {
    dsp_unit* Unit   = calloc(1, sizeof(dsp_unit));

    Unit->Library      = CreateLibrary(Name, FileName);
    Unit->TickFunction = GetLibrarySymbol(Unit->Library, "TickUGen");
    Unit->TickID       = -1;

    CreateRingBuffer(&Unit->ScopeBuffer, sizeof(audio_block), 64);

    return Unit;
}

void UpdateUnit(dsp_unit* Unit) {
    RecompileLibrary(Unit->Library);
    if (Unit->Library->LibraryNeedsReload) {
        printf("Recompiling %s\n", Unit->Library->Name);
        void (*Cleanup)(dsp_unit*) = GetLibrarySymbol(Unit->Library, "Cleanup");
        if (Cleanup) Cleanup(Unit);
        if (Unit->State) { free(Unit->State); Unit->State = NULL; }
        // Silence the output
        memset(Unit->Output,0,sizeof(Unit->Output));
        ReloadLibrary(Unit->Library);
        Unit->TickFunction = GetLibrarySymbol(Unit->Library, "TickUGen");
    }
}

void TickUnit(dsp_unit* Unit, uint32_t NumFrames, uint32_t SampleRate, long TickID) {
    if (Unit == NULL) return;
    if (Unit->TickID == TickID) return;
    Unit->TickID = TickID;
    assert(NumFrames <= ARRAY_LEN(Unit->Output));

    UpdateUnit(Unit);

    for (int I = 0; I < ARRAY_LEN(Unit->Inputs); I++) {
        TickUnit(Unit->Inputs[I].Unit, NumFrames, SampleRate, TickID);
    }
    if (Unit->TickFunction) Unit->TickFunction(Unit, NumFrames, SampleRate);

    audio_block Block;
    Block.Length = NumFrames;
    assert(NumFrames <= ARRAY_LEN(Block.Samples));
    memcpy(Block.Samples, Unit->Output, NumFrames*sizeof(float));
    WriteRingBuffer(&Unit->ScopeBuffer, &Block, 1);
}

int TickUGen(audio_state *AudioState,
    uint32_t NumFrames,
    uint32_t SampleRate,
    float* OutL,
    float* OutR)
{
    Initialize(AudioState);
    if (!AudioState->OutputUnit) return 0;

    static long TickID = 0;
    TickUnit(AudioState->OutputUnit, NumFrames, SampleRate, TickID++);

    for (int SampleIndex = 0; SampleIndex < NumFrames; SampleIndex++) {
        const float Signal = AudioState->OutputUnit->Output[SampleIndex];
        *OutL++ = Signal;
        *OutR++ = Signal;
    }

    return 0;
}

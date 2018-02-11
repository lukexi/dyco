#include "audio-dyn-interface.h"
#include "audio-lib.c"
#include <stdio.h>
#include <assert.h>
#include "utils.h"

audio_unit* CreateUnit(char* Name, char* FileName);

void FreeUnit(audio_unit* Unit) {
    if (Unit == NULL) return;
    printf("Freeing %s\n", Unit->Library->Name);

    FreeLibrary(Unit->Library);
    Unit->Library = NULL;

    for (int I = 0; I < ARRAY_LEN(Unit->Inputs); I++) {
        FreeUnit(Unit->Inputs[I].Unit);
        Unit->Inputs[I].Unit = NULL;
    }
    // FIXME this isn't safe for a diamond connection shape
    /*
        A
      /   \
    B       C
      \   /
        D
    */
    free(Unit);
}

void Cleanup(audio_state* AudioState) {
    printf("Cleaning up audio graph...\n");
    FreeUnit(AudioState->OutputUnit);
}

void Initialize(audio_state* AudioState) {
    static bool Initialized = false;
    if (Initialized) return;
    Initialized = true;

    audio_unit* Sin3 = CreateUnit("sin3", "audio-dyn-ugen-sin.c");
    Sin3->Inputs[0].Constant = 10;

    audio_unit* MulAdd1 = CreateUnit("muladd1", "audio-dyn-ugen-muladd.c");

    MulAdd1->Inputs[0].Unit = Sin3;    // input
    MulAdd1->Inputs[1].Constant = 440; // mul
    MulAdd1->Inputs[2].Constant = 880; // add

    audio_unit* Sin1 = CreateUnit("sin1", "audio-dyn-ugen-sin.c");
    audio_unit* Sin2 = CreateUnit("sin2", "audio-dyn-ugen-sin.c");

    Sin1->Inputs[0].Unit = MulAdd1; // Freq
    Sin2->Inputs[0].Constant = 770; // Freq

    AudioState->OutputUnit = CreateUnit("mix1", "audio-dyn-ugen-mix.c");
    AudioState->OutputUnit->Inputs[0].Unit = Sin1;
    AudioState->OutputUnit->Inputs[1].Unit = Sin2;
}

float GetInput(audio_input Input, uint32_t Frame) {
    if (Input.Unit) return Input.Unit->Output[Frame];
    return Input.Constant;
}

audio_unit* CreateUnit(char* Name, char* FileName) {
    audio_unit* Unit   = calloc(1, sizeof(audio_unit));


    Unit->Library      = CreateLibrary(Name, FileName);
    Unit->TickFunction = GetLibrarySymbol(Unit->Library, "TickUGen");
    Unit->TickID       = -1;

    CreateRingBuffer(&Unit->ScopeBuffer, sizeof(audio_block), 64);

    return Unit;
}

void UpdateUnit(audio_unit* Unit) {
    RecompileLibrary(Unit->Library);
    if (Unit->Library->LibraryNeedsReload) {
        printf("Recompiling %s\n", Unit->Library->Name);
        void (*Cleanup)(audio_unit*) = GetLibrarySymbol(Unit->Library, "Cleanup");
        if (Cleanup) Cleanup(Unit);
        ReloadLibrary(Unit->Library);
        Unit->TickFunction = GetLibrarySymbol(Unit->Library, "TickUGen");
    }
}

void TickUnit(audio_unit* Unit, uint32_t NumFrames, uint32_t SampleRate, long TickID) {
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
    for (int I = 0; I < NumFrames; I++) {
        Block.Samples[I] = Unit->Output[I];
    }
    WriteRingBuffer(&Unit->ScopeBuffer, &Block, 1);
}

int TickUGen(audio_state *AudioState,
    uint32_t NumFrames,
    uint32_t SampleRate,
    float* OutL,
    float* OutR)
{
    Initialize(AudioState);

    static long TickID = 0;
    TickUnit(AudioState->OutputUnit, NumFrames, SampleRate, TickID++);


    for (int SampleIndex = 0; SampleIndex < NumFrames; SampleIndex++) {
        const float Signal = AudioState->OutputUnit->Output[SampleIndex];
        *OutL++ = Signal;
        *OutR++ = Signal;
    }

    return 0;
}

#include "audio-dyn-interface.h"
#include "audio-lib.c"
#include <stdio.h>
#include "utils.h"

audio_unit* OutputUnit;

audio_unit* CreateUnit(char* Name, char* FileName);

void FreeUnit(audio_unit* Unit) {
    if (Unit == NULL) return;
    printf("Freeing %s\n", Unit->Library->Name);

    FreeLibrary(Unit->Library);
    Unit->Library = NULL;

    for (int I = 0; I < ARRAY_LEN(Unit->Inputs); I++) {
        FreeUnit(Unit->Inputs[I].InputUnit);
        Unit->Inputs[I].InputUnit = NULL;
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

void Cleanup() {
    FreeUnit(OutputUnit);
}

void Initialize() {
    static bool Initialized = false;
    if (Initialized) return;
    Initialized = true;

    audio_unit* Sin1 = CreateUnit("sin1", "audio-dyn-ugen-sin.c");
    audio_unit* Sin2 = CreateUnit("sin2", "audio-dyn-ugen-sin.c");

    Sin1->Inputs[0].Constant = 440;
    Sin2->Inputs[0].Constant = 550;

    OutputUnit = CreateUnit("mix1", "audio-dyn-ugen-mix.c");
    OutputUnit->Inputs[0].InputUnit = Sin1;
    OutputUnit->Inputs[1].InputUnit = Sin2;
}

float GetInput(audio_input Input, uint32_t Frame) {
    if (Input.InputUnit) return Input.InputUnit->Output[Frame];
    return Input.Constant;
}

audio_unit* CreateUnit(char* Name, char* FileName) {
    audio_unit* Unit   = calloc(1, sizeof(audio_unit));


    Unit->Library      = CreateLibrary(Name, FileName);
    Unit->TickFunction = GetLibrarySymbol(Unit->Library, "TickUGen");
    Unit->TickID       = -1;

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

    UpdateUnit(Unit);

    for (int I = 0; I < ARRAY_LEN(Unit->Inputs); I++) {
        TickUnit(Unit->Inputs[I].InputUnit, NumFrames, SampleRate, TickID);
    }


    if (Unit->TickFunction) Unit->TickFunction(Unit, NumFrames, SampleRate);
}

int TickUGen(audio_state *AudioState,
    uint32_t NumFrames,
    uint32_t SampleRate,
    float* OutL,
    float* OutR)
{
    Initialize();

    static long TickID = 0;
    TickUnit(OutputUnit, NumFrames, SampleRate, TickID++);


    for (int SampleIndex = 0; SampleIndex < NumFrames; SampleIndex++) {
        const float Signal = OutputUnit->Output[SampleIndex];
        *OutL++ = Signal;
        *OutR++ = Signal;
    }

    return 0;
}

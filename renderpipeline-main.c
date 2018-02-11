#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "gl.h"
#include "shader.h"
#include "quad.h"
#include "framebuffer.h"
#include "dynamic.h"
#include "renderpipeline-interface.h"

// Stage definition


void CleanupStage(render_stage* Stage) {
    FreeLibrary(Stage->Library);
}

void GetStageFunctions(render_stage* Stage) {
    Stage->TickStage    = GetLibrarySymbol(Stage->Library, "TickStage");
    Stage->CleanupStage = GetLibrarySymbol(Stage->Library, "Cleanup");
}

void CreateStage(render_stage* Stage, char* Name, char* Source) {
    Stage->Library = CreateLibrary(Name, Source);
    GetStageFunctions(Stage);
}

void UpdateStage(render_stage* Stage) {
    if (RecompileLibrary(Stage->Library)) {
        if (Stage->CleanupStage) Stage->CleanupStage();
        ReloadLibrary(Stage->Library);
        GetStageFunctions(Stage);
    }
}


// Render pipeline main
bool Initialized = false;
render_stage Stage1;
render_stage Stage2;

pipeline_state PipelineState;

void DeleteFramebuffer(framebuffer Framebuffer) {
    glDeleteTextures(1, &Framebuffer.FramebufferTex);
    glDeleteRenderbuffers(1, &Framebuffer.FramebufferRB);
    glDeleteFramebuffers(1, &Framebuffer.Framebuffer);
}

void InitFramebuffer(GLsizei Width, GLsizei Height, framebuffer* Framebuffer) {
    CreateFramebuffer(GL_RGBA8, Width, Height,
        &Framebuffer->FramebufferTex,
        &Framebuffer->Framebuffer,
        &Framebuffer->FramebufferRB);
}

void Cleanup() {
    CleanupStage(&Stage1);
    CleanupStage(&Stage2);

    glDeleteVertexArrays(1, &PipelineState.QuadVAO);

    DeleteFramebuffer(PipelineState.NewFramebuffer);
    DeleteFramebuffer(PipelineState.OldFramebuffer);
}

void Initialize() {

    PipelineState.QuadVAO = CreateQuad(FullscreenQuadVertices);

    InitFramebuffer(1024*2, 768*2, &PipelineState.NewFramebuffer);
    InitFramebuffer(1024*2, 768*2, &PipelineState.OldFramebuffer);

    CreateStage(&Stage1, "renderpipeline-stage1", "renderpipeline-stage1.c");
    CreateStage(&Stage2, "renderpipeline-stage2", "renderpipeline-stage2.c");

    Initialized = true;
}

void TickStage(render_stage* Stage, SDL_Window* Window) {
    UpdateStage(Stage);
    if (Stage->TickStage) Stage->TickStage(Window, &PipelineState);
}

void TickRenderPipeline(SDL_Window* Window) {
    if (!Initialized) Initialize();

    // Swap buffers
    framebuffer NewFramebuffer   = PipelineState.OldFramebuffer;
    framebuffer OldFramebuffer   = PipelineState.NewFramebuffer;
    PipelineState.NewFramebuffer = NewFramebuffer;
    PipelineState.OldFramebuffer = OldFramebuffer;

    TickStage(&Stage1, Window);
    TickStage(&Stage2, Window);

    SwapWindowQ(Window);
}
